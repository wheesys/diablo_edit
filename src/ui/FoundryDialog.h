#pragma once

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include "core/D2Item.h"

/**
 * FoundryDialog - 物品工坊/属性编辑对话框
 * 编辑单个 CD2Item 的所有属性（品质、前缀后缀、属性列表等）
 */
class FoundryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FoundryDialog(CD2Item& item, QWidget* parent = nullptr);
    ~FoundryDialog() override;

private slots:
    void onQualityChanged(int index);
    void onPropertyTabChanged(int index);
    void onPropertyItemChanged(QTableWidgetItem* current, QTableWidgetItem* prev);

private:
    void setupUI();
    void loadFromItem();
    bool saveToItem();
    void updatePropertyList();
    void validate(bool expr, int msgId);

    // 数据
    CD2Item& m_item;
    std::pair<CPropertyList, bool> m_props[7]; // main + 5 set + runeword

    // UI controls
    QComboBox* m_cbQuality = nullptr;
    QComboBox* m_cbClass = nullptr;
    QComboBox* m_cbMonster = nullptr;
    QComboBox* m_cbRareName[2] = {nullptr, nullptr};
    QComboBox* m_cbSubType = nullptr;
    QComboBox* m_cbPrefix[3] = {nullptr, nullptr, nullptr};
    QComboBox* m_cbSuffix[3] = {nullptr, nullptr, nullptr};
    QComboBox* m_cbSetName = nullptr;
    QComboBox* m_cbUniqueName = nullptr;

    QLineEdit* m_edLevel = nullptr;
    QCheckBox* m_chUnidentified = nullptr;
    QCheckBox* m_chEthereal = nullptr;
    QCheckBox* m_chIndestructible = nullptr;
    QLineEdit* m_edMaxDurability = nullptr;
    QLineEdit* m_edDurability = nullptr;
    QLineEdit* m_edQuantity = nullptr;
    QLineEdit* m_edOwner = nullptr;
    QLineEdit* m_edSockets = nullptr;
    QLineEdit* m_edDefence = nullptr;
    QCheckBox* m_chRuneWord = nullptr;

    QTabWidget* m_tabProp = nullptr;
    QTableWidget* m_lstProperty = nullptr;
    QLineEdit* m_edParam[4] = {nullptr, nullptr, nullptr, nullptr};
    QComboBox* m_cbParam[2] = {nullptr, nullptr};

    QLabel* m_lblExtSockets = nullptr;

    bool m_noParamChange = false;
};
