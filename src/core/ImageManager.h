#pragma once

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QString>

/**
 * ImageManager - 单例图片管理器
 * 启动时扫描 resources/images/ 下所有 BMP 文件，按数字前缀排序
 * 排序索引与 CItemMetaData::PicIndex 对应
 */
class ImageManager : public QObject
{
    Q_OBJECT

public:
    static ImageManager* instance();

    /**
     * 加载 resources/images/ 下所有 BMP 图片
     * @param imageRootPath 图片根目录路径 (resources/images)
     * @return true 成功
     */
    bool loadAll(const QString& imageRootPath);

    /**
     * 获取 PicIndex 对应图片
     * @param picIndex 图片索引，与 CItemMetaData::PicIndex 对应
     * @return QPixmap，加载失败返回空 QPixmap
     */
    QPixmap getPixmap(int picIndex) const;

    /** 图片总数 */
    int count() const { return m_pixmaps.size(); }

    /** 是否已加载 */
    bool isLoaded() const { return !m_pixmaps.isEmpty(); }

private:
    explicit ImageManager(QObject* parent = nullptr);
    ~ImageManager() override = default;
    ImageManager(const ImageManager&) = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    /** 递归扫描目录，按数字前缀收集 BMP 文件路径 */
    void scanDir(const QString& dirPath, QVector<QString>& files);

    static ImageManager* s_instance;
    QVector<QPixmap> m_pixmaps; ///< picIndex -> QPixmap
};
