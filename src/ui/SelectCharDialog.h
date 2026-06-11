#pragma once

#include <QDialog>
#include <QComboBox>

class SelectCharDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCharDialog(int& sel, QWidget* parent = nullptr);
    ~SelectCharDialog() override;

private:
    int& m_nSel;
    QComboBox* m_cbSelectChar;
};
