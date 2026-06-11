#include "ItemGridWidget.h"
#include "ItemTooltip.h"
#include "core/ImageManager.h"
#include "data/DataManager.h"
#include "FoundryDialog.h"
#include "NewItemDialog.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QApplication>

// ==================== 位置信息表（从 MFC 原版 POSITION_INFO 迁移） ====================

static PositionInfo POSITION_INFO[POSITION_END] = {
    // left, top, cols, rows, equipType
    {10, 5, 6, 8, -1},         // STASH
    {10, 5, 10, 10, -1},       // STASH_D2R
    {10, 315, 10, 4, -1},      // INVENTORY
    {320, 315, 3, 4, ~E_A_BOX},// CUBE
    {420, 315, 4, 4, E_IN_BELT}, // IN_BELT
    {70, 445, MAX_SOCKETS_DISPLAY, 1, E_SOCKET}, // IN_SOCKET

    {420, 30, 2, 2, E_HEAD},   // HEAD
    {485, 65, 1, 1, E_NECK},   // NECK
    {420, 95, 2, 3, E_BODY},   // BODY
    {320, 65, 2, 4, E_HAND},   // RIGHT_HAND
    {520, 65, 2, 4, E_HAND},   // LEFT_HAND
    {385, 190, 1, 1, E_RING},  // RIGHT_RING
    {485, 190, 1, 1, E_RING},  // LEFT_RING
    {420, 190, 2, 1, E_BELT},  // BELT
    {520, 190, 2, 2, E_FOOT},  // FOOT
    {320, 190, 2, 2, E_GLOVE}, // GLOVE

    {780, 30, 2, 2, E_HEAD},   // CORPSE_HEAD
    {845, 65, 1, 1, E_NECK},   // CORPSE_NECK
    {780, 95, 2, 3, E_BODY},   // CORPSE_BODY
    {680, 65, 2, 4, E_HAND},   // CORPSE_RIGHT_HAND
    {880, 65, 2, 4, E_HAND},   // CORPSE_LEFT_HAND
    {745, 190, 1, 1, E_RING},  // CORPSE_RIGHT_RING
    {845, 190, 1, 1, E_RING},  // CORPSE_LEFT_RING
    {780, 190, 2, 1, E_BELT},  // CORPSE_BELT
    {880, 190, 2, 2, E_FOOT},  // CORPSE_FOOT
    {680, 190, 2, 2, E_GLOVE}, // CORPSE_GLOVE

    {660, 280, 2, 2, E_HEAD},  // MERCENARY_HEAD
    {660, 345, 2, 3, E_BODY},  // MERCENARY_BODY
    {560, 315, 2, 4, E_HAND},  // MERCENARY_RIGHT_HAND
    {760, 315, 2, 4, E_HAND},  // MERCENARY_LEFT_HAND

    {600, 30, 2, 4, -1},       // GOLEM
};

// ==================== 辅助函数 ====================

static bool isCorpse(EPosition pos) { return CORPSE_HEAD <= pos && pos < CORPSE_END; }
static bool isMercenary(EPosition pos) { return MERCENARY_HEAD <= pos && pos < MERCENARY_END; }
static bool isInMouse(EPosition pos) { return IN_MOUSE == pos; }
static bool isInSocket(EPosition pos) { return IN_SOCKET == pos; }
static bool isInRecycle(EPosition pos) { return IN_RECYCLE == pos; }
static bool hasNormalII(EPosition pos) { return RIGHT_HAND == pos || LEFT_HAND == pos; }
static bool hasCorpseII(EPosition pos) { return CORPSE_RIGHT_HAND == pos || CORPSE_LEFT_HAND == pos; }
static bool isGrid(EPosition pos) { return pos < GRID_COUNT; }

static EEquip itemToEquip(const CD2Item& item) {
    if (item.IsBox()) return E_A_BOX;
    auto& meta = item.MetaData();
    if (meta.Equip == 0)
        return meta.IsGem ? E_SOCKET : E_STORAGE;
    return EEquip(1 << meta.Equip);
}

static std::tuple<EPosition, int, int> itemToPosition(int iLocation, int iPosition, int iColumn,
    int iRow, int iStoredIn, int body, bool isD2R)
{
    if (body == 3) return std::make_tuple(GOLEM, 0, 0);
    int pos = -1, x = 0, y = 0;
    switch (iLocation) {
        case 0: // grid
            pos = (iStoredIn == 1 ? INVENTORY : (iStoredIn == 4 ? CUBE : (iStoredIn == 5 ? (isD2R ? STASH_D2R : STASH) : -1)));
            x = iColumn; y = iRow;
            break;
        case 1: // equipped
            if (iPosition > 0 && iPosition <= 12) {
                x = (iPosition <= 10 ? 0 : 1);
                pos = iPosition + GRID_COUNT - 1 - 7 * x;
                if (body == 1) pos += CORPSE_HEAD - HEAD;
                else if (body == 2) {
                    switch (pos) {
                        case HEAD: pos = MERCENARY_HEAD; break;
                        case BODY: pos = MERCENARY_BODY; break;
                        case RIGHT_HAND: pos = MERCENARY_RIGHT_HAND; break;
                        case LEFT_HAND: pos = MERCENARY_LEFT_HAND; break;
                        default: break;
                    }
                }
            }
            break;
        case 2: // in belt
            pos = IN_BELT;
            x = iColumn % 4;
            y = 3 - iColumn / 4;
            break;
        case 4: // in hand (mouse)
            pos = IN_MOUSE;
            break;
    }
    return std::make_tuple(EPosition(pos), x, y);
}

