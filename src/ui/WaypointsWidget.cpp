#include "WaypointsWidget.h"
#include "data/DataManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

WaypointsWidget::WaypointsWidget(QWidget* parent)
    : QWidget(parent)
    , m_nLevel(0)
{
    ::ZeroMemory(m_byteOriginData, sizeof(m_byteOriginData));
    ::ZeroMemory(m_bWayData, sizeof(m_bWayData));
    ::ZeroMemory(m_bUIData, sizeof(m_bUIData));
    setupUI();
}

WaypointsWidget::~WaypointsWidget() = default;

void WaypointsWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // === 难度选择 ===
    auto* topRow = new QHBoxLayout;
    m_grpDifficulty = new QButtonGroup(this);
    auto* rbNormal = new QRadioButton(QStringLiteral("Normal"));
    auto* rbNightmare = new QRadioButton(QStringLiteral("Nightmare"));
    auto* rbHell = new QRadioButton(QStringLiteral("Hell"));
    m_grpDifficulty->addButton(rbNormal, 0);
    m_grpDifficulty->addButton(rbNightmare, 1);
    m_grpDifficulty->addButton(rbHell, 2);
    rbNormal->setChecked(true);
    topRow->addWidget(rbNormal);
    topRow->addWidget(rbNightmare);
    topRow->addWidget(rbHell);
    topRow->addStretch();
    mainLayout->addLayout(topRow);

    // === 传送点网格 (5 行 x 8 列 + 全选在第0列第一行) ===
    auto* grid = new QGridLayout;

    // 全选 (索引0)
    m_chkWaypoints[0] = new QCheckBox(QStringLiteral("Sel All"));
    m_chkWaypoints[0]->setTristate(true);
    grid->addWidget(m_chkWaypoints[0], 0, 0, 1, 2);

    // 39 个传送点 (1-39) 排成 5 行 x 8 列，多出的放最后
    for (int i = 1; i < WAYPOINT_COUNT; ++i) {
        int col = (i - 1) % 8;
        int row = (i - 1) / 8 + 1;  // 从第1行开始
        m_chkWaypoints[i] = new QCheckBox;
        grid->addWidget(m_chkWaypoints[i], row, col);
    }

    mainLayout->addLayout(grid);
    mainLayout->addStretch();

    // === 信号连接 ===
    connect(m_grpDifficulty, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &WaypointsWidget::setDifficulty);
    connect(m_chkWaypoints[0], &QCheckBox::stateChanged,
            this, &WaypointsWidget::onSelAllClicked);
}

void WaypointsWidget::updateUI(const CD2S_Struct& character)
{
    for (int i = 0; i < 3; ++i) {
        ::CopyMemory(m_byteOriginData[i], character.Waypoints.wp[i].Waypoints,
                     sizeof(m_byteOriginData[0]));
        getOriginData(i, m_bWayData[i]);
    }
    ::CopyMemory(m_bUIData, m_bWayData[m_nLevel], sizeof(m_bUIData));

    // 刷新 UI
    for (int i = 0; i < WAYPOINT_COUNT; ++i)
        m_chkWaypoints[i]->setChecked(m_bUIData[i]);
    m_chkWaypoints[0]->setCheckState(static_cast<Qt::CheckState>(m_bUIData[0]));
}

