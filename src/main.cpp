#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include "ui/MainWindow.h"
#include "data/DataManager.h"
#include "core/ImageManager.h"

// 全局 DataManager 指针
DataManager* g_dataMgr = nullptr;

int main(int argc, char* argv[])
{
    std::cerr << "A" << std::endl;

    QApplication app(argc, argv);

    std::cerr << "B" << std::endl;
    app.setApplicationName(QStringLiteral("Diablo Edit"));
    app.setOrganizationName(QStringLiteral("DiabloEdit"));

    std::cerr << "C" << std::endl;

    // 初始化数据管理器
    DataManager dataMgr;
    g_dataMgr = &dataMgr;

    std::cerr << "D" << std::endl;

    // 数据文件路径：可执行文件同目录下的 data/ 文件夹
    QString dataPath = QCoreApplication::applicationDirPath() + QStringLiteral("/data");
    std::cerr << "D1 path=" << dataPath.toStdString() << std::endl;
    qDebug() << "Data path:" << dataPath;

    std::cerr << "D2" << std::endl;
    bool loaded = false;
    try {
        loaded = dataMgr.loadAll(dataPath);
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION in loadAll: " << e.what() << std::endl;
        return 1;
    }
    std::cerr << "D3 loaded=" << loaded << std::endl;
    if (!loaded) {
        QMessageBox::critical(nullptr,
            QStringLiteral("Error"),
            QStringLiteral("Failed to load data files from:\n%1").arg(dataPath));
        return 1;
    }
    std::cerr << "E" << std::endl;
    qDebug() << "Data loaded successfully";

    // 初始化图片管理器
    QString imagePath = QCoreApplication::applicationDirPath() + QStringLiteral("/resources/images");
    if (!ImageManager::instance()->loadAll(imagePath)) {
        qWarning() << "Failed to load images from:" << imagePath;
    }

    try {
        std::cerr << "F" << std::endl;
        qDebug() << "Creating MainWindow...";
        MainWindow mainWindow;
        std::cerr << "G" << std::endl;
        qDebug() << "MainWindow created, showing...";
        mainWindow.show();
        return app.exec();
    } catch (const D2Error& e) {
        qCritical() << "D2Error caught:" << e.message();
        std::cerr << "D2Error: " << e.message().toStdString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        qCritical() << "Exception:" << e.what();
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
