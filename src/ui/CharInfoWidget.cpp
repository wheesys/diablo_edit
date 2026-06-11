#include "CharInfoWidget.h"
#include "WaypointsWidget.h"
#include "QuestInfoWidget.h"
#include "data/DataManager.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

// 经验值表 (1-99 级累计经验)
const DWORD LEVEL_AND_EXPERIENCE[100] = {
    0, 500, 1500, 3750, 7875, 14175, 22680, 32886, 44396, 57715,
    72144, 90180, 112725, 140906, 176132, 220165, 275207, 344008, 430010, 537513,
    671891, 839864, 1049830, 1312287, 1640359, 2050449, 2563061, 3203826, 3902260, 4663553,
    5493363, 6397855, 7383752, 8458379, 9629723, 10906488, 12298162, 13815086, 15468534, 17270791,
    19235252, 21376515, 23710491, 26254525, 29027522, 32050088, 35344686, 38935798, 42850109, 47116709,
    51767302, 56836449, 62361819, 68384473, 74949165, 82104680, 89904191, 98405658, 107672256, 117772849,
    128782495, 140783010, 153863570, 168121381, 183662396, 200602101, 219066380, 239192444, 261129853, 285041630,
    311105466, 339515048, 370481492, 404234916, 441026148, 481128591, 524840254, 572485967, 624419793, 681027665,
    742730244, 809986056, 883294891, 963201521, 1050299747, 1145236814, 1248718217, 1361512946, 1484459201, 1618470619,
    1764543065, 1923762030, 2097310703, 2286478756, 2492671933, 2717422497, 2962400612, 3229426756, 3520485254, 3837739017
};

CharInfoWidget::CharInfoWidget(QWidget* parent)
    : QWidget(parent)
{
    ::ZeroMemory(m_bSkills, sizeof(m_bSkills));
    setupUI();
    loadText();
}

CharInfoWidget::~CharInfoWidget() = default;

void CharInfoWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // === 基本信息 Group ===
    auto* grpBasic = new QGroupBox(QStringLiteral("Basic Info"));
    auto* gridBasic = new QGridLayout(grpBasic);

    m_lblVersion = new QLabel(QStringLiteral("--"));
    gridBasic->addWidget(new QLabel(QStringLiteral("Version:")), 0, 0);
    gridBasic->addWidget(m_lblVersion, 0, 1);

    m_cbCharClass = new QComboBox;
    gridBasic->addWidget(new QLabel(QStringLiteral("Class:")), 0, 2);
    gridBasic->addWidget(m_cbCharClass, 0, 3);

    m_chkLadder = new QCheckBox(QStringLiteral("Ladder"));
    m_chkExpansion = new QCheckBox(QStringLiteral("Expansion"));
    m_chkHardcore = new QCheckBox(QStringLiteral("Hardcore"));
    m_chkDiedBefore = new QCheckBox(QStringLiteral("Died"));
    auto* layFlags = new QHBoxLayout;
    layFlags->addWidget(m_chkLadder);
    layFlags->addWidget(m_chkExpansion);
    layFlags->addWidget(m_chkHardcore);
    layFlags->addWidget(m_chkDiedBefore);
    gridBasic->addLayout(layFlags, 0, 4, 1, 2);

    m_editName = new QLineEdit;
    m_editName->setMaxLength(15);
    gridBasic->addWidget(new QLabel(QStringLiteral("Name:")), 1, 0);
    gridBasic->addWidget(m_editName, 1, 1);

    m_lblCharTitle = new QLabel;
    gridBasic->addWidget(new QLabel(QStringLiteral("Title:")), 1, 2);
    gridBasic->addWidget(m_lblCharTitle, 1, 3);

    m_spinLevel = new QSpinBox;
    m_spinLevel->setRange(1, 127);
    gridBasic->addWidget(new QLabel(QStringLiteral("Level:")), 2, 0);
    gridBasic->addWidget(m_spinLevel, 2, 1);

    m_editExperience = new QLineEdit;
    m_editExperience->setReadOnly(true);
    gridBasic->addWidget(new QLabel(QStringLiteral("Exp:")), 2, 2);
    gridBasic->addWidget(m_editExperience, 2, 3);

    m_cbLevelAndExp = new QComboBox;
    for (int i = 0; i < 99; ++i) {
        m_cbLevelAndExp->addItem(QStringLiteral("%1  %2")
            .arg(i + 1, 2)
            .arg(LEVEL_AND_EXPERIENCE[i]));
    }
    m_cbLevelAndExp->setMinimumWidth(180);
    gridBasic->addWidget(m_cbLevelAndExp, 2, 4, 1, 2);

    m_cbLastDifficulty = new QComboBox;
    gridBasic->addWidget(new QLabel(QStringLiteral("Diff:")), 3, 0);
    gridBasic->addWidget(m_cbLastDifficulty, 3, 1);

    m_cbLastAct = new QComboBox;
    for (int i = 1; i <= 5; ++i)
        m_cbLastAct->addItem(QString::number(i));
    gridBasic->addWidget(new QLabel(QStringLiteral("Act:")), 3, 2);
    gridBasic->addWidget(m_cbLastAct, 3, 3);

    m_dtTime = new QDateTimeEdit;
    m_dtTime->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_dtTime->setCalendarPopup(true);
    gridBasic->addWidget(new QLabel(QStringLiteral("Time:")), 3, 4);
    gridBasic->addWidget(m_dtTime, 3, 5);

    mainLayout->addWidget(grpBasic);

    // === 属性 Group ===
    auto* grpStats = new QGroupBox(QStringLiteral("Stats"));
    auto* gridStats = new QGridLayout(grpStats);

    auto makeStatRow = [&](int row, const QString& label) -> QSpinBox* {
        auto* spin = new QSpinBox;
        spin->setRange(0, 1023);
        gridStats->addWidget(new QLabel(label + QStringLiteral(":")), row, 0);
        gridStats->addWidget(spin, row, 1);
        return spin;
    };

    m_spinStrength  = makeStatRow(0, QStringLiteral("Strength"));
    m_spinDexterity = makeStatRow(1, QStringLiteral("Dexterity"));
    m_spinVitality  = makeStatRow(2, QStringLiteral("Vitality"));
    m_spinEnergy    = makeStatRow(3, QStringLiteral("Energy"));

    m_spinStatPointsRemaining = new QSpinBox;
    m_spinStatPointsRemaining->setRange(0, 1023);
    gridStats->addWidget(new QLabel(QStringLiteral("Stat Points Left:")), 4, 0);
    gridStats->addWidget(m_spinStatPointsRemaining, 4, 1);

    auto makeLifeStamRow = [&](int row, const QString& label,
                                int maxVal) -> std::pair<QSpinBox*, QSpinBox*> {
        auto* cur = new QSpinBox;
        cur->setRange(0, maxVal);
        auto* max = new QSpinBox;
        max->setRange(0, maxVal);
        gridStats->addWidget(new QLabel(label + QStringLiteral(":")), row, 0);
        auto* lay = new QHBoxLayout;
        lay->addWidget(new QLabel(QStringLiteral("Cur:")));
        lay->addWidget(cur);
        lay->addWidget(new QLabel(QStringLiteral("Max:")));
        lay->addWidget(max);
        gridStats->addLayout(lay, row, 1, 1, 3);
        return {cur, max};
    };

    auto [curLife, maxLife]       = makeLifeStamRow(5, QStringLiteral("Life"), 8191);
    m_spinCurLife = curLife; m_spinMaxLife = maxLife;
    auto [curStam, maxStam]       = makeLifeStamRow(6, QStringLiteral("Stamina"), 8191);
    m_spinCurStamina = curStam; m_spinMaxStamina = maxStam;
    auto [curMana, maxMana]       = makeLifeStamRow(7, QStringLiteral("Mana"), 8191);
    m_spinCurMana = curMana; m_spinMaxMana = maxMana;

    mainLayout->addWidget(grpStats);

    // === 金币 Group ===
    auto* grpGold = new QGroupBox(QStringLiteral("Gold"));
    auto* gridGold = new QGridLayout(grpGold);

    m_spinGoldInBody = new QSpinBox;
    m_spinGoldInBody->setRange(0, INT_MAX);
    gridGold->addWidget(new QLabel(QStringLiteral("In Body:")), 0, 0);
    gridGold->addWidget(m_spinGoldInBody, 0, 1);
    m_lblGoldInBodyRange = new QLabel(QStringLiteral("0-10000"));
    gridGold->addWidget(m_lblGoldInBodyRange, 0, 2);

    m_spinGoldInStash = new QSpinBox;
    m_spinGoldInStash->setRange(0, INT_MAX);
    gridGold->addWidget(new QLabel(QStringLiteral("In Stash:")), 1, 0);
    gridGold->addWidget(m_spinGoldInStash, 1, 1);
    m_lblGoldInStashRange = new QLabel(QStringLiteral("0-50000"));
    gridGold->addWidget(m_lblGoldInStashRange, 1, 2);

    mainLayout->addWidget(grpGold);

    // === 技能 + 内嵌Tab ===
    auto* grpSkills = new QGroupBox(QStringLiteral("Skills & More"));
    auto* laySkills = new QVBoxLayout(grpSkills);

    auto* topRow = new QHBoxLayout;
    m_spinSkillPointsRemaining = new QSpinBox;
    m_spinSkillPointsRemaining->setRange(0, 255);
    topRow->addWidget(new QLabel(QStringLiteral("Skill Points Left:")));
    topRow->addWidget(m_spinSkillPointsRemaining);
    m_btnSkills = new QPushButton(QStringLiteral("Skills..."));
    topRow->addWidget(m_btnSkills);
    topRow->addStretch();
    laySkills->addLayout(topRow);

    m_tabSub = new QTabWidget;
    m_pageWaypoints = new WaypointsWidget;
    m_pageQuestInfo = new QuestInfoWidget;
    m_tabSub->addTab(m_pageWaypoints, QStringLiteral("Waypoints"));
    m_tabSub->addTab(m_pageQuestInfo, QStringLiteral("Quests"));
    laySkills->addWidget(m_tabSub);

    mainLayout->addWidget(grpSkills, 1);

    // === 信号连接 ===
    connect(m_spinLevel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CharInfoWidget::onLevelChanged);
    connect(m_cbLevelAndExp, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        if (idx >= 0 && idx < 99) {
            m_spinLevel->setValue(idx + 1);
            m_editExperience->setText(QString::number(LEVEL_AND_EXPERIENCE[idx]));
        }
    });
    connect(m_btnSkills, &QPushButton::clicked, this, [this]() {
        emit skillsRequested(m_cbCharClass->currentIndex(), m_bSkills);
    });
}

