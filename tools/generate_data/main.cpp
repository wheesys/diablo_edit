/**
 * @file main.cpp
 * @brief 独立命令行工具：将 .txt 文本数据编译为 QuickLZ 压缩的 .dat 二进制文件
 *
 * 用法：
 *   generate_data <input.txt> <output.dat>
 *   generate_data --all              （预留批量转换，暂未实现）
 *
 * 不依赖 Qt，纯 C++17 标准库 + QuickLZ。
 */

#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>

// QuickLZ 压缩接口（来自 src/core/）
#include "compress_quicklz.h"

using namespace std;

/**
 * @brief 读取文本文件，压缩后写入二进制 .dat 文件
 * @param input  输入 .txt 文件路径
 * @param output 输出 .dat 文件路径
 * @return 成功返回 true，失败返回 false（错误信息已打印到 stderr）
 */
bool generateData(const char* input, const char* output)
{
    if (!input || !output) {
        cerr << "错误：输入或输出路径为空" << endl;
        return false;
    }

    // 1. 读取输入文件全部内容
    ifstream inf(input, ios_base::binary);
    if (!inf.is_open()) {
        cerr << "错误：无法打开输入文件 '" << input << "'" << endl;
        return false;
    }
    string in_buf;
    in_buf.assign(istreambuf_iterator<char>(inf), istreambuf_iterator<char>());
    inf.close();

    if (in_buf.empty()) {
        cerr << "警告：输入文件 '" << input << "' 内容为空" << endl;
    }

    // 2. 使用 QuickLZ level 3 压缩
    string out_buf;
    if (!CCompressorQuickLZ().compress(in_buf, out_buf)) {
        cerr << "错误：压缩失败（输入大小=" << in_buf.size() << " 字节）" << endl;
        return false;
    }

    // 3. 写入输出文件
    ofstream outf(output, ios_base::binary | ios_base::trunc);
    if (!outf.is_open()) {
        cerr << "错误：无法创建输出文件 '" << output << "'" << endl;
        return false;
    }
    outf.write(out_buf.c_str(), static_cast<streamsize>(out_buf.size()));
    outf.close();

    if (!outf.good()) {
        cerr << "错误：写入输出文件失败 '" << output << "'" << endl;
        return false;
    }

    return true;
}

/**
 * @brief 打印使用说明
 */
void printUsage(const char* progName)
{
    cout << "用法：" << endl;
    cout << "  " << progName << " <输入.txt> <输出.dat>" << endl;
    cout << endl;
    cout << "说明：" << endl;
    cout << "  将文本数据文件（.txt）以 QuickLZ level 3 压缩为二进制文件（.dat）。" << endl;
    cout << endl;
    cout << "示例：" << endl;
    cout << "  " << progName << " itemdata.txt itemdata.dat" << endl;
    cout << "  " << progName << " property.txt property.dat" << endl;
    cout << "  " << progName << " language.txt language.dat" << endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printUsage(argc > 0 ? argv[0] : "generate_data");
        return 1;
    }

    const char* input  = argv[1];
    const char* output = argv[2];

    cout << "正在编译 " << input << " -> " << output << " ..." << endl;

    if (!generateData(input, output)) {
        cerr << "编译失败！" << endl;
        return 1;
    }

    cout << "编译成功！" << endl;
    return 0;
}