bool WaypointsWidget::gatherData(CD2S_Struct& character)
{
    // 从 UI 收集
    for (int i = 0; i < WAYPOINT_COUNT; ++i)
        m_bUIData[i] = m_chkWaypoints[i]->isChecked();
    ::CopyMemory(m_bWayData[m_nLevel], m_bUIData, sizeof(m_bUIData));

    for (int i = 0; i < 3; ++i) {
        ::ZeroMemory(character.Waypoints.wp[i].Waypoints,
                     sizeof(character.Waypoints.wp[i].Waypoints));
        for (int j = 1, mask = 1, index = 0; j < 40; ++j) {
            (character.Waypoints.wp[i].Waypoints)[index] |= (m_bWayData[i][j] ? mask : 0);
            if (mask == 0x80) {
                mask = 1;
                ++index;
            } else {
                mask <<= 1;
            }
        }
        // 处理 QuestInfo wTraval
        character.QuestInfo.QIData[i].wTraval1 = m_bWayData[i][10] ? 1 : 0;
        character.QuestInfo.QIData[i].wTraval2 = m_bWayData[i][19] ? 1 : 0;
        character.QuestInfo.QIData[i].wTraval3 = m_bWayData[i][28] ? 1 : 0;
        character.QuestInfo.QIData[i].wTraval4 = m_bWayData[i][31] ? 1 : 0;
    }
    return true;
}

void WaypointsWidget::resetAll()
{
    for (int i = 0; i < 3; ++i) {
        ::ZeroMemory(m_byteOriginData[i], sizeof(m_byteOriginData[i]));
        ::ZeroMemory(m_bWayData[i], sizeof(m_bWayData[i]));
    }
    ::ZeroMemory(m_bUIData, sizeof(m_bUIData));
    m_nLevel = 0;

    auto* rb = qobject_cast<QRadioButton*>(m_grpDifficulty->button(0));
    if (rb) rb->setChecked(true);
    for (int i = 0; i < WAYPOINT_COUNT; ++i)
        m_chkWaypoints[i]->setChecked(m_bUIData[i]);
}

void WaypointsWidget::loadText()
{
    if (!g_dataMgr) return;

    // 全选按钮文本
    m_chkWaypoints[0]->setText(g_dataMgr->otherUI(8));

    // 传送点名
    for (int i = 1; i < WAYPOINT_COUNT; ++i)
        m_chkWaypoints[i]->setText(g_dataMgr->wayPointName(i - 1));

    // 难度按钮文本
    auto setRbText = [&](int id, const QString& text) {
        auto* rb = qobject_cast<QRadioButton*>(m_grpDifficulty->button(id));
        if (rb) rb->setText(text);
    };
    for (UINT i = 0; i < g_dataMgr->difficultyNameSize(); ++i)
        setRbText(static_cast<int>(i), g_dataMgr->difficultyName(i));
}

void WaypointsWidget::getOriginData(int level, BOOL* way) const
{
    way[0] = 2;  // BST_INDETERMINATE
    for (int j = 1, mask = 1, index = 0; j < 40; ++j) {
        way[j] = (m_byteOriginData[level][index] & mask) != 0;
        if (mask == 0x80) {
            mask = 1;
            ++index;
        } else {
            mask <<= 1;
        }
    }
}

void WaypointsWidget::setDifficulty(int level)
{
    if (m_nLevel == level) return;

    // 保存当前 UI 数据
    for (int i = 0; i < WAYPOINT_COUNT; ++i)
        m_bUIData[i] = m_chkWaypoints[i]->isChecked();
    ::CopyMemory(m_bWayData[m_nLevel], m_bUIData, sizeof(m_bUIData));

    m_nLevel = level;
    ::CopyMemory(m_bUIData, m_bWayData[level], sizeof(m_bUIData));

    // 刷新 UI
    for (int i = 0; i < WAYPOINT_COUNT; ++i)
        m_chkWaypoints[i]->setChecked(m_bUIData[i]);
    m_chkWaypoints[0]->setCheckState(static_cast<Qt::CheckState>(m_bUIData[0]));
}

void WaypointsWidget::onSelAllClicked()
{
    Qt::CheckState state = m_chkWaypoints[0]->checkState();
    for (int i = 0; i < WAYPOINT_COUNT; ++i) {
        if (state == Qt::Checked)
            m_bUIData[i] = TRUE;
        else if (state == Qt::Unchecked)
            m_bUIData[i] = FALSE;
        else
            getOriginData(m_nLevel, m_bUIData);
        m_chkWaypoints[i]->setChecked(m_bUIData[i]);
    }
}
