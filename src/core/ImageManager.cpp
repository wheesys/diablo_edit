#include "ImageManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCollator>
#include <QThread>
#include <QDebug>
#include <QTextStream>

ImageManager* ImageManager::s_instance = nullptr;

ImageManager* ImageManager::instance()
{
    if (!s_instance)
        s_instance = new ImageManager;
    return s_instance;
}

ImageManager::ImageManager(QObject* parent)
    : QObject(parent)
{
}

bool ImageManager::loadAll(const QString& imageRootPath)
{
    QDir root(imageRootPath);
    if (!root.exists()) {
        qWarning() << "ImageManager: directory not found:" << imageRootPath;
        return false;
    }

    // 查找 image_order.txt（多种路径尝试）
    QStringList orderCandidates = {
        QDir(imageRootPath).filePath("../../data/image_order.txt"), // resources/images -> data/
        QDir(imageRootPath).filePath("../image_order.txt"),         // resources/images -> resources/
        QDir(imageRootPath).filePath("image_order.txt"),            // 直接放在 images 目录
    };

    for (const auto& path : orderCandidates) {
        if (QFile::exists(path)) {
            qDebug() << "ImageManager: loading with order file:" << path;
            return loadWithOrderFile(imageRootPath, path);
        }
    }

    qDebug() << "ImageManager: order file not found, loading legacy";
    return loadLegacy(imageRootPath);
}

QPixmap ImageManager::getPixmap(int picIndex) const
{
    if (picIndex < 0 || picIndex >= m_pixmaps.size())
        return QPixmap();
    return m_pixmaps[picIndex];
}

QPixmap ImageManager::getPixmapByCode(const char* typeCode) const
{
    int idx = picIndexForCode(typeCode);
    if (idx < 0)
        return QPixmap();
    return m_pixmaps[idx];
}

int ImageManager::picIndexForCode(const char* typeCode) const
{
    if (!typeCode || typeCode[0] == '\0')
        return -1;
    // 构造 DWORD（匹配 CItemMetaData::dwTypeID）
    DWORD key = 0;
    for (int i = 0; i < 4 && typeCode[i]; ++i)
        key |= (static_cast<BYTE>(typeCode[i]) << (i * 8));
    auto it = m_codeToIndex.find(key);
    return (it != m_codeToIndex.end()) ? it.value() : -1;
}

// ==================== 新版：按 order 文件加载 ====================

bool ImageManager::loadWithOrderFile(const QString& imageRootPath, const QString& orderFile)
{
    QFile file(orderFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "ImageManager: cannot open order file:" << orderFile;
        return false;
    }

    // 解析每行：code\tpath
    struct Entry { QString code; QString path; };
    QVector<Entry> entries;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        int tab = line.indexOf('\t');
        if (tab <= 0) continue;
        QString code = line.left(tab).trimmed();
        QString relPath = line.mid(tab + 1).trimmed();
        if (code.isEmpty() || relPath.isEmpty()) continue;
        entries.append({code, relPath});
    }
    file.close();

    if (entries.isEmpty()) {
        qWarning() << "ImageManager: order file is empty:" << orderFile;
        return false;
    }

    // 按序加载 BMP
    m_pixmaps.clear();
    m_pixmaps.reserve(entries.size());
    m_codeToIndex.clear();
    m_codeToIndex.reserve(entries.size());

    for (int i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        QString fullPath = QDir(imageRootPath).filePath(e.path);
        QPixmap pix;
        if (pix.load(fullPath, "BMP")) {
            m_pixmaps.append(pix);
            // 建立代码→索引映射
            if (e.code != "????") {
                DWORD key = 0;
                QByteArray codeBytes = e.code.toUtf8();
                for (int j = 0; j < 4 && j < codeBytes.size(); ++j)
                    key |= (static_cast<BYTE>(codeBytes[j]) << (j * 8));
                m_codeToIndex[key] = i;
            }
        } else {
            qWarning() << "ImageManager: failed to load" << fullPath;
            m_pixmaps.append(QPixmap()); // 占位
        }
    }

    qDebug() << "ImageManager: loaded" << m_pixmaps.size()
             << "images (order file), codes:" << m_codeToIndex.size();
    return true;
}

// ==================== 旧版：目录扫描（降级） ====================

bool ImageManager::loadLegacy(const QString& imageRootPath)
{
    QDir root(imageRootPath);

    // 1. 递归收集所有 BMP 文件路径
    QVector<QString> files;
    scanDir(imageRootPath, files);

    if (files.isEmpty()) {
        qWarning() << "ImageManager: no BMP files found in" << imageRootPath;
        return false;
    }

    // 2. 按数字前缀排序（提取文件名开头的数字）
    QCollator collator;
    collator.setNumericMode(true);

    std::sort(files.begin(), files.end(), [&collator](const QString& a, const QString& b) {
        // 先按目录名排序
        QFileInfo fa(a), fb(b);
        int dirCmp = collator.compare(fa.dir().dirName(), fb.dir().dirName());
        if (dirCmp != 0) return dirCmp < 0;
        // 再按文件名中的数字前缀排序
        return collator.compare(fa.fileName(), fb.fileName()) < 0;
    });

    // 3. 加载
    m_pixmaps.clear();
    m_pixmaps.reserve(files.size());
    m_codeToIndex.clear(); // legacy 模式不建立代码映射

    for (const auto& filePath : files) {
        QPixmap pix;
        if (pix.load(filePath, "BMP")) {
            m_pixmaps.append(pix);
        } else {
            qWarning() << "ImageManager: failed to load" << filePath;
            m_pixmaps.append(QPixmap()); // 占位
        }
    }

    qDebug() << "ImageManager: loaded" << m_pixmaps.size() << "images (legacy) from" << imageRootPath;
    return true;
}

void ImageManager::scanDir(const QString& dirPath, QVector<QString>& files)
{
    QDir dir(dirPath);
    // 只扫描 BMP 文件
    const auto entries = dir.entryInfoList(QStringList() << QStringLiteral("*.bmp"),
        QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto& info : entries)
        files.append(info.absoluteFilePath());

    // 递归子目录（各分类文件夹）
    const auto subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto& subDir : subDirs)
        scanDir(subDir.absoluteFilePath(), files);
}
