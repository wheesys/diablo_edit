#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>

#include "core/D2S_Struct.h"

class WaypointsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaypointsWidget(QWidget* parent = nullptr);
    ~WaypointsWidget() override;

    void updateUI(const CD2S_Struct& character);
    bool gatherData(CD2S_Struct& character);
    void resetAll();
    void loadText();

private:
    void setupUI();

    // 从原始字节提取位到布尔数组
    void getOriginData(int level, BOOL* way) const;

    // 当前难度切换
    void setDifficulty(int level);

    // 全选/取消
    void onSelAllClicked();

    static const int WAYPOINT_COUNT = 40;

    QButtonGroup* m_grpDifficulty;  // 3 个难度 RadioButton

    QCheckBox* m_chkWaypoints[WAYPOINT_COUNT];  // [0]=SelAll, [1..39]=waypoints

    INT m_nLevel;                        // 当前难度 0/1/2
    BYTE m_byteOriginData[3][5];         // 原始字节数据
    BOOL m_bWayData[3][WAYPOINT_COUNT];  // 3 个难度的 40 个传送点
    BOOL m_bUIData[WAYPOINT_COUNT];      // 当前显示的数据
};
