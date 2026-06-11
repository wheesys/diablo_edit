#include "FoundryDialog.h"
#include "core/ImageManager.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QApplication>

FoundryDialog::FoundryDialog(CD2Item& item, QWidget* parent)
    : QDialog(parent)
    , m_item(item)
{
    setWindowTitle(g_dataMgr ? g_dataMgr->foundryUI(0) : QStringLiteral("Foundry"));
    setMinimumSize(700, 550);
    setupUI();
    loadFromItem();
}

FoundryDialog::~FoundryDialog() = default;

void FoundryDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // === Upper: Scroll area with all controls ===
    auto* scroll = new QScrollArea;
    auto* scrollWidget = new QWidget;
    auto* formLayout = new QGridLayout(scrollWidget);
    formLayout->setColumnMinimumWidth(1, 100);

    int row = 0;

    // Quality
    formLayout->addWidget(new QLabel(QStringLiteral("Quality:")), row, 0);
    m_cbQuality = new QComboBox;
    for (UINT i = 0; i < g_dataMgr->itemQualityNameSize(); ++i)
        m_cbQuality->addItem(g_dataMgr->itemQualityName(i));
    formLayout->addWidget(m_cbQuality, row++, 1);

    // Class
    formLayout->addWidget(new QLabel(QStringLiteral("Class:")), row, 0);
    m_cbClass = new QComboBox;
    m_cbClass->addItem(QString()); // none
    for (UINT i = 0; i < DataManager::CLASS_NAME_SIZE; ++i)
        m_cbClass->addItem(g_dataMgr->className(i));
    formLayout->addWidget(m_cbClass, row++, 1);

    // Level
    formLayout->addWidget(new QLabel(QStringLiteral("Drop Level:")), row, 0);
    m_edLevel = new QLineEdit;
    m_edLevel->setMaximumWidth(80);
    formLayout->addWidget(m_edLevel, row++, 1);

    // Unidentified
    m_chUnidentified = new QCheckBox(QStringLiteral("Unidentified"));
    formLayout->addWidget(m_chUnidentified, row++, 0, 1, 2);

    // Ethereal
    m_chEthereal = new QCheckBox(QStringLiteral("Ethereal"));
    formLayout->addWidget(m_chEthereal, row++, 0, 1, 2);

    // Indestructible
    m_chIndestructible = new QCheckBox(QStringLiteral("Indestructible"));
    formLayout->addWidget(m_chIndestructible, row++, 0, 1, 2);

    // Durability
    formLayout->addWidget(new QLabel(QStringLiteral("Max Durability:")), row, 0);
    m_edMaxDurability = new QLineEdit;
    m_edMaxDurability->setMaximumWidth(80);
    formLayout->addWidget(m_edMaxDurability, row++, 1);

    formLayout->addWidget(new QLabel(QStringLiteral("Durability:")), row, 0);
    m_edDurability = new QLineEdit;
    m_edDurability->setMaximumWidth(80);
    formLayout->addWidget(m_edDurability, row++, 1);

    // Quantity
    formLayout->addWidget(new QLabel(QStringLiteral("Quantity:")), row, 0);
    m_edQuantity = new QLineEdit;
    m_edQuantity->setMaximumWidth(80);
    formLayout->addWidget(m_edQuantity, row++, 1);

    // Defence
    formLayout->addWidget(new QLabel(QStringLiteral("Defence:")), row, 0);
    m_edDefence = new QLineEdit;
    m_edDefence->setMaximumWidth(80);
    formLayout->addWidget(m_edDefence, row++, 1);

    // Sockets
    formLayout->addWidget(new QLabel(QStringLiteral("Sockets:")), row, 0);
    auto* sockLayout = new QHBoxLayout;
    m_edSockets = new QLineEdit;
    m_edSockets->setMaximumWidth(60);
    sockLayout->addWidget(m_edSockets);
    m_lblExtSockets = new QLabel(QStringLiteral("+0 ext"));
    sockLayout->addWidget(m_lblExtSockets);
    sockLayout->addStretch();
    formLayout->addLayout(sockLayout, row++, 1);

    // Owner
    formLayout->addWidget(new QLabel(QStringLiteral("Owner:")), row, 0);
    m_edOwner = new QLineEdit;
    m_edOwner->setMaximumWidth(120);
    formLayout->addWidget(m_edOwner, row++, 1);

    // Monster
    formLayout->addWidget(new QLabel(QStringLiteral("Monster:")), row, 0);
    m_cbMonster = new QComboBox;
    m_cbMonster->addItem(QString()); // none
    for (UINT i = 0; i < g_dataMgr->monsterNameSize(); ++i)
        m_cbMonster->addItem(g_dataMgr->monsterName(i));
    formLayout->addWidget(m_cbMonster, row++, 1);

    // First/Second name (rare/crafted)
    for (int i = 0; i < 2; ++i) {
        formLayout->addWidget(new QLabel(i == 0 ? QStringLiteral("First Name:") : QStringLiteral("Second Name:")), row, 0);
        m_cbRareName[i] = new QComboBox;
        m_cbRareName[i]->addItem(QString()); // none
        for (UINT j = 0; j < g_dataMgr->rareCraftedNameSize(); ++j)
            m_cbRareName[i]->addItem(g_dataMgr->rareCraftedName(j));
        formLayout->addWidget(m_cbRareName[i], row++, 1);
    }

    // Prefix/Suffix combos
    for (int i = 0; i < 3; ++i) {
        formLayout->addWidget(new QLabel(QStringLiteral("Prefix %1:").arg(i + 1)), row, 0);
        m_cbPrefix[i] = new QComboBox;
        m_cbPrefix[i]->addItem(QString()); // none
        for (UINT j = 0; j < g_dataMgr->magicPrefixSize(); ++j)
            m_cbPrefix[i]->addItem(g_dataMgr->magicPrefix(j));
        formLayout->addWidget(m_cbPrefix[i], row++, 1);

        formLayout->addWidget(new QLabel(QStringLiteral("Suffix %1:").arg(i + 1)), row, 0);
        m_cbSuffix[i] = new QComboBox;
        m_cbSuffix[i]->addItem(QString());
        for (UINT j = 0; j < g_dataMgr->magicSuffixSize(); ++j)
            m_cbSuffix[i]->addItem(g_dataMgr->magicSuffix(j));
        formLayout->addWidget(m_cbSuffix[i], row++, 1);
    }

    // Set Name
    formLayout->addWidget(new QLabel(QStringLiteral("Set Name:")), row, 0);
    m_cbSetName = new QComboBox;
    m_cbSetName->addItem(QString());
    for (UINT j = 0; j < g_dataMgr->setItemNameSize(); ++j)
        m_cbSetName->addItem(g_dataMgr->setItemName(j));
    formLayout->addWidget(m_cbSetName, row++, 1);

    // Unique Name
    formLayout->addWidget(new QLabel(QStringLiteral("Unique Name:")), row, 0);
    m_cbUniqueName = new QComboBox;
    m_cbUniqueName->addItem(QString());
    for (UINT j = 0; j < g_dataMgr->uniqueNameSize(); ++j)
        m_cbUniqueName->addItem(g_dataMgr->uniqueName(j));
    formLayout->addWidget(m_cbUniqueName, row++, 1);

    // RuneWord
    m_chRuneWord = new QCheckBox(QStringLiteral("Rune Word"));
    formLayout->addWidget(m_chRuneWord, row++, 0, 1, 2);

    // SubType
    formLayout->addWidget(new QLabel(QStringLiteral("SubType:")), row, 0);
    m_cbSubType = new QComboBox;
    m_cbSubType->addItem(QString());
    formLayout->addWidget(m_cbSubType, row++, 1);

    scroll->setWidget(scrollWidget);
    scroll->setMaximumHeight(250);

    // === Lower: Property Tab ===
    m_tabProp = new QTabWidget;
    m_tabProp->addTab(new QWidget, QStringLiteral("Main"));
    for (int i = 0; i < 5; ++i)
        m_tabProp->addTab(new QWidget, QStringLiteral("Set %1").arg(i + 1));
    m_tabProp->addTab(new QWidget, QStringLiteral("RuneWord"));

    m_lstProperty = new QTableWidget;
    m_lstProperty->setColumnCount(2);
    m_lstProperty->setHorizontalHeaderLabels(
        {QStringLiteral("Attribute"), QStringLiteral("Value")});
    m_lstProperty->horizontalHeader()->setStretchLastSection(true);
    m_lstProperty->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_lstProperty->setSelectionMode(QAbstractItemView::SingleSelection);

    // Param editors
    auto* paramWidget = new QWidget;
    auto* paramLayout = new QHBoxLayout(paramWidget);
    for (int i = 0; i < 4; ++i) {
        m_edParam[i] = new QLineEdit;
        m_edParam[i]->setMaximumWidth(80);
        paramLayout->addWidget(m_edParam[i]);
    }
    for (int i = 0; i < 2; ++i) {
        m_cbParam[i] = new QComboBox;
        m_cbParam[i]->setMaximumWidth(100);
        paramLayout->addWidget(m_cbParam[i]);
    }
    paramLayout->addStretch();

    mainLayout->addWidget(scroll);
    mainLayout->addWidget(m_lstProperty, 1);
    mainLayout->addWidget(paramWidget);

    // Buttons
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (saveToItem()) accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    // Signals
    connect(m_cbQuality, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FoundryDialog::onQualityChanged);
    connect(m_tabProp, &QTabWidget::currentChanged,
        this, &FoundryDialog::onPropertyTabChanged);
    connect(m_lstProperty, &QTableWidget::currentItemChanged,
        this, &FoundryDialog::onPropertyItemChanged);
}