static std::tuple<int, int, int, int, int> positionToItem(EPosition pos, int x, int y) {
    int loc = 0, body = 0, col = 0, row = 0, store = 0;
    switch (pos) {
        case STASH: case STASH_D2R: col = x; row = y; store = 5; break;
        case INVENTORY: col = x; row = y; store = 1; break;
        case CUBE: col = x; row = y; store = 4; break;
        case IN_BELT: loc = 2; col = (3 - y) * 4 + x; break;
        case IN_SOCKET: loc = 6; col = x; break;
        case RIGHT_HAND: case LEFT_HAND: body = (x == 1 ? 7 : 0);
            [[fallthrough]];
        case HEAD: case NECK: case BODY: case RIGHT_RING: case LEFT_RING:
        case BELT: case FOOT: case GLOVE:
            loc = 1; body += pos - HEAD + 1; break;
        case CORPSE_RIGHT_HAND: case CORPSE_LEFT_HAND: body = (x == 1 ? 7 : 0);
            [[fallthrough]];
        case CORPSE_HEAD: case CORPSE_NECK: case CORPSE_BODY: case CORPSE_RIGHT_RING:
        case CORPSE_LEFT_RING: case CORPSE_BELT: case CORPSE_FOOT: case CORPSE_GLOVE:
            loc = 1; body += pos - CORPSE_HEAD + 1; break;
        case MERCENARY_HEAD:     loc = 1; body = 1; break;
        case MERCENARY_BODY:     loc = 1; body = 3; break;
        case MERCENARY_RIGHT_HAND: loc = 1; body = 4; break;
        case MERCENARY_LEFT_HAND:  loc = 1; body = 5; break;
        case IN_MOUSE: loc = 4; break;
        default: break;
    }
    return std::make_tuple(loc, body, col, row, store);
}

// ==================== CItemView ====================

CItemView::CItemView(const CD2Item& it, EEquip eq, EPosition pos, int x, int y)
    : item(it)
    , picIndex(it.MetaData().PicIndex)
    , equip(eq)
    , position(pos)
    , gridX(x), gridY(y)
    , gridWidth((it.MetaData().Range >> 4) & 0xF)
    , gridHeight(it.MetaData().Range & 0xF)
{
    gemItems.resize(item.Sockets(), -1);
}

QSize CItemView::viewSize() const {
    return QSize(gridWidth * GRID_WIDTH, gridHeight * GRID_WIDTH);
}

int CItemView::gemCount() const {
    int r = 0;
    for (int i : gemItems)
        if (i >= 0) ++r;
    return r;
}

const CD2Item& CItemView::updateItem(QVector<CItemView>& vItemViews) {
    auto t = positionToItem(position, gridX, gridY);
    item.iLocation = std::get<0>(t);
    item.iPosition = std::get<1>(t);
    item.iColumn = std::get<2>(t);
    item.iRow = std::get<3>(t);
    item.iStoredIn = std::get<4>(t);

    item.aGemItems.clear();
    int gems = 0;
    for (int i : gemItems) {
        if (i < 0) continue;
        item.aGemItems.push_back(vItemViews[i].updateItem(vItemViews));
        ++gems;
    }
    if (item.pItemInfo.exist() && item.pItemInfo->pExtItemInfo.exist())
        item.pItemInfo->pExtItemInfo->nGems = gems;
    return item;
}

// ==================== GridView ====================

GridView::GridView(EPosition pos, const PositionInfo& info)
    : rect(info.left, info.top, info.cols * GRID_WIDTH, info.rows * GRID_WIDTH)
    , equipType(EEquip(info.equipType))
    , m_position(pos)
    , m_cols(info.cols)
    , m_rows(info.rows)
{
    int cellCount = (isGrid() ? m_cols * m_rows : (hasNormalII(EPosition(pos)) || hasCorpseII(EPosition(pos)) ? 2 : 1));
    m_itemIndex.resize(cellCount, -1);
}

bool GridView::isGrid() const { return ::isGrid(EPosition(m_position)); }
bool GridView::isSockets() const { return ::isInSocket(EPosition(m_position)); }

void GridView::reset() { std::fill(m_itemIndex.begin(), m_itemIndex.end(), -1); }

int GridView::itemIndex(int x, int y) const {
    int idx = x + y * m_cols;
    if (idx < 0 || idx >= m_itemIndex.size()) return -1;
    return m_itemIndex[idx];
}

void GridView::setItemIndex(int index, int x, int y) {
    int idx = x + y * m_cols;
    if (idx >= 0 && idx < m_itemIndex.size())
        m_itemIndex[idx] = index;
}

void GridView::setItemIndex(int index, int x, int y, int width, int height) {
    if (isGrid())
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j)
                setItemIndex(index, x + i, y + j);
    else
        setItemIndex(index, x, y);
}

QPoint GridView::indexToXY(int x, int y, int width, int height) const {
    if (isGrid())
        return QPoint(rect.left() + x * GRID_WIDTH, rect.top() + y * GRID_WIDTH);
    int px = rect.left() + (m_cols - width) * GRID_WIDTH / 2;
    int py = rect.top() + (m_rows - height) * GRID_WIDTH / 2;
    return QPoint(px, py);
}

