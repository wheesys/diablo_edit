/**
 * 独立测试 Merc 段读取 — 验证 v105 non-simple padding 修复
 */
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <cstdio>
#include <vector>

#include "core/D2Item.h"
#include "core/D2S_Struct.h"
#include "data/DataManager.h"

DataManager* g_dataMgr = nullptr;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    if (argc < 3) {
        qCritical() << "用法:" << argv[0] << "<d2s文件> <data目录>";
        return 1;
    }

    DataManager mgr;
    if (!mgr.loadAll(argv[2])) {
        qCritical() << "数据加载失败!";
        return 1;
    }
    g_dataMgr = &mgr;

    QFile file(argv[1]);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << "无法打开文件!";
        return 1;
    }
    QByteArray fileData = file.readAll();
    file.close();

    fprintf(stderr, "=== 测试 Merc ItemList (从字节 2014) ===\n");

    const BYTE* raw = reinterpret_cast<const BYTE*>(fileData.constData());

    // 定位到 merc ItemList: file byte 2014 (0x07DE)
    CInBitsStream bs(raw + 2014, fileData.size() - 2014);
    fprintf(stderr, "  Stream at byte 0x%04X\n", bs.BytePos());

    // CItemList::ReadData
    WORD wMagic, nItems;
    bs >> wMagic >> nItems;
    fprintf(stderr, "  JM=0x%04X nItems=%u\n", wMagic, nItems);
    if (wMagic != 0x4D4A) {
        fprintf(stderr, "  ERROR: Bad JM magic!\n");
        return 1;
    }

    int ok = 0, fail = 0;
    for (int i = 0; i < nItems && bs.Good(); ++i) {
        DWORD startByte = bs.BytePos();
        CD2Item item;
        try {
            item.ReadData(bs, 0x69);
            DWORD typeID = item.MetaData().dwTypeID;
            DWORD endByte = bs.BytePos();
            char codeStr[5] = {0};
            codeStr[0] = (typeID) & 0xFF;
            codeStr[1] = (typeID >> 8) & 0xFF;
            codeStr[2] = (typeID >> 16) & 0xFF;
            codeStr[3] = (typeID >> 24) & 0xFF;
            fprintf(stderr, "  Merc[%d] @ 0x%04X->0x%04X: '%s' simple=%d OK\n",
                    i, startByte, endByte,
                    codeStr,
                    item.bSimple ? 1 : 0);
            ok++;
        } catch (const D2Error& e) {
            fprintf(stderr, "  Merc[%d] @ 0x%04X: FAIL D2Error(%d)\n", i, startByte, e.code());
            fail++;
        } catch (const std::exception& e) {
            fprintf(stderr, "  Merc[%d] @ 0x%04X: FAIL %s\n", i, startByte, e.what());
            fail++;
        }
    }
    fprintf(stderr, "  Result: %d OK, %d FAIL\n", ok, fail);
    return fail > 0 ? 1 : 0;
}
