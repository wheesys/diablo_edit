#include "MainWindow.h"
#include "CharInfoWidget.h"
#include "ItemGridWidget.h"
#include "SkillsDialog.h"
#include "SelectCharDialog.h"
#include "FoundryDialog.h"
#include "NewItemDialog.h"
#include "core/ImageManager.h"

#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QMessageBox>
#include <QStyle>
#include <QAction>
#include <QActionGroup>

#include "data/DataManager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Diablo Edit"));
    resize(900, 650);

    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 角色基本信息编辑页
    m_charInfo = new CharInfoWidget;
    m_tabWidget->addTab(m_charInfo, QStringLiteral("Character"));

    // 物品编辑页
    m_itemGrid = new ItemGridWidget;
    m_tabWidget->addTab(m_itemGrid, QStringLiteral("Items"));

    createMenus();
    createToolBar();
    statusBar()->showMessage(QStringLiteral("Ready"));

    // 连接技能编辑器信号
    connect(m_charInfo, &CharInfoWidget::skillsRequested, this, [this](int charClass, BYTE skills[30]) {
        SkillsDialog dlg(charClass, skills, this);
        dlg.exec();
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));

    fileMenu->addAction(QStringLiteral("&New"), this, &MainWindow::onFileNew, QKeySequence::New);
    fileMenu->addAction(QStringLiteral("&Open..."), this, &MainWindow::onFileOpen, QKeySequence::Open);
    fileMenu->addAction(QStringLiteral("&Save"), this, &MainWindow::onFileSave, QKeySequence::Save);
    fileMenu->addAction(QStringLiteral("Save &As..."), this, &MainWindow::onFileSaveAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("E&xit"), this, &QWidget::close, QKeySequence::Quit);

    // Language menu
    auto* langMenu = menuBar()->addMenu(QStringLiteral("&Language"));
    m_langGroup = new QActionGroup(this);
    m_langGroup->setExclusive(true);

    QStringList langNames = {QStringLiteral("English"), QStringLiteral("简体中文"), QStringLiteral("繁體中文")};
    for (int i = 0; i < langNames.size(); ++i) {
        auto* act = langMenu->addAction(langNames[i],
            this, [this, i]() { onLanguageChanged(i); });
        act->setCheckable(true);
        act->setChecked(i == (g_dataMgr ? g_dataMgr->langIndex() : 0));
        m_langGroup->addAction(act);
    }

    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(QStringLiteral("&About"), this, [this]() {
        QMessageBox::about(this, QStringLiteral("About Diablo Edit"),
            QStringLiteral("Diablo Edit - Diablo II Save Editor\nCross-platform Qt version"));
    });
}

void MainWindow::createToolBar()
{
    auto* toolbar = addToolBar(QStringLiteral("Main"));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon),
        QStringLiteral("New"), this, &MainWindow::onFileNew);
    toolbar->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon),
        QStringLiteral("Open"), this, &MainWindow::onFileOpen);
    toolbar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton),
        QStringLiteral("Save"), this, &MainWindow::onFileSave);
}

void MainWindow::onFileNew()
{
    int cls = 0;
    SelectCharDialog dlg(cls, this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_character.Reset();
    m_currentFile.clear();
    m_character.charClass = cls;

    // 使用新角色模板数据
    if (g_dataMgr)
        m_character = g_dataMgr->newCharacter();

    m_character.charClass = cls;
    updateTitle();
    if (m_charInfo)
        m_charInfo->updateUI(m_character);
    if (m_itemGrid)
        m_itemGrid->updateUI(m_character);
    m_tabWidget->setCurrentIndex(0);
    statusBar()->showMessage(QStringLiteral("New character"));
}

void MainWindow::onFileOpen()
{
    QString path = QFileDialog::getOpenFileName(this,
        QStringLiteral("Open D2S File"),
        QString(),
        QStringLiteral("D2S Files (*.d2s);;All Files (*)"));
    if (path.isEmpty()) return;

    try {
        m_character.ReadFile(path);
    } catch (const D2Error& e) {
        qWarning() << "D2Error:" << e.message();
        QMessageBox::critical(this, QStringLiteral("Error"),
            QStringLiteral("Failed to load file:\n%1\n\n%2").arg(path, e.message()));
        return;
    }

    m_currentFile = path;
    updateTitle();
    if (m_charInfo)
        m_charInfo->updateUI(m_character);
    if (m_itemGrid)
        m_itemGrid->updateUI(m_character);
    statusBar()->showMessage(QStringLiteral("Loaded: %1").arg(path));
}

void MainWindow::onFileSave()
{
    if (m_currentFile.isEmpty()) {
        onFileSaveAs();
        return;
    }
    if (!m_charInfo || !m_charInfo->gatherData(m_character))
        return; // gatherData 失败时已显示错误消息
    if (!m_itemGrid || !m_itemGrid->gatherData(m_character))
        return;

    try {
        m_character.WriteFile(m_currentFile);
    } catch (const D2Error& e) {
        QMessageBox::critical(this, QStringLiteral("Error"),
            QStringLiteral("Failed to save file:\n%1").arg(e.message()));
        return;
    }
    statusBar()->showMessage(QStringLiteral("Saved: %1").arg(m_currentFile));
}

void MainWindow::onFileSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("Save D2S File"),
        m_currentFile,
        QStringLiteral("D2S Files (*.d2s);;All Files (*)"));
    if (path.isEmpty()) return;

    m_currentFile = path;
    onFileSave();
    updateTitle();
}

void MainWindow::updateTitle()
{
    if (m_currentFile.isEmpty())
        setWindowTitle(QStringLiteral("Diablo Edit"));
    else
        setWindowTitle(QStringLiteral("Diablo Edit - %1").arg(m_currentFile));
}

void MainWindow::onLanguageChanged(int langIndex)
{
    if (!g_dataMgr) return;
    g_dataMgr->setLangIndex(langIndex);
    m_charInfo->loadText();
    m_itemGrid->loadText();
    // Update menu text
    m_tabWidget->setTabText(0, g_dataMgr->charBasicInfoUI(0));
    m_tabWidget->setTabText(1, g_dataMgr->charItemsUI(0));
    update();
}
