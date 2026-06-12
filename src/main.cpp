#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <cstdio>
#include "ui/MainWindow.h"
#include "data/DataManager.h"
#include "core/ImageManager.h"

// 全局 DataManager 指针
DataManager* g_dataMgr = nullptr;

// 日志文件句柄
static FILE* g_logFile = nullptr;

// Qt 消息处理器：将 qDebug/qWarning/qCritical 输出到文件
static void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    if (g_logFile) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        const char* prefix = "";
        switch (type) {
            case QtDebugMsg:    prefix = "D"; break;
            case QtWarningMsg:  prefix = "W"; break;
            case QtCriticalMsg: prefix = "E"; break;
            case QtFatalMsg:    prefix = "F"; break;
            default: break;
        }
        fprintf(g_logFile, "[%s] %s\n", prefix, msg.toUtf8().constData());
        fflush(g_logFile);
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 打开日志文件（与 exe 同级目录）
    QString logPath = QCoreApplication::applicationDirPath() + QStringLiteral("/diabloedit.log");
    g_logFile = fopen(logPath.toUtf8().constData(), "w");
    qInstallMessageHandler(messageHandler);

    if (g_logFile) {
        qDebug() << "Log started:" << QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    app.setApplicationName(QStringLiteral("Diablo Edit"));
    app.setOrganizationName(QStringLiteral("DiabloEdit"));

    // 初始化数据管理器
    DataManager dataMgr;
    g_dataMgr = &dataMgr;

    // 数据文件路径：可执行文件同目录下的 data/ 文件夹
    QString dataPath = QCoreApplication::applicationDirPath() + QStringLiteral("/data");
    qDebug() << "Data path:" << dataPath;

    bool loaded = false;
    try {
        loaded = dataMgr.loadAll(dataPath);
    } catch (const std::exception& e) {
        qWarning() << "EXCEPTION in loadAll:" << e.what();
        return 1;
    }
    qDebug() << "Loaded:" << loaded;
    if (!loaded) {
        QMessageBox::critical(nullptr,
            QStringLiteral("Error"),
            QStringLiteral("Failed to load data files from:\n%1").arg(dataPath));
        return 1;
    }
    qDebug() << "Data loaded successfully";

    // 初始化图片管理器
    QString imagePath = QCoreApplication::applicationDirPath() + QStringLiteral("/resources/images");
    if (!ImageManager::instance()->loadAll(imagePath)) {
        qWarning() << "Failed to load images from:" << imagePath;
    }

    try {
        qDebug() << "Creating MainWindow...";
        MainWindow mainWindow;
        qDebug() << "MainWindow created, showing...";
        mainWindow.show();
        return app.exec();
    } catch (const D2Error& e) {
        qCritical() << "D2Error caught:" << e.message();
        return 1;
    } catch (const std::exception& e) {
        qCritical() << "Exception:" << e.what();
        return 1;
    }
}
