#include "SkillsDialog.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>

SkillsDialog::SkillsDialog(int charClass, BYTE skills[30], QWidget* parent)
    : QDialog(parent)
    , m_nCharClass(charClass)
    , m_pData(skills)
    , m_bAll(0)
{
    // 从绝对索引转换为相对索引
    for (int i = 0; i < SKILL_SIZE; ++i)
        m_pSkills[DataManager::CLASS_SKILL_INDEX[m_nCharClass][i]] = m_pData[i];

    setupUI();
    loadText();
}

SkillsDialog::~SkillsDialog() = default;

void SkillsDialog::setupUI()
{
    setMinimumWidth(500);

    auto* mainLayout = new QVBoxLayout(this);

    // 3 个技能树，每树 10 个技能
    auto* grid = new QGridLayout;

    for (int tab = 0; tab < TAB_SIZE; ++tab) {
        auto* grp = new QGroupBox;
        auto* grpLayout = new QVBoxLayout(grp);

        m_tabLabels[tab] = new QLabel;
        m_tabLabels[tab]->setStyleSheet(QStringLiteral("font-weight: bold;"));
        grpLayout->addWidget(m_tabLabels[tab]);

        for (int i = 0; i < 10; ++i) {
            int idx = tab * 10 + i;
            auto* row = new QHBoxLayout;
            m_skillLabels[idx] = new QLabel;
            row->addWidget(m_skillLabels[idx]);
            m_spinSkills[idx] = new QSpinBox;
            m_spinSkills[idx]->setRange(0, 127);
            m_spinSkills[idx]->setValue(m_pSkills[idx]);
            m_spinSkills[idx]->setFixedWidth(70);
            row->addWidget(m_spinSkills[idx]);
            row->addStretch();
            grpLayout->addLayout(row);
        }

        grid->addWidget(grp, 0, tab);
    }

    mainLayout->addLayout(grid);

    // 批量设置
    auto* setAllRow = new QHBoxLayout;
    auto* btnSetAll = new QPushButton(QStringLiteral("Set All"));
    m_spinSetAll = new QSpinBox;
    m_spinSetAll->setRange(0, 127);
    m_spinSetAll->setValue(m_bAll);
    setAllRow->addWidget(btnSetAll);
    setAllRow->addWidget(m_spinSetAll);
    setAllRow->addStretch();
    mainLayout->addLayout(setAllRow);

    // 按钮
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    // 连接
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        for (int i = 0; i < SKILL_SIZE; ++i)
            m_pSkills[i] = static_cast<BYTE>(m_spinSkills[i]->value());

        for (int i = 0; i < SKILL_SIZE; ++i) {
            if (m_pSkills[i] > 127) {
                QMessageBox::warning(this,
                    g_dataMgr ? g_dataMgr->msgWarning() : QStringLiteral("Warning"),
                    g_dataMgr ? g_dataMgr->msgBoxInfo(5) : QStringLiteral("Skill value cannot exceed 127."));
                return;
            }
        }

        for (int i = 0; i < SKILL_SIZE; ++i)
            m_pData[i] = m_pSkills[DataManager::CLASS_SKILL_INDEX[m_nCharClass][i]];

        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(btnSetAll, &QPushButton::clicked, this, [this]() {
        BYTE val = static_cast<BYTE>(m_spinSetAll->value());
        if (val > 127) {
            QMessageBox::warning(this,
                g_dataMgr ? g_dataMgr->msgWarning() : QStringLiteral("Warning"),
                g_dataMgr ? g_dataMgr->msgBoxInfo(5) : QStringLiteral("Skill value cannot exceed 127."));
            return;
        }
        m_bAll = val;
        for (int i = 0; i < SKILL_SIZE; ++i) {
            m_pSkills[i] = m_bAll;
            m_spinSkills[i]->setValue(m_bAll);
        }
    });
}

void SkillsDialog::loadText()
{
    if (!g_dataMgr) return;

    setWindowTitle(g_dataMgr->otherUI(1 + m_nCharClass));

    for (int tab = 0; tab < TAB_SIZE; ++tab) {
        m_tabLabels[tab]->setText(g_dataMgr->classSkillTabName(tab, m_nCharClass));
        for (int i = 0; i < 10; ++i) {
            int idx = tab * 10 + i;
            m_skillLabels[idx]->setText(g_dataMgr->classSkillName(idx, m_nCharClass));
        }
    }
}
