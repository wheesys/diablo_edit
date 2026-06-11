#include "QuestInfoWidget.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

const WORD QuestInfoWidget::QUEST_COMPLETE[27] = {
    0x1001, 0x101d, 0x900d, 0x101d, 0x1055, 0x101d,   // Act I
    0x101d, 0x1c39, 0x100d, 0x1181, 0x1005, 0x1e25,   // Act II
    0x1001, 0x10fd, 0x11d9, 0x1001, 0x100d, 0x1871,   // Act III
    0x1001, 0x1301, 0x1001,                            // Act IV
    0x9021, 0x1001, 0x178d, 0x901d, 0x132d, 0x169d    // Act V
};

QuestInfoWidget::QuestInfoWidget(QWidget* parent)
    : QWidget(parent)
    , m_nLevel(0)
{
    ::ZeroMemory(m_bUIData, sizeof(m_bUIData));
    for (int i = 0; i < LEVEL_SIZE; ++i)
        ::ZeroMemory(m_bQuestInfo[i], sizeof(m_bQuestInfo[0]));
    setupUI();
}

QuestInfoWidget::~QuestInfoWidget() = default;

void QuestInfoWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // === 难度选择 ===
    auto* topRow = new QHBoxLayout;
    m_grpDifficulty = new QButtonGroup(this);
    auto* rbNormal = new QRadioButton(QStringLiteral("Normal"));
    auto* rbNM = new QRadioButton(QStringLiteral("Nightmare"));
    auto* rbHell = new QRadioButton(QStringLiteral("Hell"));
    m_grpDifficulty->addButton(rbNormal, 0);
    m_grpDifficulty->addButton(rbNM, 1);
    m_grpDifficulty->addButton(rbHell, 2);
    rbNormal->setChecked(true);
    topRow->addWidget(rbNormal);
    topRow->addWidget(rbNM);
    topRow->addWidget(rbHell);
    topRow->addStretch();
    mainLayout->addLayout(topRow);

    // === 任务分组 ===
    // Act I:  索引 0-5 (6个) + 28 (ResetStats)
    // Act II: 索引 6-11 (6个)
    // Act III: 12-17 (6个)
    // Act IV: 18-20 (3个)
    // Act V:  21-26 (6个)
    // 附加:   27

    auto* mainGrid = new QGridLayout;

    auto addActGroup = [&](int startIdx, int count, const QString& title,
                           int row, int col) {
        auto* grp = new QGroupBox(title);
        auto* grid = new QGridLayout(grp);
        for (int i = 0; i < count; ++i) {
            m_chkQuests[startIdx + i] = new QCheckBox;
            grid->addWidget(m_chkQuests[startIdx + i], i, 0, Qt::AlignLeft);
        }
        mainGrid->addWidget(grp, row, col, 1, 1, Qt::AlignTop);
        return grp;
    };

    int row = 0;

    // 第0行：Act I + Act II
    addActGroup(0, 6, QStringLiteral("Act I"), row, 0);
    addActGroup(6, 6, QStringLiteral("Act II"), row, 1);

    // 第1行：Act III + Act IV
    row = 1;
    addActGroup(12, 6, QStringLiteral("Act III"), row, 0);
    addActGroup(18, 3, QStringLiteral("Act IV"), row, 1);

    // 第2行：Act V + 附加
    row = 2;
    addActGroup(21, 6, QStringLiteral("Act V"), row, 0);

    auto* grpExtra = new QGroupBox(QStringLiteral("Extra"));
    auto* extraGrid = new QGridLayout(grpExtra);
    m_chkQuests[27] = new QCheckBox;  // 附加
    m_chkQuests[28] = new QCheckBox(QStringLiteral("Reset Stats"));
    extraGrid->addWidget(m_chkQuests[27], 0, 0);
    extraGrid->addWidget(m_chkQuests[28], 1, 0);
    mainGrid->addWidget(grpExtra, row, 1, Qt::AlignTop);

    mainLayout->addLayout(mainGrid);
    mainLayout->addStretch();

    // === 信号连接 ===
    connect(m_grpDifficulty, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &QuestInfoWidget::setDifficulty);
    connect(m_chkQuests[0], &QCheckBox::stateChanged,
            this, &QuestInfoWidget::updateResetStats);
}

