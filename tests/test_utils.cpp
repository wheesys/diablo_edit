#include "test_utils.h"
#include "data/DataManager.h"
#include <QCoreApplication>
#include <QDebug>

// 全局 DataManager 指针（测试环境中定义）
DataManager* g_dataMgr = nullptr;

namespace TestUtils {

QString projectRoot() {
    // 优先使用编译期注入的 PROJECT_SOURCE_DIR
#ifdef PROJECT_SOURCE_DIR
    QString compiledPath = QStringLiteral(PROJECT_SOURCE_DIR);
    if (QDir(compiledPath).exists("data/itemdata.dat"))
        return compiledPath;
#endif

    // 运行时推断（从 build 子目录向上找）
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/..",
        QDir::currentPath(),
        QDir::currentPath() + "/..",
    };
    for (const auto& path : candidates) {
        if (QDir(path).exists("data/itemdata.dat"))
            return QDir(path).absolutePath();
    }
    qWarning() << "TestUtils: cannot find project root, using cwd";
    return QDir::currentPath();
}

QString dataDir() { return projectRoot() + "/data"; }

QString imagesDir() { return projectRoot() + "/resources/images"; }

QString referenceDir() { return projectRoot() + "/tests/reference"; }

} // namespace TestUtils
