#include "ImageManager.h"

#include <QDir>
#include <QFileInfo>
#include <QCollator>
#include <QThread>
#include <QDebug>

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

    // 3. 按排序顺序加载所有 BMP
    m_pixmaps.clear();
    m_pixmaps.reserve(files.size());

    for (const auto& filePath : files) {
        QPixmap pix;
        if (pix.load(filePath, "BMP")) {
            m_pixmaps.append(pix);
        } else {
            qWarning() << "ImageManager: failed to load" << filePath;
            m_pixmaps.append(QPixmap()); // 占位
        }
    }

    qDebug() << "ImageManager: loaded" << m_pixmaps.size() << "images from" << imageRootPath;
    return true;
}

QPixmap ImageManager::getPixmap(int picIndex) const
{
    if (picIndex < 0 || picIndex >= m_pixmaps.size())
        return QPixmap();
    return m_pixmaps[picIndex];
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
