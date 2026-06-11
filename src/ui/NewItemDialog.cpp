#include "NewItemDialog.h"
#include "core/ImageManager.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSplitter>
#include <QPixmap>

NewItemDialog::NewItemDialog(std::unique_ptr<CD2Item>& outItem, QWidget* parent)
    : QDialog(parent)
    , m_outItem(outItem)
{
    setWindowTitle(g_dataMgr ? g_dataMgr->newItemUI(0) : QStringLiteral("New Item"));
    setMinimumSize(500, 400);
    setupUI();
    loadCategories();
}

NewItemDialog::~NewItemDialog() = default;

void NewItemDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* splitter = new QSplitter(Qt::Horizontal);

    // 左侧：分类树
    m_tree = new QTreeWidget;
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(true);
    splitter->addWidget(m_tree);

    // 右侧：预览区
    auto* rightPanel = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightPanel);
    m_previewLabel = new QLabel;
    m_previewLabel->setFixedSize(60, 60);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    rightLayout->addWidget(m_previewLabel);

    m_itemNameLabel = new QLabel;
    m_itemNameLabel->setAlignment(Qt::AlignCenter);
    m_itemNameLabel->setWordWrap(true);
    rightLayout->addWidget(m_itemNameLabel);

    rightLayout->addStretch();
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // 按钮
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &NewItemDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    connect(m_tree, &QTreeWidget::currentItemChanged, this, &NewItemDialog::onTreeSelectionChanged);
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &NewItemDialog::onTreeDoubleClicked);
}

void NewItemDialog::loadCategories() {
    if (!g_dataMgr) return;

    auto& categories = g_dataMgr->itemCategories();
    for (size_t catIdx = 0; catIdx < categories.size(); ++catIdx) {
        auto& category = categories[catIdx];
        if (category.empty()) continue;

        auto* catItem = new QTreeWidgetItem(m_tree, QStringList(
            g_dataMgr->itemCatagoryName((UINT)catIdx)));

        for (size_t itemIdx = 0; itemIdx < category.size(); ++itemIdx) {
            auto& meta = category[itemIdx];
            QString name = g_dataMgr->itemName(meta.NameIndex);
            auto* itemNode = new QTreeWidgetItem(catItem, QStringList(name));
            itemNode->setData(0, Qt::UserRole, QVariant::fromValue(meta.dwTypeID));
        }
    }
    m_tree->expandAll();
}

void NewItemDialog::onTreeSelectionChanged() {
    auto* item = m_tree->currentItem();
    if (!item || !item->data(0, Qt::UserRole).isValid()) {
        m_previewLabel->clear();
        m_itemNameLabel->clear();
        m_displayPicIndex = -1;
        return;
    }

    DWORD typeID = item->data(0, Qt::UserRole).value<DWORD>();
    auto* meta = g_dataMgr->itemMetaData(typeID);
    if (!meta) return;

    m_displayPicIndex = meta->PicIndex;
    QPixmap pix = ImageManager::instance()->getPixmap(m_displayPicIndex);
    if (!pix.isNull()) {
        m_previewLabel->setPixmap(pix.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_previewLabel->clear();
    }
    m_itemNameLabel->setText(g_dataMgr->itemName(meta->NameIndex));
}

void NewItemDialog::onTreeDoubleClicked(QTreeWidgetItem* item, int /*column*/) {
    if (!item || !item->data(0, Qt::UserRole).isValid()) return;
    accept();
}

void NewItemDialog::accept() {
    auto* item = m_tree->currentItem();
    if (!item || !item->data(0, Qt::UserRole).isValid()) {
        QDialog::accept();
        return;
    }

    DWORD typeID = item->data(0, Qt::UserRole).value<DWORD>();
    m_outItem = std::make_unique<CD2Item>(typeID);
    QDialog::accept();
}