void CharInfoWidget::updateUI(const CD2S_Struct& character)
{
    switch (character.dwVersion) {
        case 0x69: m_lblVersion->setText(QStringLiteral("3.1")); break;
        case 0x64: m_lblVersion->setText(QStringLiteral("Warlock")); break;
        case 0x63: m_lblVersion->setText(QStringLiteral("PTR2.5")); break;
        case 0x62: m_lblVersion->setText(QStringLiteral("PTR2.4")); break;
        case 0x61: m_lblVersion->setText(QStringLiteral("Resurrected")); break;
        case 0x60: m_lblVersion->setText(QStringLiteral("1.1x")); break;
        case 0x5C: m_lblVersion->setText(QStringLiteral("1.09")); break;
        case 0x59: m_lblVersion->setText(QStringLiteral("Std 1.08")); break;
        case 0x57: m_lblVersion->setText(QStringLiteral("1.07/Exp 1.08")); break;
        case 0x47: m_lblVersion->setText(QStringLiteral("1.00-1.06")); break;
        default:   m_lblVersion->setText(g_dataMgr ? g_dataMgr->msgUnknown() : QStringLiteral("Unknown"));
    }

    m_cbCharClass->setCurrentIndex(character.charClass);
    m_chkLadder->setChecked(character.isLadder());
    m_chkExpansion->setChecked(character.isExpansion());
    m_chkHardcore->setChecked(character.isHardcore());
    m_chkDiedBefore->setChecked(character.isDiedBefore());
    m_editName->setText(character.name());

    if (character.charTitle == 0xF)
        m_lblCharTitle->setText(QStringLiteral("Patriarch/Matriarch"));
    else if (character.charTitle >= 0xA)
        m_lblCharTitle->setText(QStringLiteral("Champion"));
    else if (character.charTitle >= 5)
        m_lblCharTitle->setText(QStringLiteral("Slayer"));
    else
        m_lblCharTitle->clear();

    for (int i = 0; i < 3; ++i) {
        if (character.Town[i]) {
            m_cbLastDifficulty->setCurrentIndex(i);
            m_cbLastAct->setCurrentIndex(character.Town[i] & 7);
        }
    }

    m_dtTime->setDateTime(QDateTime::fromSecsSinceEpoch(character.dwTime));

    m_spinStrength->setValue(character.PlayerStats.m_adwValue[0]);
    m_spinEnergy->setValue(character.PlayerStats.m_adwValue[1]);
    m_spinDexterity->setValue(character.PlayerStats.m_adwValue[2]);
    m_spinVitality->setValue(character.PlayerStats.m_adwValue[3]);
    m_spinStatPointsRemaining->setValue(character.PlayerStats.m_adwValue[4]);
    m_spinSkillPointsRemaining->setValue(character.PlayerStats.m_adwValue[5]);
    m_spinCurLife->setValue(character.PlayerStats.m_adwValue[6] >> 8);
    m_spinMaxLife->setValue(character.PlayerStats.m_adwValue[7] >> 8);
    m_spinCurMana->setValue(character.PlayerStats.m_adwValue[8] >> 8);
    m_spinMaxMana->setValue(character.PlayerStats.m_adwValue[9] >> 8);
    m_spinCurStamina->setValue(character.PlayerStats.m_adwValue[0xA] >> 8);
    m_spinMaxStamina->setValue(character.PlayerStats.m_adwValue[0xB] >> 8);

    int level = character.PlayerStats.m_adwValue[0xC] & 0xFF;
    m_spinLevel->setValue(level > 0 ? level : 1);
    m_editExperience->setText(QString::number(character.PlayerStats.m_adwValue[0xD]));
    m_spinGoldInBody->setValue(character.PlayerStats.m_adwValue[0xE]);
    m_spinGoldInStash->setValue(character.PlayerStats.m_adwValue[0xF]);

    if (level >= 1 && level <= 99)
        m_cbLevelAndExp->setCurrentIndex(level - 1);

    onLevelChanged();

    ::CopyMemory(m_bSkills, character.Skills.bSkillLevel, sizeof(m_bSkills));

    // 更新子页面
    m_pageWaypoints->updateUI(character);
    m_pageQuestInfo->updateUI(character);
}