GridView::HitResult GridView::xyToPositionIndex(QPoint pt, bool altWeapon, bool corpseII, int col, int row) const {
    if (isGrid()) {
        int x = (pt.x() - (col - 1) * GRID_WIDTH / 2 - rect.left()) / GRID_WIDTH;
        int y = (pt.y() - (row - 1) * GRID_WIDTH / 2 - rect.top()) / GRID_WIDTH;
        x = qBound(0, x, m_cols - 1);
        y = qBound(0, y, m_rows - 1);
        return {m_position, x, y};
    } else if (hasNormalII(EPosition(m_position))) {
        return {m_position, altWeapon ? 1 : 0, 0};
    } else if (hasCorpseII(EPosition(m_position))) {
        return {m_position, corpseII ? 1 : 0, 0};
    }
    return {m_position, 0, 0};
}

std::pair<bool, int> GridView::putItem(int index, int x, int y, int width, int height, EEquip equip) {
    if (!canEquip(equip)) return {false, -1};
    if (isGrid()) {
        if (x + width > m_cols || y + height > m_rows) return {false, -1};
        int exist = -1;
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j) {
                int e = itemIndex(x + i, y + j);
                if (e >= 0 && exist >= 0 && e != exist) return {false, -1};
                exist = qMax(exist, e);
            }
        if (exist >= 0) return {false, exist};
        setItemIndex(index, x, y, width, height);
    } else {
        int e = itemIndex(x, y);
        if (e >= 0) return {false, e};
        setItemIndex(index, x, y);
    }
    return {true, -1};
}

// ==================== ItemGridWidget ====================

ItemGridWidget::ItemGridWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(960, 520);
    setMouseTracking(true);
    setupMercUI();
    initGridView();
}

ItemGridWidget::~ItemGridWidget() = default;

void ItemGridWidget::initGridView() {
    m_gridViews.clear();
    for (int i = STASH; i < POSITION_END; ++i)
        m_gridViews.emplace_back(EPosition(i), POSITION_INFO[i]);
    setD2R(true);
}

void ItemGridWidget::setupMercUI() {
    auto* topLayout = new QHBoxLayout;

    auto* row1 = new QHBoxLayout;
    m_chMercenary = new QCheckBox(QStringLiteral("Mercenary"));
    row1->addWidget(m_chMercenary);

    m_cbMercType = new QComboBox;
    m_cbMercType->setEnabled(false);
    row1->addWidget(new QLabel(QStringLiteral("Type:")));
    row1->addWidget(m_cbMercType);

    m_cbMercName = new QComboBox;
    m_cbMercName->setEnabled(false);
    row1->addWidget(new QLabel(QStringLiteral("Name:")));
    row1->addWidget(m_cbMercName);

    m_edMercExp = new QLineEdit;
    m_edMercExp->setEnabled(false);
    m_edMercExp->setMaximumWidth(80);
    row1->addWidget(new QLabel(QStringLiteral("Exp:")));
    row1->addWidget(m_edMercExp);

    m_chMercDead = new QCheckBox(QStringLiteral("Dead"));
    m_chMercDead->setEnabled(false);
    row1->addWidget(m_chMercDead);

    m_chCorpse = new QCheckBox(QStringLiteral("Corpse"));
    row1->addWidget(m_chCorpse);

    m_chSecondHand = new QCheckBox(QStringLiteral("II Hand"));
    row1->addWidget(m_chSecondHand);

    m_chD2R = new QCheckBox(QStringLiteral("D2R"));
    m_chD2R->setChecked(true);
    row1->addWidget(m_chD2R);

    row1->addStretch();
    topLayout->addLayout(row1);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->addLayout(topLayout);

    // recycle list
    m_lstRecycle = new QListWidget;
    m_lstRecycle->setMaximumHeight(100);
    m_lstRecycle->setToolTip(QStringLiteral("Double-click to recover item"));

    auto* bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_lstRecycle);
    bottomLayout->addStretch();

    // 使用 scroll area 包含画布
    mainLayout->addStretch(1);
    mainLayout->addLayout(bottomLayout);
    mainLayout->addSpacing(20);

    // signals
    connect(m_chMercenary, &QCheckBox::toggled, this, [this](bool checked) {
        m_hasMercenary = checked;
        for (int i = MERCENARY_HEAD; i < MERCENARY_END; ++i)
            m_gridViews[i].setEnabled(checked);
        m_cbMercName->setEnabled(checked);
        m_cbMercType->setEnabled(checked);
        m_edMercExp->setEnabled(checked);
        m_chMercDead->setEnabled(checked);
        if (!checked && m_selectedItemIndex >= 0 && isMercenary(m_itemViews[m_selectedItemIndex].position)) {
            m_selectedItemIndex = m_selectedSocketIndex = -1;
            m_gridViews[IN_SOCKET].reset();
        }
        update();
    });

    connect(m_chCorpse, &QCheckBox::toggled, this, [this](bool checked) {
        m_hasCorpse = checked;
        for (int i = CORPSE_HEAD; i < CORPSE_END; ++i)
            m_gridViews[i].setEnabled(checked);
        if (!checked && m_selectedItemIndex >= 0 && isCorpse(m_itemViews[m_selectedItemIndex].position)) {
            m_selectedItemIndex = m_selectedSocketIndex = -1;
            m_gridViews[IN_SOCKET].reset();
        }
        update();
    });

    connect(m_chSecondHand, &QCheckBox::toggled, this, [this]() {
        m_secondHand = m_chSecondHand->isChecked();
        if (m_selectedItemIndex >= 0) {
            auto& g = m_gridViews[m_itemViews[m_selectedItemIndex].position];
            if (g.position() == RIGHT_HAND || g.position() == LEFT_HAND) {
                m_selectedItemIndex = m_selectedSocketIndex = -1;
                m_gridViews[IN_SOCKET].reset();
            }
        }
        update();
    });

    connect(m_chD2R, &QCheckBox::toggled, this, &ItemGridWidget::setD2R);
    connect(m_cbMercType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ItemGridWidget::onMercTypeChanged);

    connect(m_lstRecycle, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int row = m_lstRecycle->row(item);
        if (row < 0) return;
        QVariant data = item->data(Qt::UserRole);
        int idx = data.toInt();
        if (idx < 0 || idx >= m_itemViews.size()) return;
        auto& view = m_itemViews[idx];
        view.position = IN_MOUSE;
        m_pickedItemIndex = idx;
        m_itemCursor = QCursor(createItemCursor(view));
        setCursor(m_itemCursor);
        delete m_lstRecycle->takeItem(row);
        update();
    });
}

