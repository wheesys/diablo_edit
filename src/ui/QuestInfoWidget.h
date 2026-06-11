#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QButtonGroup>

#include "core/D2S_Struct.h"

class QuestInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuestInfoWidget(QWidget* parent = nullptr);
    ~QuestInfoWidget() override;

    void updateUI(const CD2S_Struct& character);
    bool gatherData(CD2S_Struct& character);
    void resetAll();
    void loadText();

private:
    void setupUI();

    // 切换难度
    void setDifficulty(int level);

    // 重置统计按钮启用状态
    void updateResetStats();

    static const int QUEST_NAME_SIZE = 29;
    static const int LEVEL_SIZE = 3;
    static const WORD QUEST_COMPLETE[27];

    QButtonGroup* m_grpDifficulty;                      // 3 个难度 RadioButton
    QCheckBox* m_chkQuests[QUEST_NAME_SIZE];             // 29 个任务 CheckBox

    INT m_nLevel;                                        // 当前难度
    BOOL m_bQuestInfo[LEVEL_SIZE][QUEST_NAME_SIZE];      // 3 难度 x 29 任务
    BOOL m_bUIData[QUEST_NAME_SIZE];                     // 当前显示数据
};