bool CharInfoWidget::gatherData(CD2S_Struct& character)
{
    QString name = m_editName->text().trimmed();
    if (!::CheckCharName(name)) {
        QMessageBox::critical(this,
            g_dataMgr ? g_dataMgr->msgError() : QStringLiteral("Error"),
            g_dataMgr ? g_dataMgr->msgBoxInfo(0) : QStringLiteral("Invalid character name."));
        return false;
    }
    character.name(name);

    int level = m_spinLevel->value();
    if (level < 1 || level > 127) {
        QMessageBox::critical(this,
            g_dataMgr ? g_dataMgr->msgError() : QStringLiteral("Error"),
            g_dataMgr ? g_dataMgr->msgBoxInfo(1) : QStringLiteral("Level out of range."));
        return false;
    }

    auto checkRange = [](int val, int maxVal) { return val < 0 || val > maxVal; };
    if (checkRange(m_spinStrength->value(), 1023) ||
        checkRange(m_spinEnergy->value(), 1023) ||
        checkRange(m_spinDexterity->value(), 1023) ||
        checkRange(m_spinVitality->value(), 1023) ||
        checkRange(m_spinStatPointsRemaining->value(), 1023) ||
        checkRange(m_spinSkillPointsRemaining->value(), 255) ||
        checkRange(m_spinCurLife->value(), 8191) ||
        checkRange(m_spinMaxLife->value(), 8191) ||
        checkRange(m_spinCurMana->value(), 8191) ||
        checkRange(m_spinMaxMana->value(), 8191) ||
        checkRange(m_spinCurStamina->value(), 8191) ||
        checkRange(m_spinMaxStamina->value(), 8191)) {
        QMessageBox::critical(this,
            g_dataMgr ? g_dataMgr->msgError() : QStringLiteral("Error"),
            g_dataMgr ? g_dataMgr->msgBoxInfo(2) : QStringLiteral("Stat value out of range."));
        return false;
    }

    character.charClass = m_cbCharClass->currentIndex();

    if (m_chkLadder->isChecked())
        character.charType |= 0x40;
    else
        character.charType &= ~0x40;
    if (m_chkExpansion->isChecked())
        character.charType |= 0x20;
    else
        character.charType &= ~0x20;
    if (m_chkHardcore->isChecked())
        character.charType |= 4;
    else
        character.charType &= ~4;
    if (m_chkDiedBefore->isChecked())
        character.charType |= 8;
    else
        character.charType &= ~8;

    character.dwTime = static_cast<DWORD>(m_dtTime->dateTime().toSecsSinceEpoch());

    ::ZeroMemory(character.Town, sizeof(character.Town));
    character.Town[m_cbLastDifficulty->currentIndex()] = 0x80 + m_cbLastAct->currentIndex();

    character.PlayerStats.m_adwValue[0] = m_spinStrength->value();
    character.PlayerStats.m_adwValue[1] = m_spinEnergy->value();
    character.PlayerStats.m_adwValue[2] = m_spinDexterity->value();
    character.PlayerStats.m_adwValue[3] = m_spinVitality->value();
    character.PlayerStats.m_adwValue[4] = m_spinStatPointsRemaining->value();
    character.PlayerStats.m_adwValue[5] = m_spinSkillPointsRemaining->value();
    character.PlayerStats.m_adwValue[6] = m_spinCurLife->value() << 8;
    character.PlayerStats.m_adwValue[7] = m_spinMaxLife->value() << 8;
    character.PlayerStats.m_adwValue[8] = m_spinCurMana->value() << 8;
    character.PlayerStats.m_adwValue[9] = m_spinMaxMana->value() << 8;
    character.PlayerStats.m_adwValue[0xA] = m_spinCurStamina->value() << 8;
    character.PlayerStats.m_adwValue[0xB] = m_spinMaxStamina->value() << 8;
    character.PlayerStats.m_adwValue[0xC] = character.charLevel = level;
    character.PlayerStats.m_adwValue[0xD] = m_editExperience->text().toUInt();
    character.PlayerStats.m_adwValue[0xE] = m_spinGoldInBody->value();
    character.PlayerStats.m_adwValue[0xF] = m_spinGoldInStash->value();

    ::CopyMemory(character.Skills.bSkillLevel, m_bSkills, sizeof(m_bSkills));

    // 收集子页面数据
    if (!m_pageWaypoints->gatherData(character) || !m_pageQuestInfo->gatherData(character))
        return false;

    return true;
}