void ItemGridWidget::calculateScale() {
    // 简单版本：直接使用 GRID_WIDTH，不做 DPI 缩放
    // MFC 原版从 dialog resource 中计算缩放，Qt 中我们用固定值
}

void ItemGridWidget::setD2R(bool v) {
    m_isD2R = v;
    m_gridViews[STASH].setEnabled(!v);
    m_gridViews[STASH].setVisible(!v);
    m_gridViews[STASH_D2R].setEnabled(v);
    m_gridViews[STASH_D2R].setVisible(v);
    if (m_chD2R) m_chD2R->setChecked(v);
    update();
}

// ==================== 绘制 ====================

static void drawGrid(QPainter& painter, const QRect& rect, int intervalX, int intervalY) {
    for (int x = rect.left(); x <= rect.right(); x += intervalX)
        painter.drawLine(x, rect.top(), x, rect.bottom());
    for (int y = rect.top(); y <= rect.bottom(); y += intervalY)
        painter.drawLine(rect.left(), y, rect.right(), y);
}

void ItemGridWidget::drawGrids(QPainter& painter) {
    painter.save();
    QPen pen(QColor(0, 200, 100), 1);
    painter.setPen(pen);
    for (auto& g : m_gridViews) {
        if (!g.visible()) continue;
        if (g.isGrid())
            drawGrid(painter, g.rect, GRID_WIDTH, GRID_WIDTH);
        else
            drawGrid(painter, g.rect, g.rect.width(), g.rect.height());
    }
    painter.restore();
}

void ItemGridWidget::drawItemAt(QPainter& painter, QPoint pos, const CItemView& view) {
    auto* img = ImageManager::instance();
    QPixmap pix = img->getPixmap(view.picIndex);
    if (pix.isNull()) return;
    QSize sz = view.viewSize();
    painter.drawPixmap(QRect(pos, sz), pix);
}

void ItemGridWidget::drawAllItems(QPainter& painter) {
    QRect selectedRect, socketRect;
    for (int i = 0; i < m_itemViews.size(); ++i) {
        auto& view = m_itemViews[i];
        if (isInRecycle(view.position) || isInMouse(view.position) || isInSocket(view.position))
            continue;
        // I/II hand filter
        if ((view.position == RIGHT_HAND || view.position == LEFT_HAND)
            && view.gridX != (m_secondHand ? 1 : 0))
            continue;
        if ((view.position == CORPSE_RIGHT_HAND || view.position == CORPSE_LEFT_HAND)
            && view.gridX != (m_showCorpseIIHand ? 1 : 0))
            continue;

        QPoint pos = getItemPositionXY(view);
        drawItemAt(painter, pos, view);

        if (i == m_selectedItemIndex) {
            selectedRect = QRect(pos, view.viewSize());
            for (int g : view.gemItems) {
                if (g < 0) continue;
                auto& gemView = m_itemViews[g];
                QPoint gPos = getItemPositionXY(gemView);
                drawItemAt(painter, gPos, gemView);
                if (g == m_selectedSocketIndex)
                    socketRect = QRect(gPos, gemView.viewSize());
            }
        }
    }
    // highlight
    const int penW = 2;
    if (socketRect.isValid()) {
        painter.save();
        painter.setPen(QPen(Qt::red, penW));
        drawGrid(painter, socketRect, socketRect.width(), socketRect.height());
        painter.restore();
    }
    if (selectedRect.isValid()) {
        painter.save();
        painter.setPen(QPen(socketRect.isValid() ? QColor(220, 50, 100) : Qt::red, penW));
        drawGrid(painter, selectedRect, selectedRect.width(), selectedRect.height());
        painter.restore();
    }
}

void ItemGridWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(20, 20, 20));
    // offset 绘图起始位置（在 misc controls 下方）
    painter.translate(0, 40);
    drawGrids(painter);
    drawAllItems(painter);
}

// ==================== 鼠标事件 ====================

