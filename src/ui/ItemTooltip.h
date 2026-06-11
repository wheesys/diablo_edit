#pragma once

#include <QWidget>
#include <QStringList>
#include <QColor>
#include <QVector>

#include "core/D2Item.h"

class ItemTooltip : public QWidget
{
    Q_OBJECT

public:
    explicit ItemTooltip(QWidget* parent = nullptr);
    ~ItemTooltip() override;

    // 显示物品信息
    void showItemInfo(const CD2Item* pItem, QPoint screenPos);

    // 设置透明度 (0-255, 255=不透明)
    void setTooltipTransparency(int alpha);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void addMsg(const QColor& color, const QString& msg);
    void addPropertyList(const QColor& color, DWORD version, const CPropertyList& propList);

    // 颜色索引
    enum { WHITE, BLUE, GREEN, RARE, UNIQUE, CRAFT, RED, GRAY };

    static const QColor FONT_COLORS[8];
    static const int HEIGHT_PER_LINE = 20;
    static const int WIDTH_PER_CHAR = 9;
    static const int WINDOW_WIDTH_MIN = 273;

    int m_nTransparency;
    const CD2Item* m_pItem;
    QVector<QPair<QColor, QString>> m_itemMsg;
};
