#include "SelectCharDialog.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

SelectCharDialog::SelectCharDialog(int& sel, QWidget* parent)
    : QDialog(parent)
    , m_nSel(sel)
{
    setWindowTitle(g_dataMgr ? g_dataMgr->otherUI(0) : QStringLiteral("Select Class"));

    auto* mainLayout = new QVBoxLayout(this);

    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel(QStringLiteral("Class:")));
    m_cbSelectChar = new QComboBox;
    if (g_dataMgr) {
        for (UINT i = 0; i < DataManager::CLASS_NAME_SIZE; ++i)
            m_cbSelectChar->addItem(g_dataMgr->className(i));
    }
    m_cbSelectChar->setCurrentIndex(0);
    row->addWidget(m_cbSelectChar);
    mainLayout->addLayout(row);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        m_nSel = m_cbSelectChar->currentIndex();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SelectCharDialog::~SelectCharDialog() = default;
