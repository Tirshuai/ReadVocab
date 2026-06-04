#ifndef WORDBOOKMANAGER_H
#define WORDBOOKMANAGER_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>

// 词书管理器：负责词书选择、切换、配置持久化
namespace WordbookManager {

// 可选的词书列表
inline QStringList availableWordbooks() {
    return {"cet4", "cet6", "ky", "toefl", "gre", "ielts"};
}

// 词书对应的中文名
inline QString displayName(const QString &tag) {
    if (tag == "cet4")   return QString::fromUtf8("CET-4 (大学英语四级)");
    if (tag == "cet6")   return QString::fromUtf8("CET-6 (大学英语六级)");
    if (tag == "ky")     return QString::fromUtf8("考研英语");
    if (tag == "toefl")  return QString::fromUtf8("TOEFL (托福)");
    if (tag == "gre")    return QString::fromUtf8("GRE");
    if (tag == "ielts")  return QString::fromUtf8("IELTS (雅思)");
    return tag;
}

// 获取当前词书名称
QString currentWordbook();

// 切换词书（关闭旧连接 → 打开新连接 → 创建表 → 保存配置）
// 返回 true 表示切换成功
bool switchWordbook(const QString &name);

// 初始化：读取配置文件，连接数据库。如果无配置则返回空字符串让调用方弹窗选择
QString initWordbook();

// 获取 .db 文件路径
QString dbPath(const QString &name);

} // namespace WordbookManager

#endif // WORDBOOKMANAGER_H
