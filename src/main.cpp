#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "fastmodewidget.h"
#include "scenariomodewidget.h"
#include "wordbookmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ========== 词书初始化 ==========
    QString wordbook = WordbookManager::initWordbook();

    // 如果没有配置文件（首次启动），弹窗让用户选择词书
    if (wordbook.isEmpty()) {
        QStringList items;
        QStringList tags = WordbookManager::availableWordbooks();
        for (const QString &tag : tags) {
            items << WordbookManager::displayName(tag);
        }

        bool ok = false;
        QString choice = QInputDialog::getItem(
            nullptr,
            QString::fromUtf8("选择词书"),
            QString::fromUtf8("欢迎使用单词背诵软件！\n请选择您要使用的词书："),
            items,
            0,      // 默认选中第一个
            false,  // 不可编辑
            &ok
        );

        if (!ok || choice.isEmpty()) {
            // 用户取消了选择，默认使用 CET-4
            wordbook = "cet4";
        } else {
            // 找到对应的 tag
            int idx = items.indexOf(choice);
            wordbook = (idx >= 0 && idx < tags.size()) ? tags[idx] : "cet4";
        }

        if (!WordbookManager::switchWordbook(wordbook)) {
            QMessageBox::critical(nullptr, QString::fromUtf8("错误"),
                                  QString::fromUtf8("无法打开词书数据库！"));
            return -1;
        }
    }

    qDebug() << "当前词书：" << WordbookManager::currentWordbook()
             << "(" << WordbookManager::displayName(WordbookManager::currentWordbook()) << ")";

    // ========== 主窗口 ==========
    MainWindow w;
    w.show();

    return a.exec();
}
