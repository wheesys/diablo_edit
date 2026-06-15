/**
 * 小型工具：将 itemdata.txt 压缩为 itemdata.dat
 * 用法: build_itemdata <input.txt> <output.dat>
 */
#include <QCoreApplication>
#include <QFile>
#include <cstdio>
#include <string>

#include "src/core/compress_quicklz.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "用法: %s <input.txt> <output.dat>\n", argv[0]);
        return 1;
    }

    // 1. 读取输入文件
    QFile inFile(argv[1]);
    if (!inFile.open(QFile::ReadOnly)) {
        fprintf(stderr, "无法打开输入文件: %s\n", argv[1]);
        return 1;
    }
    QByteArray raw = inFile.readAll();
    inFile.close();

    // 2. 构建内容：版本字节 + "ITEM" magic + 原始内容
    //    loadCompressedFile 解压后检查 out_buf[1..4] == "ITEM"
    std::string content;
    content.push_back('\x01');  // 版本字节
    content.append("ITEM");      // magic
    content.append(raw.constData(), raw.size());

    // 3. QuickLZ 压缩
    std::string compressed;
    if (!CCompressorQuickLZ().compress(content, compressed)) {
        fprintf(stderr, "压缩失败!\n");
        return 1;
    }

    // 4. 写入输出文件
    QFile outFile(argv[2]);
    if (!outFile.open(QFile::WriteOnly)) {
        fprintf(stderr, "无法打开输出文件: %s\n", argv[2]);
        return 1;
    }
    outFile.write(compressed.data(), compressed.size());
    outFile.close();

    fprintf(stderr, "成功: %s (%zu bytes) -> %s (%zu bytes)\n",
            argv[1], content.size(), argv[2], compressed.size());
    return 0;
}
