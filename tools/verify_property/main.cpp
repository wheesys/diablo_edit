/**
 * @file main.cpp
 * @brief 验证 D2R 3.1 存档的属性解析 — 复用项目 DataManager + D2S_Struct
 *
 * 使用方式: verify_31 <d2s文件路径> <data目录路径>
 * 示例: verify_31 doc/杜五_v3.1.d2s data/
 */
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "core/D2S_Struct.h"
#include "core/D2Item.h"
#include "data/DataManager.h"

// 全局 DataManager 指针（项目代码依赖此全局变量）
DataManager* g_dataMgr = nullptr;

int main(int argc, char *argv[]) {
    fprintf(stderr, "VERIFY: main() started\n"); fflush(stderr);
    QCoreApplication app(argc, argv);

    if (argc < 3) {
        qCritical() << "用法:" << argv[0] << "<d2s文件> <data目录>";
        return 1;
    }

    // 1. 加载数据文件
    qInfo() << "=== 加载数据文件 ===";
    DataManager mgr;
    if (!mgr.loadAll(argv[2])) {
        fprintf(stderr, "VERIFY: loadAll failed\n"); fflush(stderr);
        qCritical() << "数据文件加载失败!";
        return 1;
    }
    g_dataMgr = &mgr;
    qInfo() << "  language:" << mgr.langCount() << "种语言";
    qInfo() << "  property: 共" << mgr.propertyNameSize() << "个属性名";
    qInfo() << "  itemdata:" << mgr.itemCategories().size() << "个分类";

    // 2. 读取存档
    qInfo() << "\n=== 读取存档 ===" << argv[1];
    QFile file(argv[1]);
    if (!file.open(QFile::ReadOnly)) {
        fprintf(stderr, "VERIFY: file open failed\n"); fflush(stderr);
        qCritical() << "无法打开存档文件!";
        return 1;
    }
    QByteArray data = file.readAll();
    file.close();
    qInfo() << "  文件大小:" << data.size() << "字节";

    // 3. 解析存档
    CD2S_Struct d2s;
    try {
        CInBitsStream bs(reinterpret_cast<const BYTE*>(data.constData()), data.size());
        d2s.ReadData(bs);
    } catch (const D2Error &e) {
        fprintf(stderr, "VERIFY: ReadData threw D2Error code=%d\n", e.code()); fflush(stderr);
        qCritical() << "解析失败:" << e.message();
        return 1;
    }
    qInfo() << "  版本:" << Qt::hex << d2s.dwVersion;
    qInfo() << "  角色等级:" << d2s.charLevel;
    qInfo() << "  职业:" << d2s.charClass;

    // 4. 统计物品属性
    qInfo() << "\n=== 物品属性验证 ===";
    const auto &items = d2s.ItemList.vItems;
    qInfo() << "  物品总数:" << items.size();

    int totalProps = 0;
    int maxPropId = 0;

    for (size_t i = 0; i < items.size(); ++i) {
        const auto &item = items[i];

        // 获取物品名称(可能因数据无效而崩溃，保守处理)
        QString itemName;
        try {
            itemName = item.ItemName();
        } catch (...) {
            itemName = QStringLiteral("(unknown)");
        }

        // 获取属性列表
        if (!item.HasPropertyList()) continue;

        const CPropertyList &propList = item.pItemInfo->pTpSpInfo->stPropertyList;
        int itemPropCount = (int)propList.mProperty.size();
        totalProps += itemPropCount;

        qInfo() << "\n  [" << i << "]" << itemName
                 << "-" << itemPropCount << "个属性:";

        for (const auto &prop : propList.mProperty) {
                WORD propId = prop.first;
                if (propId > maxPropId) maxPropId = propId;
                DWORD propVal = prop.second;
                QString desc = mgr.propertyDescription(d2s.dwVersion, propId, propVal);
                if (desc.isEmpty())
                    desc = mgr.propertyDescription(d2s.dwVersion, propId);

                qInfo() << "    ID=" << propId
                         << "值=" << propVal << "→" << desc;
            }
        }

    qInfo() << "\n=== 验证结论 ===";
    qInfo() << "  总属性数:" << totalProps;
    qInfo() << "  最大属性ID:" << maxPropId;
    qInfo() << "  property.dat 属性定义数:" << mgr.propertyNameSize();
    if (maxPropId < (int)mgr.propertyNameSize()) {
        qInfo() << "  结论: ✅ property.dat 完全兼容3.1存档，所有属性ID均在定义范围内";
    } else {
        qWarning() << "  结论: ❌ 存在超出 property.dat 定义范围的属性ID!";
    }
    fprintf(stderr, "VERIFY: success\n"); fflush(stderr);
    return 0;
}
