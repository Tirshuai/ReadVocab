#include "scenariomodewidget.h"
#include <QSqlDatabase>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QHBoxLayout>
#include <algorithm>
#include <QDialog>

ScenarioModeWidget::ScenarioModeWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("情境模式");
    setFixedSize(680, 700);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(20);
    lay->setContentsMargins(30, 30, 30, 30);

    textBrowser = new QTextBrowser();
    // 黑底白字样式
    textBrowser->setStyleSheet(R"(
        QTextBrowser {
            background-color: #000000;
            color: #ffffff;
            font-size: 17px;
            line-height: 1.7;
        }
        a {
            color: #4fc3f7;
            font-weight: bold;
            text-decoration: none;
        }
    )");
    textBrowser->setOpenLinks(false);
    textBrowser->setOpenExternalLinks(false);
    lay->addWidget(textBrowser);

    btnBack = new QPushButton("← 返回学习新词");
    btnBack->setFixedHeight(50);
    lay->addWidget(btnBack);

    connect(textBrowser, &QTextBrowser::anchorClicked, this, &ScenarioModeWidget::onWordClicked);
    connect(btnBack, &QPushButton::clicked, this, &ScenarioModeWidget::onBack);

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    query = new QSqlQuery(db);

    loadScenario();
}

void ScenarioModeWidget::loadScenario()
{
    QString html = R"(
        <p>When the fear began to <a href='word://grip'>grip</a> her, she tried to stay calm.</p>
        <p>The loud crash did not <a href='word://startle'>startle</a> the brave child.</p>
        <p>After five <a href='word://successive'>successive</a> victories, they won the championship.</p>
    )";
    textBrowser->setHtml(html);
}

void ScenarioModeWidget::onWordClicked(const QUrl &url)
{
    QString word = url.host();
    if (url.scheme() != "word" || word.isEmpty()) return;

    query->prepare("SELECT id, meaning FROM vocabulary WHERE word = ? LIMIT 1");
    query->addBindValue(word);

    if (!query->exec() || !query->next()) {
        QMessageBox::warning(this, "提示", "数据库中无该单词！");
        return;
    }

    int wordId = query->value(0).toInt();
    QString correctMeaning = query->value(1).toString();

    // 直接弹出答题，不再判断是否已学习
    showQuizDialog(wordId, word, correctMeaning);
}

// 弹窗：手动关闭，无自动延时
void ScenarioModeWidget::showQuizDialog(int wordId, const QString &word, const QString &correctMeaning)
{
    QDialog d(this);
    d.setWindowTitle("情境单词练习");
    d.setFixedSize(580, 520);

    QVBoxLayout *layout = new QVBoxLayout(&d);
    layout->setSpacing(22);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *labWord = new QLabel(word);
    labWord->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(32);
    f.setBold(true);
    labWord->setFont(f);
    layout->addWidget(labWord);

    // 取3个错误释义
    QStringList options;
    query->prepare("SELECT meaning FROM vocabulary WHERE word != ? ORDER BY RANDOM() LIMIT 3");
    query->addBindValue(word);
    query->exec();
    while (query->next()) options << query->value(0).toString();
    options << correctMeaning;
    std::shuffle(options.begin(), options.end(), *QRandomGenerator::global());

    QPushButton *optA = new QPushButton("1. " + options[0]);
    QPushButton *optB = new QPushButton("2. " + options[1]);
    QPushButton *optC = new QPushButton("3. " + options[2]);
    QPushButton *optD = new QPushButton("4. " + options[3]);

    QList<QPushButton*> opts = {optA, optB, optC, optD};
    for (auto btn : opts) {
        btn->setMinimumHeight(70);
        btn->setStyleSheet("text-align: left; padding:14px; font-size:16px;");
        layout->addWidget(btn);
    }

    // 点击答题、变色、入库，弹窗保留，手动关闭
    for (auto btn : opts) {
        connect(btn, &QPushButton::clicked, &d, [&, btn, wordId, correctMeaning, opts]() {
            bool correct = btn->text().endsWith(correctMeaning);

            for (auto b : opts) b->setEnabled(false);
            if (correct) {
                btn->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:14px; font-size:16px");
            } else {
                btn->setStyleSheet("background-color:#F44336; color:white;");
                for (auto b : opts) {
                    if (b->text().endsWith(correctMeaning)) {
                        b->setStyleSheet("background-color:#4CAF50; color:white; text-align:left; padding:14px; font-size:16px");
                    }
                }
            }
            saveRecord(wordId, correct);
        });
    }

    d.exec();
}

void ScenarioModeWidget::saveRecord(int wordId, bool correct)
{
    query->prepare(R"(
        INSERT OR REPLACE INTO learn_record
        (word_id, is_learned, study_date, is_correct)
        VALUES (?, 1, ?, ?)
    )");
    query->addBindValue(wordId);
    query->addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    query->addBindValue(correct ? 1 : 0);
    query->exec();
}

void ScenarioModeWidget::onBack()
{
    emit backToLearnNew();
    close();
}