void FoundryDialog::loadFromItem() {
    auto& meta = m_item.MetaData();
    m_cbQuality->setCurrentIndex(m_item.Quality());

    // Read extended info
    if (m_item.pItemInfo.exist() && m_item.pItemInfo->pExtItemInfo.exist()) {
        auto& ext = *m_item.pItemInfo->pExtItemInfo;
        m_chUnidentified->setChecked(!m_item.bIdentified);
        m_chEthereal->setChecked(m_item.bEthereal);
        m_edLevel->setText(QString::number(ext.iDropLevel));

        if (ext.wClass.exist())
            m_cbClass->setCurrentIndex(ext.wClass + 1); // +1 for empty
        if (ext.wMonsterID.exist())
            m_cbMonster->setCurrentIndex(ext.wMonsterID + 1);

        // Rare/Craft names
        if (ext.pRareName.exist()) {
            auto& name = *ext.pRareName;
            m_cbRareName[0]->setCurrentIndex(name.iName1 + 1);
            m_cbRareName[1]->setCurrentIndex(name.iName2 + 1);
        }
        if (ext.pCraftName.exist()) {
            auto& name = *ext.pCraftName;
            m_cbRareName[0]->setCurrentIndex(name.iName1 + 1);
            m_cbRareName[1]->setCurrentIndex(name.iName2 + 1);
        }

        // Prefix/Suffix
        auto setPS = [](bool exist, MayExist<WORD>& idx, QComboBox* cb) {
            if (exist) cb->setCurrentIndex(idx + 1);
        };
        if (ext.pRareName.exist()) {
            auto& n = *ext.pRareName;
            setPS(n.bPref1, n.wPref1, m_cbPrefix[0]);
            setPS(n.bSuff1, n.wSuff1, m_cbSuffix[0]);
            setPS(n.bPref2, n.wPref2, m_cbPrefix[1]);
            setPS(n.bSuff2, n.wSuff2, m_cbSuffix[1]);
            setPS(n.bPref3, n.wPref3, m_cbPrefix[2]);
            setPS(n.bSuff3, n.wSuff3, m_cbSuffix[2]);
        }
        if (ext.wUniID.exist())
            m_cbUniqueName->setCurrentIndex((ext.wUniID & 0xFFFF) + 1);
        if (ext.wSetID.exist())
            m_cbSetName->setCurrentIndex(ext.wSetID + 1);
    }

    // Type specific info
    if (m_item.pItemInfo.exist() && m_item.pItemInfo->pTpSpInfo.exist()) {
        auto& spec = *m_item.pItemInfo->pTpSpInfo;
        if (spec.iDefence.exist())
            m_edDefence->setText(QString::number(spec.GetDefence()));
        if (spec.iMaxDurability.exist())
            m_edMaxDurability->setText(QString::number((WORD)spec.iMaxDurability));
        if (spec.iCurDur.exist())
            m_edDurability->setText(QString::number((WORD)spec.iCurDur));
        if (spec.iSocket.exist())
            m_edSockets->setText(QString::number((int)(BYTE)spec.iSocket));
        if (spec.iQuantity.exist())
            m_edQuantity->setText(QString::number((WORD)spec.iQuantity));

        m_props[0] = {spec.stPropertyList, true};
        for (int i = 0; i < 5; ++i) {
            if (spec.aHasSetPropList.exist()) {
                spec.aHasSetPropList.ensure();
                if (i < (int)spec.aHasSetPropList.size())
                    m_props[i + 1].second = spec.aHasSetPropList[i] != 0;
            }
            if (m_props[i + 1].second && spec.apSetProperty[i].exist())
                m_props[i + 1].first = *spec.apSetProperty[i];
        }
        m_props[6].second = spec.stRuneWordPropertyList.exist();
        if (m_props[6].second)
            m_props[6].first = *spec.stRuneWordPropertyList;
    }

    m_chRuneWord->setChecked(m_item.IsRuneWord());
    m_chIndestructible->setChecked(
        m_item.HasPropertyList() && m_item.pItemInfo->pTpSpInfo->IsIndestructible());

    onPropertyTabChanged(0);
}

