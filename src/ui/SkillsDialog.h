#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QLabel>

#include "core/D2Types.h"

class SkillsDialog : public QDialog
{
    Q_OBJECT

public:
    SkillsDialog(int charClass, BYTE skills[30], QWidget* parent = nullptr);
    ~SkillsDialog() override;

    void loadText();

private:
    void setupUI();

    static const int SKILL_SIZE = 30;
    static const int TAB_SIZE = 3;

    int m_nCharClass;
    BYTE* m_pData;            // 外部技能数据指针 (绝对索引)
    BYTE m_pSkills[SKILL_SIZE];  // 相对索引技能值
    BYTE m_bAll;              // 批量设置值

    QLabel* m_tabLabels[3];
    QLabel* m_skillLabels[SKILL_SIZE];
    QSpinBox* m_spinSkills[SKILL_SIZE];
    QSpinBox* m_spinSetAll;
};
