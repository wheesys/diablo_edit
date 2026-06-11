#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QLabel>
#include <memory>
#include "core/D2Item.h"

/**
 * NewItemDialog - 新建物品对话框
 * 显示物品分类树，支持预览和创建 CD2Item
 */
class NewItemDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param outItem 输出参数，创建成功时通过智能指针传出
     * @param parent 父窗口
     */
    explicit NewItemDialog(std::unique_ptr<CD2Item>& outItem, QWidget* parent = nullptr);
    ~NewItemDialog() override;

private slots:
    void onTreeSelectionChanged();
    void onTreeDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUI();
    void loadCategories();
    void accept() override;

    std::unique_ptr<CD2Item>& m_outItem;
    QTreeWidget* m_tree = nullptr;
    QLabel* m_previewLabel = nullptr;
    QLabel* m_itemNameLabel = nullptr;
    int m_displayPicIndex = -1;
};