bool FoundryDialog::saveToItem() {
    // Validate
    validate(m_cbQuality->currentIndex() >= 0, 48);
    validate((!m_chRuneWord->isChecked()) || m_chRuneWord->isChecked(), 49);

    // Quality
    if (m_item.pItemInfo.exist() && m_item.pItemInfo->pExtItemInfo.exist())
        m_item.pItemInfo->pExtItemInfo->iQuality = m_cbQuality->currentIndex();

    m_item.bIdentified = !m_chUnidentified->isChecked();
    m_item.bEthereal = m_chEthereal->isChecked();

    // Ext item info
    if (m_item.pItemInfo.exist() && m_item.pItemInfo->pExtItemInfo.exist()) {
        auto& ext = *m_item.pItemInfo->pExtItemInfo;
        ext.iDropLevel = m_edLevel->text().toInt();
        if (m_cbClass->currentIndex() > 0) {
            ext.bClass = true;
            ext.wClass.ensure() = m_cbClass->currentIndex() - 1;
        } else {
            ext.bClass = false;
            ext.wClass.reset();
        }
        if (m_cbMonster->currentIndex() > 0) {
            ext.wMonsterID.ensure() = m_cbMonster->currentIndex() - 1;
        } else {
            ext.wMonsterID.reset();
        }
        if (m_cbSetName->currentIndex() > 0)
            ext.wSetID.ensure() = m_cbSetName->currentIndex() - 1;
        else ext.wSetID.reset();
        if (m_cbUniqueName->currentIndex() > 0)
            ext.wUniID.ensure() = m_cbUniqueName->currentIndex() - 1;
        else ext.wUniID.reset();
    }

    // Type specific
    if (m_item.pItemInfo.exist() && m_item.pItemInfo->pTpSpInfo.exist()) {
        auto& spec = *m_item.pItemInfo->pTpSpInfo;
        if (!m_edDefence->text().isEmpty())
            spec.SetDefence(m_edDefence->text().toInt());
        if (!m_edMaxDurability->text().isEmpty() && spec.iMaxDurability.exist())
            spec.iMaxDurability.ensure() = m_edMaxDurability->text().toUInt();
        if (!m_edDurability->text().isEmpty() && spec.iCurDur.exist())
            spec.iCurDur.ensure() = m_edDurability->text().toUInt();
        if (!m_edQuantity->text().isEmpty() && spec.iQuantity.exist())
            spec.iQuantity.ensure() = m_edQuantity->text().toUInt();
        if (!m_edSockets->text().isEmpty() && spec.iSocket.exist())
            spec.iSocket.ensure() = m_edSockets->text().toUInt();

        spec.stPropertyList = m_props[0].first;
        if (!spec.aHasSetPropList.exist() || !spec.apSetProperty) {
            // need to preserve - just skip set props for now
        } else {
            spec.aHasSetPropList.ensure();
            for (int i = 0; i < 5 && i < (int)spec.aHasSetPropList.size(); ++i) {
                spec.aHasSetPropList[i] = m_props[i + 1].second ? 1 : 0;
                if (m_props[i + 1].second && spec.apSetProperty[i].exist())
                    *spec.apSetProperty[i] = m_props[i + 1].first;
            }
        }
        if (m_props[6].second && spec.stRuneWordPropertyList.exist())
            *spec.stRuneWordPropertyList = m_props[6].first;
    }

    m_item.bRuneWord = m_chRuneWord->isChecked();
    return true;
}

