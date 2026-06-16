#pragma once

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QVector>

#include "D2Types.h"  // for DWORD, BYTE

/**
 * ImageManager - 单例图片管理器
 * 启动时按 image_order.txt 指定的顺序加载图片，
 * 支持通过物品代码（4字符 typeCode）或 PicIndex 查找图片。
 */
class ImageManager : public QObject
{
    Q_OBJECT

public:
    static ImageManager* instance();

    /**
     * 加载图片。自动检测 image_order.txt：
     *   存在 → 按顺序文件加载（MFC RC 兼容顺序）
     *   不存在 → 按目录扫描加载（降级）
     * @param imageRootPath 图片根目录路径 (resources/images)
     * @return true 成功
     */
    bool loadAll(const QString& imageRootPath);

    /**
     * 获取 PicIndex 对应图片（旧 API，保持兼容）
     */
    QPixmap getPixmap(int picIndex) const;

    /**
     * 通过物品类型代码获取图片（新 API）
     * @param typeCode 4字符物品代码，如 "elx", "hpo"
     */
    QPixmap getPixmapByCode(const char* typeCode) const;

    /**
     * 通过物品类型代码获取 PicIndex
     * @return PicIndex，未找到返回 -1
     */
    int picIndexForCode(const char* typeCode) const;

    /** 图片总数 */
    int count() const { return m_pixmaps.size(); }

    /** 是否已加载 */
    bool isLoaded() const { return !m_pixmaps.isEmpty(); }

private:
    explicit ImageManager(QObject* parent = nullptr);
    ~ImageManager() override = default;
    ImageManager(const ImageManager&) = delete;
    ImageManager& operator=(const ImageManager&) = delete;

    /** 旧版：递归扫描目录加载（降级方案） */
    void scanDir(const QString& dirPath, QVector<QString>& files);
    bool loadLegacy(const QString& imageRootPath);

    /** 新版：按 image_order.txt 加载 */
    bool loadWithOrderFile(const QString& imageRootPath, const QString& orderFile);

    static ImageManager* s_instance;
    QVector<QPixmap> m_pixmaps;         ///< picIndex -> QPixmap
    QHash<DWORD, int> m_codeToIndex;    ///< DWTypeID -> picIndex
};