void ItemGridWidget::mouseMoveEvent(QMouseEvent* event) {
    m_lastMousePos = event->pos();
    if (m_pickedItemIndex < 0) {
        // 移动到格子上时 hover 显示 tooltip 信息（简化：不显示悬浮窗）
    }
    update();
    QWidget::mouseMoveEvent(event);
}

ItemGridWidget::HitResult ItemGridWidget::HitTestPosition(QPoint pos, int col, int row) const {
    pos -= QPoint(0, 40); // offset
    for (auto& g : m_gridViews) {
        if (g.enabled() && g.rect.contains(pos)) {
            auto r = g.xyToPositionIndex(pos, m_secondHand, m_showCorpseIIHand, col, row);
            return {r.pos, r.x, r.y};
        }
    }
    return {-1, -1, -1};
}

void ItemGridWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    QPoint pt = event->pos();
    if (m_pickedItemIndex < 0) {
        // 拿起物品
        auto ht = HitTestPosition(pt);
        int pos = ht.pos, x = ht.x, y = ht.y;
        if (pos >= 0) {
            auto& grid = m_gridViews[pos];
            if (grid.enabled()) {
                int index = grid.itemIndex(x, y);
                if (index >= 0 && index < m_itemViews.size()) {
                    if (grid.isSockets()) {
                        auto& gems = selectedParentItemView().gemItems;
                        if (x < gems.size()) gems[x] = -1;
                    }
                    auto& view = m_itemViews[index];
                    m_pickedItemIndex = index;
                    m_itemCursor = QCursor(createItemCursor(view));
                    setCursor(m_itemCursor);
                    grid.setItemIndex(-1, view.gridX, view.gridY, view.gridWidth, view.gridHeight);
                    view.position = IN_MOUSE;
                    update();
                }
            }
        } else if (event->pos().y() > height() - 130) {
            // 点击回收站 - 丢弃鼠标上的物品
            if (m_pickedItemIndex >= 0) {
                auto& view = pickedItemView();
                view.position = IN_RECYCLE;
                auto* item = new QListWidgetItem(view.item.ItemName(), m_lstRecycle);
                item->setData(Qt::UserRole, m_pickedItemIndex);
                m_pickedItemIndex = -1;
                setCursor(Qt::ArrowCursor);
                update();
            }
        }
    } else {
        // 放下物品
        auto& view = pickedItemView();
        auto ht = HitTestPosition(pt, view.gridWidth, view.gridHeight);
        int pos = ht.pos, x = ht.x, y = ht.y;
        if (pos >= 0) {
            auto& grid = m_gridViews[pos];
            if (grid.enabled()) {
                if (grid.isSockets()) {
                    // 镶嵌宝石
                    if (m_selectedItemIndex >= 0 && x < selectedParentItemView().gemItems.size()) {
                        auto r = putItemInGrid(EPosition(pos), x, y);
                        if (r.first) {
                            selectedParentItemView().gemItems[x] = m_pickedItemIndex;
                            m_pickedItemIndex = r.second;
                            update();
                        }
                    }
                } else {
                    auto r = putItemInGrid(EPosition(pos), x, y);
                    if (r.first) {
                        m_pickedItemIndex = r.second;
                        update();
                    }
                }
            }
        }
    }

    QWidget::mousePressEvent(event);
}

std::pair<bool, int> ItemGridWidget::putItemInGrid(EPosition pos, int x, int y) {
    auto& view = pickedItemView();
    auto& grid = m_gridViews[pos];
    auto r = grid.putItem(m_pickedItemIndex, x, y, view.gridWidth, view.gridHeight, view.equip);
    if (r.first) {
        view.position = pos;
        view.gridX = x; view.gridY = y;
        if (m_pickedItemIndex < 0) return {true, -1};
        setCursor(Qt::ArrowCursor);
        if (m_pickedItemIndex == m_selectedSocketIndex)
            m_selectedSocketIndex = -1;
        return {true, -1};
    } else if (r.second >= 0) {
        auto& v = m_itemViews[r.second];
        grid.setItemIndex(-1, v.gridX, v.gridY, v.gridWidth, v.gridHeight);
        auto r2 = grid.putItem(m_pickedItemIndex, x, y, view.gridWidth, view.gridHeight, view.equip);
        if (r2.first) {
            view.position = pos;
            view.gridX = x; view.gridY = y;
            v.position = IN_MOUSE;
            m_itemCursor = QCursor(createItemCursor(v));
            setCursor(m_itemCursor);
            return {true, r.second};
        }
    }
    return {false, -1};
}

void ItemGridWidget::contextMenuEvent(QContextMenuEvent* event) {
    // mark click on item
    bool clickOnItem = false;
    if (m_pickedItemIndex < 0) {
        auto ht = HitTestPosition(event->pos());
        if (ht.pos >= 0) {
            auto& grid = m_gridViews[ht.pos];
            if (grid.enabled() && grid.itemIndex(ht.x, ht.y) >= 0) {
                clickOnItem = true;
                // update selection
                int pos = ht.pos, x = ht.x, y = ht.y;
                int index = grid.itemIndex(x, y);
                if (index >= 0) {
                    if (grid.isSockets()) {
                        if (index != m_selectedSocketIndex) m_selectedSocketIndex = index;
                    } else if (index != m_selectedItemIndex || m_selectedSocketIndex >= 0) {
                        m_selectedItemIndex = index;
                        m_selectedSocketIndex = -1;
                        auto& v = m_itemViews[index];
                        for (int i = 0; i < MAX_SOCKETS_DISPLAY; ++i)
                            m_gridViews[IN_SOCKET].setItemIndex(
                                (i < v.gemItems.size() ? v.gemItems[i] : -1), i, 0);
                    }
                }
                update();
            }
        }
    }
    showContextMenu(event->globalPos());
}

