#pragma once

#include <QString>
#include <QDir>

/// 测试工具：共享的工具函数
namespace TestUtils {

/// 获取项目根目录（使用编译期注入或运行时推断）
QString projectRoot();

/// 获取测试数据目录
QString dataDir();

/// 获取图片目录
QString imagesDir();

/// 获取参考截图目录
QString referenceDir();

} // namespace TestUtils