void FoundryDialog::validate(bool expr, int msgId) {
    if (!expr) {
        QMessageBox::critical(this,
            g_dataMgr->msgError(),
            g_dataMgr->msgBoxInfo(msgId));
    }
}

// ===== property list display =====

void FoundryDialog::onPropertyTabChanged(int index) {
    updatePropertyList();
}

void FoundryDialog::updatePropertyList() {
    m_lstProperty->setRowCount(0);
    int tabIdx = m_tabProp->currentIndex();
    if (tabIdx < 0 || tabIdx > 6) return;

    auto& prop = m_props[tabIdx];
    if (!prop.second) return;

    const DWORD ver = m_item.dwVersion;
    for (size_t i = 0; i < prop.first.mProperty.size(); ++i) {
        WORD id = prop.first.mProperty[i].first;
        DWORD val = prop.first.mProperty[i].second;

        m_lstProperty->insertRow((int)i);
        QString desc = g_dataMgr->propertyDescription(ver, id, val);
        auto* descItem = new QTableWidgetItem(desc);
        auto* idItem = new QTableWidgetItem(QString::number(id));
        idItem->setData(Qt::UserRole, QVariant::fromValue(val));
        m_lstProperty->setItem((int)i, 0, descItem);
        m_lstProperty->setItem((int)i, 1, idItem);
    }
    m_lstProperty->resizeColumnsToContents();
}

void FoundryDialog::onPropertyItemChanged(QTableWidgetItem* current, QTableWidgetItem* prev) {
    Q_UNUSED(prev);
    if (!current) return;
    int row = current->row();
    auto* idItem = m_lstProperty->item(row, 1);
    if (!idItem) return;
    WORD id = idItem->text().toUInt();
    DWORD val = idItem->data(Qt::UserRole).value<DWORD>();

    auto params = g_dataMgr->propertyParameters(m_item.dwVersion, id, val);
    for (int i = 0; i < 4; ++i) {
        if (m_edParam[i]) {
            m_edParam[i]->setVisible(i < (int)params.size());
            if (i < (int)params.size()) {
                m_edParam[i]->setText(QString::number(params[i].iValue));
            } else {
                m_edParam[i]->clear();
            }
        }
    }
    for (int i = 0; i < 2; ++i) {
        if (m_cbParam[i]) m_cbParam[i]->setVisible(false);
    }
}

void FoundryDialog::onQualityChanged(int index) {
    Q_UNUSED(index);
    updatePropertyList();
}