// ==================== 右键菜单 ====================

void ItemGridWidget::showContextMenu(QPoint globalPos) {
    if (!m_hasCharacter || m_pickedItemIndex >= 0) return;

    bool clickOnItem = (m_selectedItemIndex >= 0 || m_selectedSocketIndex >= 0);
    bool editable = clickOnItem && selectedItemView().item.IsEditable();
    bool notBox = clickOnItem && !selectedItemView().item.IsBox();

    QMenu menu(this);
    QAction* actImport = menu.addAction(QStringLiteral("Import"));
    QAction* actExport = menu.addAction(QStringLiteral("Export"));
    menu.addSeparator();
    QAction* actModify = menu.addAction(QStringLiteral("Modify"));
    QAction* actCopy   = menu.addAction(QStringLiteral("Copy"));
    QAction* actPaste  = menu.addAction(QStringLiteral("Paste"));
    menu.addSeparator();
    QAction* actNew   = menu.addAction(QStringLiteral("New"));
    QAction* actRemove = menu.addAction(QStringLiteral("Remove"));

    actImport->setEnabled(!clickOnItem);
    actExport->setEnabled(clickOnItem);
    actCopy->setEnabled(clickOnItem && notBox);
    actPaste->setEnabled(m_copiedItemIndex >= 0);
    actModify->setEnabled(clickOnItem && editable);
    actNew->setEnabled(!clickOnItem);
    actRemove->setEnabled(clickOnItem);

    QAction* chosen = menu.exec(globalPos);
    if (!chosen) return;

    if (chosen == actImport) onItemImport();
    else if (chosen == actExport) onItemExport();
    else if (chosen == actCopy) onItemCopy();
    else if (chosen == actPaste) onItemPaste();
    else if (chosen == actModify) onItemModify();
    else if (chosen == actNew) onItemNew();
    else if (chosen == actRemove) onItemRemove();
}

void ItemGridWidget::onItemImport() {
    QString path = QFileDialog::getOpenFileName(this,
        QStringLiteral("Import Item"),
        QString(),
        QStringLiteral("Diablo II Item (*.d2i);;All Files (*)"));
    if (path.isEmpty()) return;

    CD2Item item;
    QFile file(path);
    if (item.ReadFile(file)) {
        item.iLocation = 4;
        addItemInGrid(item, 0);
        update();
    }
}

void ItemGridWidget::onItemExport() {
    auto& view = selectedItemView();
    auto& item = view.updateItem(m_itemViews);
    QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("Export Item"),
        view.item.ItemName() + QStringLiteral(".d2i"),
        QStringLiteral("Diablo II Item (*.d2i);;All Files (*)"));
    if (path.isEmpty()) return;
    QFile file(path);
    item.WriteFile(file);
}

void ItemGridWidget::onItemCopy() {
    m_copiedItemIndex = (m_selectedSocketIndex >= 0 ? m_selectedSocketIndex : m_selectedItemIndex);
    onItemPaste();
}

void ItemGridWidget::onItemPaste() {
    if (m_copiedItemIndex < 0 || m_copiedItemIndex >= m_itemViews.size()) return;
    CD2Item item(m_itemViews[m_copiedItemIndex].item);
    item.iLocation = 4;
    addItemInGrid(item, 0);
    update();
}

void ItemGridWidget::onItemModify() {
    auto& view = selectedItemView();
    FoundryDialog dlg(view.item, this);
    if (dlg.exec() == QDialog::Accepted) {
        update();
    }
}

void ItemGridWidget::onItemNew() {
    std::unique_ptr<CD2Item> newItem;
    NewItemDialog dlg(newItem, this);
    if (dlg.exec() == QDialog::Accepted && newItem) {
        newItem->iLocation = 4;
        addItemInGrid(*newItem, 0);
        update();
    }
}

void ItemGridWidget::onItemRemove() {
    auto& view = selectedItemView();
    auto& grid = m_gridViews[view.position];
    if (grid.isSockets()) {
        auto& gems = selectedParentItemView().gemItems;
        if (view.gridX < gems.size()) gems[view.gridX] = -1;
    } else {
        m_gridViews[IN_SOCKET].reset();
    }
    grid.setItemIndex(-1, view.gridX, view.gridY, view.gridWidth, view.gridHeight);
    view.position = IN_RECYCLE;
    auto* item = new QListWidgetItem(view.item.ItemName(), m_lstRecycle);
    item->setData(Qt::UserRole, m_selectedItemIndex);
    if (m_selectedSocketIndex >= 0)
        m_selectedSocketIndex = -1;
    else
        m_selectedItemIndex = m_selectedSocketIndex = -1;
    update();
}

// ==================== 数据接口 ====================

