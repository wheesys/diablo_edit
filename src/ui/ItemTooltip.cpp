#include "ItemTooltip.h"
#include "data/DataManager.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <algorithm>
#include <numeric>

const int ItemTooltip::WINDOW_WIDTH_MIN;
const QColor ItemTooltip::FONT_COLORS[8] = {
    QColor(255, 255, 255),   // WHITE
    QColor(0, 0, 255),       // BLUE
    QColor(0, 255, 0),       // GREEN
    QColor(255, 255, 0),     // RARE
    QColor(148, 128, 100),   // UNIQUE
    QColor(255, 128, 0),     // CRAFT
    QColor(255, 0, 0),       // RED
    QColor(100, 100, 100),   // GRAY
};

ItemTooltip::ItemTooltip(QWidget* parent)
    : QWidget(parent)
    , m_nTransparency(230)
    , m_pItem(nullptr)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setMouseTracking(true);
    hide();
}

ItemTooltip::~ItemTooltip() = default;

void ItemTooltip::addMsg(const QColor& color, const QString& msg)
{
    if (!msg.isEmpty())
        m_itemMsg.append({color, msg});
}

void ItemTooltip::addPropertyList(const QColor& color, DWORD version, const CPropertyList& propList)
{
    if (!g_dataMgr) return;
    for (const auto& [id, val] : propList.mProperty) {
        if (id == 194 || id == 152)  // extend sockets & indestructible
            continue;
        QString desc = g_dataMgr->propertyDescription(version, id, val);
        addMsg(color, desc);
    }
}

