#pragma once

#include "D2Types.h"
#include <QString>
#include <QByteArray>
#include <stdexcept>

// 前向声明 DataManager，实际实现在 src/data/DataManager.h
class DataManager;

// 全局 DataManager 指针，在应用启动时设置
extern DataManager* g_dataMgr;

// D2 文件解析异常（替代 MFC 的 throw ::theApp.MsgBoxInfo(n) 模式）
class D2Error : public std::runtime_error {
public:
    explicit D2Error(const QString& msg) : std::runtime_error(msg.toStdString()), m_msg(msg), m_code(-1) {}
    explicit D2Error(int msgId); // 从消息ID构建错误信息
    const QString& message() const { return m_msg; }
    int code() const { return m_code; }
private:
    QString m_msg;
    int m_code;
};

// 跨平台 UTF-8 名称编解码（替代 MultiByteToWideChar/WideCharToMultiByte）
QString DecodeCharName(const BYTE* name);
QByteArray EncodeCharName(const QString& name);

// 检查角色名称合法性
BOOL CheckCharName(const QString& name);
