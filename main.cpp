#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "mainwindow.h"
#include "fastmodewidget.h"
#include "scenariomodewidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ========== 全局唯一：vocabulary.db，连接名 wordconn ==========
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "wordconn");
    db.setDatabaseName("vocabulary.db");

    if (!db.open()) {
        qCritical() << "无法打开 vocabulary.db：" << db.lastError().text();
        return -1;
    }
    qDebug() << "成功连接到 vocabulary.db（全局连接 wordconn）";

    QSqlQuery query(db);

    // ==============================
    // 单词关联器 必需的两张表（精简稳定版）
    // ==============================
    query.exec("CREATE TABLE IF NOT EXISTS relate_group ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "group_name TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS relate_word ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "group_id INTEGER,"
               "word_id INTEGER)");

    // ==============================
    // 单词表（兼容旧表，不破坏数据）
    // ==============================
    query.exec("CREATE TABLE IF NOT EXISTS vocabulary ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "word TEXT NOT NULL UNIQUE,"
               "phonetic TEXT,"
               "part TEXT,"
               "meaning TEXT,"
               "example TEXT)");

    // ==============================
    // 学习记录表（兼容旧表）
    // ==============================
    query.exec("CREATE TABLE IF NOT EXISTS learn_record ("
               "word_id INTEGER,"
               "is_learned INTEGER DEFAULT 0,"
               "study_date TEXT,"
               "is_correct INTEGER DEFAULT 0,"
               "starred INTEGER DEFAULT 0,"
               "PRIMARY KEY(word_id)"
               ")");

    // ==============================
    // 复习记录表（你本次需要）
    // ==============================
    query.exec("CREATE TABLE IF NOT EXISTS review_history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "word TEXT NOT NULL,"
               "is_correct INTEGER NOT NULL,"
               "review_time DATETIME NOT NULL,"
               "mode TEXT NOT NULL)");

    // ==============================
    // 主窗口
    // ==============================
    MainWindow w;
    w.show();

    return a.exec();
}