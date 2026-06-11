#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>

#include "core/D2S_Struct.h"

// 经验值表 (D2 1-99级)
extern const DWORD LEVEL_AND_EXPERIENCE[100];

class WaypointsWidget;
class QuestInfoWidget;

class CharInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharInfoWidget(QWidget* parent = nullptr);
    ~CharInfoWidget() override;

    // 从 CD2S_Struct 加载数据到界面
    void updateUI(const CD2S_Struct& character);
    // 从界面收集数据写入 CD2S_Struct，失败返回 false
    bool gatherData(CD2S_Struct& character);
    // 重置所有界面控件为默认值
    void resetAll();
    // 加载多语言字符串
    void loadText();

signals:
    void skillsRequested(int charClass, BYTE skills[30]);

private slots:
    void onLevelChanged();

private:
    void setupUI();

    // --- 角色基本信息 ---
    QLabel*     m_lblVersion;
    QLineEdit*  m_editName;
    QComboBox*  m_cbCharClass;
    QCheckBox*  m_chkLadder;
    QCheckBox*  m_chkExpansion;
    QCheckBox*  m_chkHardcore;
    QCheckBox*  m_chkDiedBefore;
    QLabel*     m_lblCharTitle;

    // --- 等级和经验 ---
    QSpinBox*   m_spinLevel;
    QLineEdit*  m_editExperience;
    QComboBox*  m_cbLevelAndExp;

    // --- 最后游戏进度 ---
    QComboBox*  m_cbLastDifficulty;
    QComboBox*  m_cbLastAct;

    // --- 时间 ---
    QDateTimeEdit* m_dtTime;

    // --- 属性 ---
    QSpinBox*   m_spinStrength;
    QSpinBox*   m_spinDexterity;
    QSpinBox*   m_spinVitality;
    QSpinBox*   m_spinEnergy;
    QSpinBox*   m_spinStatPointsRemaining;

    // --- 生命/耐力/法力 ---
    QSpinBox*   m_spinMaxLife;
    QSpinBox*   m_spinCurLife;
    QSpinBox*   m_spinMaxStamina;
    QSpinBox*   m_spinCurStamina;
    QSpinBox*   m_spinMaxMana;
    QSpinBox*   m_spinCurMana;

    // --- 金币 ---
    QSpinBox*   m_spinGoldInBody;
    QSpinBox*   m_spinGoldInStash;
    QLabel*     m_lblGoldInBodyRange;
    QLabel*     m_lblGoldInStashRange;
    DWORD       m_dwMaxGoldInBody = 0;
    DWORD       m_dwMaxGoldInStash = 0;

    // --- 技能 ---
    QSpinBox*   m_spinSkillPointsRemaining;
    QPushButton* m_btnSkills;
    BYTE        m_bSkills[30];

    // --- 内嵌子页 ---
    QTabWidget* m_tabSub;
    WaypointsWidget* m_pageWaypoints;
    QuestInfoWidget* m_pageQuestInfo;
};
