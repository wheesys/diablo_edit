#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QActionGroup>

#include "core/D2S_Struct.h"

class CharInfoWidget;
class ItemGridWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onLanguageChanged(int langIndex);

private:
    void createMenus();
    void createToolBar();
    void updateTitle();

    QTabWidget* m_tabWidget = nullptr;
    CharInfoWidget* m_charInfo = nullptr;
    ItemGridWidget* m_itemGrid = nullptr;
    CD2S_Struct m_character;
    QString m_currentFile;
    QActionGroup* m_langGroup = nullptr;
};
