#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QVector>
#include <QRect>

#include "core/D2S_Struct.h"
#include "core/D2Item.h"

// ==================== 枚举定义 ====================

enum EEquip {
    E_STORAGE = 1,
    E_HEAD = 1 << 1,
    E_NECK = 1 << 2,
    E_BODY = 1 << 3,
    E_HAND = 1 << 4,
    E_RING = 1 << 5,
    E_BELT = 1 << 6,
    E_FOOT = 1 << 7,
    E_GLOVE = 1 << 8,
    E_IN_BELT = 1 << 9,
    E_SOCKET = 1 << 10,
    E_A_BOX = 1 << 11
};

enum EPosition {
    STASH, STASH_D2R, INVENTORY, CUBE, IN_BELT, IN_SOCKET,
    GRID_COUNT,
    HEAD = GRID_COUNT, NECK, BODY, RIGHT_HAND, LEFT_HAND,
    RIGHT_RING, LEFT_RING, BELT, FOOT, GLOVE,
    CORPSE_HEAD, CORPSE_NECK, CORPSE_BODY, CORPSE_RIGHT_HAND, CORPSE_LEFT_HAND,
    CORPSE_RIGHT_RING, CORPSE_LEFT_RING, CORPSE_BELT, CORPSE_FOOT, CORPSE_GLOVE,
    CORPSE_END,
    MERCENARY_HEAD = CORPSE_END, MERCENARY_BODY,
    MERCENARY_RIGHT_HAND, MERCENARY_LEFT_HAND,
    MERCENARY_END,
    GOLEM = MERCENARY_END,
    POSITION_END,
    IN_MOUSE = POSITION_END,
    IN_RECYCLE
};

static const int GRID_WIDTH = 30;           // 每格像素
static const int MAX_SOCKETS_DISPLAY = 7;   // 最大孔数

// ==================== 网格位置信息 ====================

struct PositionInfo {
    int left, top, cols, rows;
    int equipType; // EEquip 或 ~EEquip
};

// ==================== 物品视图结构 ====================

struct CItemView {
    CD2Item item;
    int picIndex;
    EEquip equip;
    EPosition position;
    int gridX, gridY;       // 网格坐标
    int gridWidth, gridHeight; // 占用网格大小
    QVector<int> gemItems;  // 镶嵌物品在 m_vItemViews 中的索引，-1 为空

    CItemView(const CD2Item& it, EEquip eq, EPosition pos, int x, int y);
    QSize viewSize() const;
    int gemCount() const;
    const CD2Item& updateItem(QVector<CItemView>& vItemViews);
};

// ==================== 网格视图 ====================

class GridView {
public:
    int position() const { return m_position; }
    QRect rect;             // 像素区域
    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    EEquip equipType;       // 可装备的物品类型

    explicit GridView(EPosition pos, const PositionInfo& info);

    bool visible() const { return m_visible; }
    void setVisible(bool v) { m_visible = v; }
    bool enabled() const { return m_enabled && m_visible; }
    void setEnabled(bool v) { m_enabled = v; }
    bool isGrid() const;
    bool isSockets() const;

    void reset();
    bool canEquip(EEquip equip) const { return (equip & equipType) != 0; }
    int itemIndex(int x, int y) const;
    void setItemIndex(int index, int x, int y);
    void setItemIndex(int index, int x, int y, int width, int height);

    QPoint indexToXY(int x, int y, int width, int height) const;

    struct HitResult {
        int pos;  // EPosition
        int x, y; // 网格坐标
    };
    HitResult xyToPositionIndex(QPoint pt, bool alternativeWeapon, bool corpseII, int col, int row) const;

    // 尝试放入物品。返回 (成功, 已存在的物品索引/-1)
    std::pair<bool, int> putItem(int index, int x, int y, int width, int height, EEquip equip);

private:
    int m_position;
    int m_cols, m_rows;
    bool m_enabled = true;
    bool m_visible = true;
    QVector<int> m_itemIndex; // 网格内的物品索引，-1 为空
};

// ==================== ItemGridWidget ====================

class ItemGridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ItemGridWidget(QWidget* parent = nullptr);
    ~ItemGridWidget() override;

    // 标准接口
    void updateUI(const CD2S_Struct& character);
    bool gatherData(CD2S_Struct& character);
    void resetAll();
    void loadText();
    void setD2R(bool v);

signals:
    void mercenaryChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // ===== 网格和物品管理 =====
    void calculateScale();
    void initGridView();
    void addItemInGrid(const CD2Item& item, int body);
    QPoint getItemPositionXY(const CItemView& view) const;
    CItemView& selectedParentItemView();
    CItemView& selectedItemView();
    CItemView& pickedItemView();

    // Hit test result: position enum, grid x, grid y
    struct HitResult { int pos = -1, x = 0, y = 0; };
    HitResult HitTestPosition(QPoint pos, int col = 1, int row = 1) const;
    std::pair<bool, int> putItemInGrid(EPosition pos, int x, int y);

    // ===== 绘制 =====
    void drawGrids(QPainter& painter);
    void drawItemAt(QPainter& painter, QPoint pos, const CItemView& view);
    void drawAllItems(QPainter& painter);
    QPixmap createItemCursor(const CItemView& view);

    // ===== 右键菜单 =====
    void showContextMenu(QPoint globalPos);
    void onItemImport();
    void onItemExport();
    void onItemCopy();
    void onItemPaste();
    void onItemModify();
    void onItemNew();
    void onItemRemove();

    // ===== 雇佣兵 =====
    void setupMercUI();
    void refreshMercNames();
    void onMercTypeChanged();
    static int mercNameGroup(int typeName);

    // ===== 状态 =====
    bool m_hasCharacter = false;
    bool m_isD2R = true;

    // 物品视图
    QVector<CItemView> m_itemViews;
    bool m_hasCorpse = false;
    DWORD m_weaponSet = 0;
    bool m_secondHand = false;

    // 网格
    QVector<GridView> m_gridViews;

    // 选中和拿起
    int m_selectedItemIndex = -1;
    int m_selectedSocketIndex = -1;
    int m_pickedItemIndex = -1;
    bool m_showCorpseIIHand = false;

    // 剪贴板
    int m_copiedItemIndex = -1;

    // 雇佣兵
    bool m_hasMercenary = false;
    QComboBox* m_cbMercType = nullptr;
    QComboBox* m_cbMercName = nullptr;
    QLineEdit* m_edMercExp = nullptr;
    QCheckBox* m_chMercDead = nullptr;
    QCheckBox* m_chCorpse = nullptr;
    QCheckBox* m_chMercenary = nullptr;
    QCheckBox* m_chSecondHand = nullptr;
    int m_mercNameGroup = -1;

    // 回收站
    QListWidget* m_lstRecycle = nullptr;

    // D2R 切换
    QCheckBox* m_chD2R = nullptr;

    // 鼠标光标
    QCursor m_itemCursor;

    // 上一步鼠标位置
    QPoint m_lastMousePos;
};
