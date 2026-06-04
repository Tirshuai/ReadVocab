#include "wordbookmanager.h"
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

namespace WordbookManager {

static QString g_currentWordbook;

QString currentWordbook() {
    return g_currentWordbook;
}

QString configFilePath() {
    return QCoreApplication::applicationDirPath() + "/wordbook_config.txt";
}

QString dbPath(const QString &name) {
    // 在多个可能的位置搜索 .db 文件
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList searchDirs;

    // 1. 可执行文件同目录下的 lib/（部署场景）
    searchDirs << appDir + "/lib";
    // 2. 可执行文件上级目录的 lib/（Qt Creator 典型 build/xxx/ 结构）
    searchDirs << appDir + "/../lib";
    // 3. 可执行文件上两级目录的 lib/（build/xxx/debug/ 结构）
    searchDirs << appDir + "/../../lib";
    // 4. 当前工作目录下的 lib/
    searchDirs << QDir::currentPath() + "/lib";

    QString fileName = name + ".db";
    for (const QString &dir : searchDirs) {
        QString fullPath = QDir(dir).absoluteFilePath(fileName);
        if (QFile::exists(fullPath)) {
            return QDir::cleanPath(fullPath);
        }
    }

    // 都没找到，默认返回部署路径（让 SQLite 报错提示）
    return QDir::cleanPath(appDir + "/lib/" + fileName);
}

// 保存当前词书选择到配置文件
static void saveConfig(const QString &name) {
    QFile file(configFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << name;
        file.close();
    }
}

// 创建所有需要的表（与之前的 main.cpp 一致）
static void createTables(QSqlDatabase &db) {
    QSqlQuery query(db);

    query.exec("CREATE TABLE IF NOT EXISTS vocabulary ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "word TEXT NOT NULL UNIQUE,"
               "phonetic TEXT,"
               "part TEXT,"
               "meaning TEXT,"
               "example TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS learn_record ("
               "word_id INTEGER,"
               "is_learned INTEGER DEFAULT 0,"
               "study_date TEXT,"
               "is_correct INTEGER DEFAULT 0,"
               "starred INTEGER DEFAULT 0,"
               "PRIMARY KEY(word_id)"
               ")");

    query.exec("CREATE TABLE IF NOT EXISTS review_history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "word TEXT NOT NULL,"
               "is_correct INTEGER NOT NULL,"
               "review_time DATETIME NOT NULL,"
               "mode TEXT NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS relate_group ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "group_name TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS relate_word ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "group_id INTEGER,"
               "word_id INTEGER)");
}

bool switchWordbook(const QString &name) {
    // 1. 关闭旧连接
    {
        QSqlDatabase db = QSqlDatabase::database("wordconn");
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase("wordconn");

    // 2. 打开新连接
    QString path = dbPath(name);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "wordconn");
    db.setDatabaseName(path);

    if (!db.open()) {
        qCritical() << "切换词书失败：" << path << db.lastError().text();
        return false;
    }

    qDebug() << "词书已切换至：" << name << "(" << path << ")";

    // 3. 创建表
    createTables(db);

    // 4. 保存配置
    g_currentWordbook = name;
    saveConfig(name);

    return true;
}

QString initWordbook() {
    // 尝试读取配置文件
    QFile file(configFilePath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString name = stream.readLine().trimmed();
        file.close();

        // 验证词书名是否有效
        QStringList valid = availableWordbooks();
        if (valid.contains(name)) {
            // 尝试连接
            QString path = dbPath(name);
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "wordconn");
            db.setDatabaseName(path);

            if (db.open()) {
                g_currentWordbook = name;
                createTables(db);
                qDebug() << "已加载词书：" << name << "(" << path << ")";
                return name;
            } else {
                qWarning() << "无法打开已保存的词书数据库：" << path;
                QSqlDatabase::removeDatabase("wordconn");
            }
        }
    }

    // 配置文件不存在或无效，返回空字符串表示需要用户选择
    return QString();
}

} // namespace WordbookManager