void ItemTooltip::showItemInfo(const CD2Item* pItem, QPoint screenPos)
{
    if (!pItem || !g_dataMgr) return;

    m_pItem = pItem;
    m_itemMsg.clear();

    if (pItem->bEar) {
        const auto& ear = *pItem->pEar;
        addMsg(FONT_COLORS[WHITE], g_dataMgr->itemSuspendUI(10) + ear.sEarName);
        addMsg(FONT_COLORS[WHITE], g_dataMgr->className(ear.iEarClass));
        addMsg(FONT_COLORS[WHITE], QString::number(ear.iEarLevel));
    } else {
        const auto& meta = pItem->MetaData();
        BYTE quality = pItem->Quality();

        QColor itemColor;
        if (meta.IsUnique)
            itemColor = FONT_COLORS[UNIQUE];
        else if (meta.IsCraft)
            itemColor = FONT_COLORS[CRAFT];
        else if (quality <= 3)
            itemColor = pItem->bEthereal ? FONT_COLORS[GRAY] : FONT_COLORS[WHITE];
        else
            itemColor = FONT_COLORS[quality - 3];

        if (!pItem->bSimple && pItem->pItemInfo->pExtItemInfo.exist()) {
            const auto& extInfo = *pItem->pItemInfo->pExtItemInfo;
            if (extInfo.sPersonName.exist())
                addMsg(itemColor, QString::fromUtf8(reinterpret_cast<const char*>(&*extInfo.sPersonName.begin())) + "'s");
        }

        QString itemName = g_dataMgr->itemName(meta.NameIndex);

        if (!pItem->bSimple && pItem->pItemInfo->pExtItemInfo.exist()) {
            const auto& extInfo = *pItem->pItemInfo->pExtItemInfo;
            switch (quality) {
            case 1:
                addMsg(itemColor, g_dataMgr->itemSuspendUI(0) + itemName);
                break;
            case 2:
                if (extInfo.wMonsterID.exist())
                    addMsg(itemColor, g_dataMgr->monsterName(extInfo.wMonsterID) + QStringLiteral(" ") + itemName);
                else
                    addMsg(itemColor, itemName);
                break;
            case 3:
                addMsg(itemColor, g_dataMgr->itemSuspendUI(1) + itemName);
                break;
            case 4:
                addMsg(itemColor, g_dataMgr->magicPrefix(extInfo.wPrefix) + QStringLiteral(" ")
                        + itemName + QStringLiteral(" ")
                        + g_dataMgr->magicSuffix(extInfo.wSuffix));
                break;
            case 5:
                addMsg(itemColor, g_dataMgr->setItemName(extInfo.wSetID));
                break;
            case 6:
                if (extInfo.pRareName.exist()) {
                    const auto& rare = *extInfo.pRareName;
                    addMsg(itemColor, g_dataMgr->rareCraftedName(rare.iName1) + QStringLiteral(" ")
                            + g_dataMgr->rareCraftedName(rare.iName2));
                }
                break;
            case 7:
                if (extInfo.wUniID < g_dataMgr->uniqueNameSize())
                    addMsg(itemColor, g_dataMgr->uniqueName(extInfo.wUniID));
                break;
            case 8:
                if (extInfo.pCraftName.exist()) {
                    const auto& craft = *extInfo.pCraftName;
                    addMsg(itemColor, g_dataMgr->rareCraftedName(craft.iName1) + QStringLiteral(" ")
                            + g_dataMgr->rareCraftedName(craft.iName2));
                }
                break;
            }
        } else {
            addMsg(itemColor, itemName);
        }

        if (quality <= 3 && pItem->IsRuneWord()) {
            int id = pItem->RuneWordId();
            addMsg(FONT_COLORS[UNIQUE], g_dataMgr->runeWordName(id));
        }

        if (meta.HasDef && pItem->pItemInfo->pTpSpInfo.exist()) {
            addMsg(FONT_COLORS[WHITE],
                QStringLiteral("Defense: %1").arg(pItem->pItemInfo->pTpSpInfo->GetDefence()));
        } else if (meta.Damage1Min) {
            addMsg(FONT_COLORS[WHITE],
                QStringLiteral("Damage: %1-%2").arg(meta.Damage1Min).arg(meta.Damage1Max));
        } else if (meta.Damage2Min) {
            addMsg(FONT_COLORS[WHITE],
                QStringLiteral("2-Hand Damage: %1-%2").arg(meta.Damage2Min).arg(meta.Damage2Max));
        }

        if (meta.IsStacked && pItem->pItemInfo->pTpSpInfo.exist()) {
            addMsg(FONT_COLORS[WHITE],
                QStringLiteral("Quantity: %1").arg(static_cast<int>(pItem->pItemInfo->pTpSpInfo->iQuantity)));
        } else if (pItem->pItemInfo->IsGold()) {
            addMsg(FONT_COLORS[WHITE],
                QStringLiteral("Gold: %1").arg(pItem->pItemInfo->pGold->wQuantity));
        }

        if (meta.HasDur && pItem->pItemInfo->pTpSpInfo.exist()) {
            if (!pItem->pItemInfo->pTpSpInfo->IsIndestructible()) {
                addMsg(FONT_COLORS[WHITE],
                    QStringLiteral("Durability: %1/%2")
                        .arg(static_cast<int>(pItem->pItemInfo->pTpSpInfo->iCurDur))
                        .arg(static_cast<int>(pItem->pItemInfo->pTpSpInfo->iMaxDurability)));
            } else {
                addMsg(FONT_COLORS[BLUE], g_dataMgr->itemSuspendUI(7));
            }
        }

        if (!pItem->bSimple && pItem->pItemInfo->pTpSpInfo.exist()) {
            addPropertyList(FONT_COLORS[BLUE], pItem->dwVersion,
                pItem->pItemInfo->pTpSpInfo->stPropertyList);

            if (pItem->IsSet()) {
                const auto& setProps = pItem->pItemInfo->pTpSpInfo->apSetProperty;
                for (size_t i = 0; i < 5; ++i) {
                    if (setProps[i].exist())
                        addPropertyList(FONT_COLORS[GREEN], pItem->dwVersion, *setProps[i]);
                }
            } else if (pItem->IsRuneWord() && pItem->pItemInfo->pTpSpInfo->stRuneWordPropertyList.exist()) {
                addPropertyList(FONT_COLORS[BLUE], pItem->dwVersion,
                    *pItem->pItemInfo->pTpSpInfo->stRuneWordPropertyList);
            }
        }

        if (pItem->bEthereal)
            addMsg(FONT_COLORS[BLUE], g_dataMgr->itemSuspendUI(8));

        if (pItem->bSocketed) {
            int sockets = pItem->Sockets();
            if (sockets > 0) {
                int gems = pItem->Gems();
                addMsg(FONT_COLORS[BLUE],
                    QStringLiteral("Socketed: %1/%2").arg(gems).arg(sockets));
            }
        }
    }

    if (!pItem->bIdentified)
        addMsg(FONT_COLORS[RED], g_dataMgr->itemSuspendUI(12));

    QFontMetrics fm(font());
    int maxWidth = 0;
    for (const auto& [color, msg] : m_itemMsg) {
        int textWidth = fm.horizontalAdvance(msg);
        maxWidth = std::max(maxWidth, textWidth);
    }

    int w = std::max(WINDOW_WIDTH_MIN, maxWidth + WIDTH_PER_CHAR);
    int h = HEIGHT_PER_LINE * (m_itemMsg.size() + 1);

    auto* screen = QApplication::screenAt(screenPos);
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        if (screenPos.x() + w > screenRect.right())
            screenPos.setX(screenRect.right() - w);
        if (screenPos.y() + h > screenRect.bottom())
            screenPos.setY(screenRect.bottom() - h);
    }

    setGeometry(screenPos.x(), screenPos.y(), w, h);
    setWindowOpacity(m_nTransparency / 255.0);
    raise();
    show();
}

void ItemTooltip::setTooltipTransparency(int alpha)
{
    m_nTransparency = std::clamp(alpha, 0, 255);
    if (isVisible())
        setWindowOpacity(m_nTransparency / 255.0);
}

void ItemTooltip::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), Qt::black);

    QRect lineRect = rect();
    lineRect.setTop(HEIGHT_PER_LINE / 2);
    lineRect.setBottom(lineRect.top() + HEIGHT_PER_LINE);

    painter.setPen(Qt::white);
    for (const auto& [color, msg] : m_itemMsg) {
        painter.setPen(color);
        painter.drawText(lineRect, Qt::AlignCenter, msg);
        lineRect.moveTop(lineRect.bottom());
        lineRect.setBottom(lineRect.bottom() + HEIGHT_PER_LINE);
    }
}

void ItemTooltip::mousePressEvent(QMouseEvent* /*event*/)
{
    hide();
}