void QuestInfoWidget::updateUI(const CD2S_Struct& character)
{
    for (int i = 0; i < LEVEL_SIZE; ++i) {
        for (int j = 0; j < 6; ++j) {
            m_bQuestInfo[i][j] = (character.QuestInfo.QIData[i].wActI[j] & 1) != 0;
            m_bQuestInfo[i][6 + j] = (character.QuestInfo.QIData[i].wActII[j] & 1) != 0;
            m_bQuestInfo[i][12 + j] = (character.QuestInfo.QIData[i].wActIII[j] & 1) != 0;
            m_bQuestInfo[i][21 + j] = (character.QuestInfo.QIData[i].wActV[j] & 1) != 0;
            if (j < 3)
                m_bQuestInfo[i][18 + j] = (character.QuestInfo.QIData[i].wActIV[j] & 1) != 0;
        }
        m_bQuestInfo[i][27] = (character.QuestInfo.QIData[i].wActI[3] & 0x400) != 0;
        m_bQuestInfo[i][28] = character.QuestInfo.QIData[i].bResetStats == 1;
    }
    ::CopyMemory(m_bUIData, m_bQuestInfo[m_nLevel], sizeof(m_bUIData));

    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_chkQuests[i]->setChecked(m_bUIData[i]);
    updateResetStats();
}

bool QuestInfoWidget::gatherData(CD2S_Struct& character)
{
    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_bUIData[i] = m_chkQuests[i]->isChecked();
    ::CopyMemory(m_bQuestInfo[m_nLevel], m_bUIData, sizeof(m_bUIData));

    for (int i = 0; i < LEVEL_SIZE; ++i) {
        for (int j = 0; j < 6; ++j) {
            if (((character.QuestInfo.QIData[i].wActI)[j] & 1) != m_bQuestInfo[i][j])
                (character.QuestInfo.QIData[i].wActI)[j] = (m_bQuestInfo[i][j] ? QUEST_COMPLETE[j] : 0);
            if (((character.QuestInfo.QIData[i].wActII)[j] & 1) != m_bQuestInfo[i][6 + j])
                (character.QuestInfo.QIData[i].wActII)[j] = (m_bQuestInfo[i][6 + j] ? QUEST_COMPLETE[6 + j] : 0);
            if (((character.QuestInfo.QIData[i].wActIII)[j] & 1) != m_bQuestInfo[i][12 + j])
                (character.QuestInfo.QIData[i].wActIII)[j] = (m_bQuestInfo[i][12 + j] ? QUEST_COMPLETE[12 + j] : 0);
            if (((character.QuestInfo.QIData[i].wActV)[j] & 1) != m_bQuestInfo[i][21 + j])
                (character.QuestInfo.QIData[i].wActV)[j] = (m_bQuestInfo[i][21 + j] ? QUEST_COMPLETE[21 + j] : 0);
            if (j < 3 && ((character.QuestInfo.QIData[i].wActIV)[j] & 1) != m_bQuestInfo[i][18 + j])
                (character.QuestInfo.QIData[i].wActIV)[j] = (m_bQuestInfo[i][18 + j] ? QUEST_COMPLETE[18 + j] : 0);
        }
        m_bQuestInfo[i][27]
            ? (character.QuestInfo.QIData[i].wActI[3] |= 0x400)
            : (character.QuestInfo.QIData[i].wActI[3] &= ~0x400);
        character.QuestInfo.QIData[i].bResetStats
            = m_bQuestInfo[i][0] ? (m_bQuestInfo[i][28] ? 1 : 2) : 0;
    }
    return true;
}

void QuestInfoWidget::resetAll()
{
    m_nLevel = 0;
    for (int i = 0; i < LEVEL_SIZE; ++i)
        ::ZeroMemory(m_bQuestInfo[i], sizeof(m_bQuestInfo[i]));
    ::ZeroMemory(m_bUIData, sizeof(m_bUIData));

    auto* rb = qobject_cast<QRadioButton*>(m_grpDifficulty->button(0));
    if (rb) rb->setChecked(true);
    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_chkQuests[i]->setChecked(m_bUIData[i]);
    updateResetStats();
}

void QuestInfoWidget::loadText()
{
    if (!g_dataMgr) return;

    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_chkQuests[i]->setText(g_dataMgr->questName(i));

    auto setRbText = [&](int id, const QString& text) {
        auto* rb = qobject_cast<QRadioButton*>(m_grpDifficulty->button(id));
        if (rb) rb->setText(text);
    };
    for (UINT i = 0; i < LEVEL_SIZE; ++i)
        setRbText(static_cast<int>(i), g_dataMgr->difficultyName(i));

    updateResetStats();
}

void QuestInfoWidget::setDifficulty(int level)
{
    if (m_nLevel == level) return;

    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_bUIData[i] = m_chkQuests[i]->isChecked();
    ::CopyMemory(m_bQuestInfo[m_nLevel], m_bUIData, sizeof(m_bUIData));

    m_nLevel = level;
    ::CopyMemory(m_bUIData, m_bQuestInfo[level], sizeof(m_bUIData));

    for (int i = 0; i < QUEST_NAME_SIZE; ++i)
        m_chkQuests[i]->setChecked(m_bUIData[i]);
    updateResetStats();
}

void QuestInfoWidget::updateResetStats()
{
    m_chkQuests[28]->setEnabled(m_chkQuests[0]->isChecked());
}