void CharInfoWidget::resetAll()
{
    m_lblVersion->setText(QStringLiteral("--"));
    m_editName->clear();
    m_cbCharClass->setCurrentIndex(0);
    m_chkLadder->setChecked(false);
    m_chkExpansion->setChecked(false);
    m_chkHardcore->setChecked(false);
    m_chkDiedBefore->setChecked(false);
    m_lblCharTitle->clear();
    m_spinLevel->setValue(1);
    m_editExperience->clear();
    m_cbLevelAndExp->setCurrentIndex(0);
    m_cbLastDifficulty->setCurrentIndex(0);
    m_cbLastAct->setCurrentIndex(0);
    m_dtTime->setDateTime(QDateTime::currentDateTime());
    m_spinStrength->setValue(0);
    m_spinDexterity->setValue(0);
    m_spinVitality->setValue(0);
    m_spinEnergy->setValue(0);
    m_spinStatPointsRemaining->setValue(0);
    m_spinSkillPointsRemaining->setValue(0);
    m_spinCurLife->setValue(0);
    m_spinMaxLife->setValue(0);
    m_spinCurStamina->setValue(0);
    m_spinMaxStamina->setValue(0);
    m_spinCurMana->setValue(0);
    m_spinMaxMana->setValue(0);
    m_spinGoldInBody->setValue(0);
    m_spinGoldInStash->setValue(0);
    m_lblGoldInBodyRange->setText(QStringLiteral("0-10000"));
    m_lblGoldInStashRange->setText(QStringLiteral("0-50000"));
    ::ZeroMemory(m_bSkills, sizeof(m_bSkills));

    // 重置子页面
    m_pageWaypoints->resetAll();
    m_pageQuestInfo->resetAll();
}

void CharInfoWidget::loadText()
{
    if (!g_dataMgr) return;

    m_tabSub->setTabText(0, g_dataMgr->charBasicInfoUI(1));
    m_tabSub->setTabText(1, g_dataMgr->charBasicInfoUI(2));
    m_btnSkills->setText(g_dataMgr->charBasicInfoUI(3));

    int sel = m_cbCharClass->currentIndex();
    m_cbCharClass->clear();
    for (UINT i = 0; i < DataManager::CLASS_NAME_SIZE; ++i)
        m_cbCharClass->addItem(g_dataMgr->className(i));
    m_cbCharClass->setCurrentIndex(sel);

    sel = m_cbLastDifficulty->currentIndex();
    m_cbLastDifficulty->clear();
    for (UINT i = 0; i < g_dataMgr->difficultyNameSize(); ++i)
        m_cbLastDifficulty->addItem(g_dataMgr->difficultyName(i));
    m_cbLastDifficulty->setCurrentIndex(sel);

    // 加载子页面文本
    m_pageWaypoints->loadText();
    m_pageQuestInfo->loadText();
}

void CharInfoWidget::onLevelChanged()
{
    int level = m_spinLevel->value();
    if (level < 1 || level > 127) return;

    if (level <= 99)
        m_editExperience->setText(QString::number(LEVEL_AND_EXPERIENCE[level - 1]));

    m_dwMaxGoldInBody = level * 10000;
    m_lblGoldInBodyRange->setText(QStringLiteral("0-%1").arg(m_dwMaxGoldInBody));

    m_dwMaxGoldInStash = (level < 31 ? (level / 10 + 1) * 50000
                                     : (level / 2 + 1) * 50000);
    m_lblGoldInStashRange->setText(QStringLiteral("0-%1").arg(m_dwMaxGoldInStash));
}