void ItemGridWidget::updateUI(const CD2S_Struct& character) {
    resetAll();
    m_hasCharacter = true;
    setD2R(character.dwVersion >= 0x61); // D2R
    m_weaponSet = character.dwWeaponSet;
    m_secondHand = (m_weaponSet != 0);
    if (m_chSecondHand) m_chSecondHand->setChecked(m_secondHand);

    // character items
    for (auto& item : character.ItemList.vItems)
        addItemInGrid(item, 0);

    // corpse
    if (character.HasCorpse() && character.stCorpse.pCorpseData.exist()) {
        m_hasCorpse = true;
        if (m_chCorpse) m_chCorpse->setChecked(true);
        for (int i = CORPSE_HEAD; i < CORPSE_END; ++i)
            m_gridViews[i].setEnabled(true);
        for (auto& item : character.stCorpse.pCorpseData->stItems.vItems)
            addItemInGrid(item, 1);
    }

    // mercenary
    if (character.HasMercenary()) {
        m_hasMercenary = true;
        if (m_chMercenary) m_chMercenary->setChecked(true);
        for (int i = MERCENARY_HEAD; i < MERCENARY_END; ++i)
            m_gridViews[i].setEnabled(true);
        m_cbMercName->setEnabled(true);
        m_cbMercType->setEnabled(true);
        m_edMercExp->setEnabled(true);
        m_chMercDead->setEnabled(true);

        m_cbMercType->setCurrentIndex(character.wMercType);
        onMercTypeChanged();
        m_cbMercName->setCurrentIndex(character.wMercName);
        m_edMercExp->setText(QString::number(character.dwMercExp));
        m_chMercDead->setChecked(character.bMercDead);

        if (character.stMercenary.stItems.exist())
            for (auto& item : character.stMercenary.stItems->vItems)
                addItemInGrid(item, 2);
    }

    // golem
    if (character.stGolem.bHasGolem && character.stGolem.pItem.exist())
        addItemInGrid(*character.stGolem.pItem, 3);

    update();
}

bool ItemGridWidget::gatherData(CD2S_Struct& character) {
    // validate mercenary
    if (m_hasMercenary) {
        if (m_cbMercType && m_cbMercType->currentIndex() < 0) {
            QMessageBox::critical(this,
                g_dataMgr ? g_dataMgr->msgError() : QStringLiteral("Error"),
                g_dataMgr ? g_dataMgr->msgBoxInfo(46) : QStringLiteral("Invalid mercenary type."));
            return false;
        }
        if (m_cbMercName && m_cbMercName->currentIndex() < (m_isD2R ? 0 : 1)) {
            QMessageBox::critical(this,
                g_dataMgr ? g_dataMgr->msgError() : QStringLiteral("Error"),
                g_dataMgr ? g_dataMgr->msgBoxInfo(47) : QStringLiteral("Invalid mercenary name."));
            return false;
        }
    }

    CItemList normal, corpse, merc;
    MayExist<CD2Item> golem;

    for (auto& view : m_itemViews) {
        if (isInRecycle(view.position) || isInSocket(view.position))
            continue;
        auto& item = view.updateItem(m_itemViews);
        if (isCorpse(view.position))
            corpse.vItems.push_back(item);
        else if (isMercenary(view.position))
            merc.vItems.push_back(item);
        else if (view.position == GOLEM)
            golem.ensure() = item;
        else
            normal.vItems.push_back(item);
    }

    character.ItemList.SwapItems(normal);

    if (m_hasCorpse) {
        character.stCorpse.wCount = 1;
        character.stCorpse.pCorpseData.ensure().stItems.SwapItems(corpse);
    } else {
        character.stCorpse.wCount = 0;
        character.stCorpse.pCorpseData.reset();
    }

    if (m_hasMercenary) {
        if (!character.dwMercControl)
            character.dwMercControl = (rand() | 0x10);
        character.wMercType = m_cbMercType ? m_cbMercType->currentIndex() : 0;
        character.wMercName = m_cbMercName ? m_cbMercName->currentIndex() : 0;
        character.dwMercExp = m_edMercExp ? m_edMercExp->text().toUInt() : 0;
        character.bMercDead = m_chMercDead ? m_chMercDead->isChecked() : false;
        character.stMercenary.stItems.ensure().SwapItems(merc);
    } else {
        character.dwMercControl = 0;
        character.wMercType = 0;
        character.wMercName = 0;
        character.dwMercExp = 0;
        character.bMercDead = false;
        character.stMercenary.stItems.reset();
    }

    character.stGolem.bHasGolem = golem.exist();
    character.stGolem.pItem.swap(golem);
    return true;
}

void ItemGridWidget::resetAll() {
    for (auto& grid : m_gridViews)
        grid.reset();
    m_itemViews.clear();
    m_hasCorpse = false;
    m_weaponSet = 0;
    m_secondHand = false;
    if (m_chSecondHand) m_chSecondHand->setChecked(false);
    if (m_chCorpse) m_chCorpse->setChecked(false);

    m_selectedItemIndex = m_selectedSocketIndex = -1;
    m_pickedItemIndex = -1;
    setCursor(Qt::ArrowCursor);
    m_copiedItemIndex = -1;

    m_hasMercenary = false;
    if (m_chMercenary) m_chMercenary->setChecked(false);
    if (m_cbMercType) m_cbMercType->setCurrentIndex(-1);
    if (m_cbMercName) m_cbMercName->setCurrentIndex(-1);
    if (m_edMercExp) m_edMercExp->clear();
    if (m_chMercDead) m_chMercDead->setChecked(false);

    m_lstRecycle->clear();
    m_hasCharacter = false;
    setD2R(true);
    update();
}

void ItemGridWidget::loadText() {
    if (!g_dataMgr) return;
    if (m_chMercenary) m_chMercenary->setText(g_dataMgr->charItemsUI(5));
    if (m_chCorpse) m_chCorpse->setText(g_dataMgr->charItemsUI(4));

    // Merc type
    int sel = m_cbMercType ? m_cbMercType->currentIndex() : -1;
    if (m_cbMercType) {
        m_cbMercType->clear();
        for (UINT i = 0; i < g_dataMgr->mercenaryTypeNameSize(); ++i)
            m_cbMercType->addItem(g_dataMgr->mercenaryTypeName(i));
        if (sel >= 0) m_cbMercType->setCurrentIndex(sel);
    }
    m_mercNameGroup = -1;
    refreshMercNames();
}

void ItemGridWidget::addItemInGrid(const CD2Item& item, int body) {
    EEquip equip = itemToEquip(item);
    auto t = itemToPosition(item.iLocation, item.iPosition, item.iColumn, item.iRow, item.iStoredIn, body, m_isD2R);
    EPosition pos = std::get<0>(t);
    int x = std::get<1>(t), y = std::get<2>(t);
    int index = m_itemViews.size();
    m_itemViews.emplace_back(item, equip, pos, x, y);

    if (isInMouse(pos)) {
        m_pickedItemIndex = index;
        m_itemCursor = QCursor(createItemCursor(m_itemViews[index]));
        setCursor(m_itemCursor);
    } else {
        m_gridViews[pos].putItem(index, x, y, m_itemViews[index].gridWidth, m_itemViews[index].gridHeight, equip);
    }

    // gems
    if (!item.aGemItems.empty()) {
        for (int g = 0; g < item.aGemItems.size() && g < m_itemViews[index].gemItems.size(); ++g) {
            auto& gem = item.aGemItems[g];
            if (gem.iColumn < 0 || gem.iColumn >= m_itemViews[index].gemItems.size())
                continue;
            m_itemViews[index].gemItems[gem.iColumn] = m_itemViews.size();
            m_itemViews.emplace_back(gem, itemToEquip(gem), IN_SOCKET, gem.iColumn, 0);
        }
    }
}

QPoint ItemGridWidget::getItemPositionXY(const CItemView& view) const {
    if (view.position >= POSITION_END) return QPoint();
    return m_gridViews[view.position].indexToXY(view.gridX, view.gridY, view.gridWidth, view.gridHeight);
}

CItemView& ItemGridWidget::selectedParentItemView() {
    return m_itemViews[m_selectedItemIndex];
}

CItemView& ItemGridWidget::selectedItemView() {
    if (m_selectedSocketIndex < 0)
        return selectedParentItemView();
    return m_itemViews[m_selectedSocketIndex];
}

CItemView& ItemGridWidget::pickedItemView() {
    return m_itemViews[m_pickedItemIndex];
}

QPixmap ItemGridWidget::createItemCursor(const CItemView& view) {
    auto* img = ImageManager::instance();
    QPixmap pix = img->getPixmap(view.picIndex);
    if (pix.isNull()) return QPixmap();
    QSize sz = view.viewSize();
    return pix.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

// ==================== 雇佣兵 ====================

int ItemGridWidget::mercNameGroup(int name) {
    if (name < 0) return -1;
    static const int NAME_INDEX[] = {5, 14, 23, 29, 35};
    static const int TYPE_INDEX[] = {0, 1, 2, 3, 1, 3};
    int type = std::lower_bound(std::begin(NAME_INDEX), std::end(NAME_INDEX), name) - std::begin(NAME_INDEX);
    return TYPE_INDEX[type];
}

void ItemGridWidget::onMercTypeChanged() {
    int curIdx = m_cbMercType->currentIndex();
    int g = mercNameGroup(curIdx);
    if (m_mercNameGroup == g) return;
    m_mercNameGroup = g;
    refreshMercNames();
}

void ItemGridWidget::refreshMercNames() {
    if (!m_cbMercName || !g_dataMgr) return;
    int sel = m_cbMercName->currentIndex();
    m_cbMercName->clear();

    auto fillNames = [&](auto nameFn, UINT count, bool needEmpty) {
        for (UINT i = 0; i < count; ++i) {
            if (needEmpty && !m_isD2R && i == 0)
                m_cbMercName->addItem(QString());
            else
                m_cbMercName->addItem(nameFn(i));
        }
    };

    switch (m_mercNameGroup) {
        case 0: fillNames([&](UINT i) { return g_dataMgr->mercenaryScoutName(i); },
            g_dataMgr->mercenaryNameScoutSize(), true); break;
        case 1: fillNames([&](UINT i) { return g_dataMgr->mercenaryMercName(i); },
            g_dataMgr->mercenaryNameMercSize(), true); break;
        case 2: fillNames([&](UINT i) { return g_dataMgr->mercenarySorcerorName(i); },
            g_dataMgr->mercenaryNameSorcerorSize(), true); break;
        case 3: fillNames([&](UINT i) { return g_dataMgr->mercenaryBarbarianName(i); },
            g_dataMgr->mercenaryNameBarbarianSize(), true); break;
        default: break;
    }
    if (sel >= 0 && sel < m_cbMercName->count())
        m_cbMercName->setCurrentIndex(sel);
}

void ItemGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // Simple: just repaint, positions are fixed offsets
    update();
}
