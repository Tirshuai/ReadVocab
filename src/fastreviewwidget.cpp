#include "fastreviewwidget.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

FastReviewWidget::FastReviewWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("快速复习");
    setFixedSize(580, 780); // 变大！和学习页面一样
    initUI();
}

void FastReviewWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(22);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // ========== 返回按钮 ==========
    QHBoxLayout *topLayout = new QHBoxLayout;
    QPushButton *btnBack = new QPushButton("← 返回");
    topLayout->addWidget(btnBack);
    topLayout->addStretch();

    // ========== 进度条 ==========
    labelProgress = new QLabel("进度：0/5");
    labelProgress->setAlignment(Qt::AlignCenter);

    // ========== 单词显示 ==========
    labelWord = new QLabel("WORD");
    labelWord->setAlignment(Qt::AlignCenter);
    QFont wordFont;
    wordFont.setPointSize(34);
    wordFont.setBold(true);
    labelWord->setFont(wordFont);

    // ========== 四个选项按钮 ==========
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->setSpacing(14);

    for (int i = 0; i < 4; i++) {
        options[i] = new QPushButton();
        options[i]->setMinimumHeight(70);
        optionLayout->addWidget(options[i]);
        connect(options[i], &QPushButton::clicked, this, &FastReviewWidget::onOptionClicked);
    }

    // ========== 下一题 ==========
    btnNext = new QPushButton("下一题");
    btnNext->setFixedHeight(65);
    btnNext->setVisible(false);
    connect(btnNext, &QPushButton::clicked, this, &FastReviewWidget::nextQuestion);

    // ========== 布局组装 ==========
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(labelProgress);
    mainLayout->addWidget(labelWord);
    mainLayout->addLayout(optionLayout);
    mainLayout->addWidget(btnNext);

    // ========== 全局暗黑风格：黑底白字 ==========
    setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
        }
        QPushButton {
            background-color: #1E1E1E;
            color: white;
            border: none;
            border-radius: 6px;
            text-align: left;
            padding: 12px;
            font-size: 15px;
        }
        QPushButton:hover {
            background-color: #333333;
        }
    )");

    connect(btnBack, &QPushButton::clicked, this, [this]() {
        emit backToReview();
        close();
    });
}

void FastReviewWidget::startReview()
{
    currentIndex = 0;
    correctCount = 0;
    loadLast5Words();

    if (wordList.isEmpty()) {
        QMessageBox::information(this, "提示", "暂无学习过的单词！");
        emit backToReview();
        close();
        return;
    }

    showCurrentQuestion();
}

void FastReviewWidget::loadLast5Words()
{
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    q.exec("SELECT v.word, v.meaning FROM vocabulary v "
           "JOIN learn_record r ON v.id = r.word_id "
           "ORDER BY r.study_date DESC LIMIT 5");

    wordList.clear();
    while (q.next()) {
        QVariantMap m;
        m["word"] = q.value("word");
        m["meaning"] = q.value("meaning");
        wordList.append(m);
    }
}

void FastReviewWidget::showCurrentQuestion()
{
    if (currentIndex >= wordList.size()) {
        showFinishDialog();
        return;
    }

    btnNext->setVisible(false);
    auto map = wordList[currentIndex];
    QString word = map["word"].toString();
    correctMeaning = map["meaning"].toString();

    labelWord->setText(word);
    labelProgress->setText(QString("进度：%1/%2").arg(currentIndex + 1).arg(wordList.size()));

    // 重置所有按钮
    for (int i = 0; i < 4; i++) {
        options[i]->setEnabled(true);
        options[i]->setStyleSheet("");
    }

    // 获取3个错误答案
    QStringList wrongOptions = getRandomWrongMeanings(correctMeaning, 3);
    QStringList allOptions = wrongOptions << correctMeaning;

    // 打乱顺序
    std::shuffle(allOptions.begin(), allOptions.end(), *QRandomGenerator::global());

    // 设置按钮文字
    options[0]->setText("1. " + allOptions[0]);
    options[1]->setText("2. " + allOptions[1]);
    options[2]->setText("3. " + allOptions[2]);
    options[3]->setText("4. " + allOptions[3]);

    // 找到正确按钮
    correctBtn = nullptr;
    for (auto *btn : options) {
        if (btn->text().endsWith(correctMeaning)) {
            correctBtn = btn;
            break;
        }
    }
}

void FastReviewWidget::onOptionClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    bool isCorrect = (clickedBtn == correctBtn);

    // 禁用所有按钮
    for (auto *btn : options) btn->setEnabled(false);

    // 高亮
    if (isCorrect) {
        clickedBtn->setStyleSheet("background-color:#4CAF50; color:white;");
        correctCount++;
    } else {
        clickedBtn->setStyleSheet("background-color:#F44336; color:white;");
        correctBtn->setStyleSheet("background-color:#4CAF50; color:white;");
    }

    // 保存复习记录
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    q.prepare("INSERT INTO review_history (word, is_correct, review_time, mode) "
              "VALUES (?, ?, datetime('now','localtime'), 'fast')");
    q.addBindValue(wordList[currentIndex]["word"].toString());
    q.addBindValue(isCorrect ? 1 : 0);
    q.exec();

    btnNext->setVisible(true);
}

void FastReviewWidget::nextQuestion()
{
    currentIndex++;
    showCurrentQuestion();
}

void FastReviewWidget::showFinishDialog()
{
    QMessageBox::information(this, "复习完成",
                             QString("✅ 复习结束！\n正确：%1\n总数：%2\n正确率：%3%")
                                 .arg(correctCount)
                                 .arg(wordList.size())
                                 .arg(wordList.isEmpty() ? 0 : (correctCount * 100 / wordList.size())));

    emit backToReview();
    close();
}

QStringList FastReviewWidget::getRandomWrongMeanings(const QString &correct, int count)
{
    QStringList list;
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);

    QString temp = correct;
    temp.replace("'", "''");

    q.exec(QString("SELECT meaning FROM vocabulary WHERE meaning != '%1' ORDER BY random() LIMIT %2")
               .arg(temp).arg(count));

    while (q.next()) list << q.value(0).toString();
    while (list.size() < count) list << "错误释义";

    return list;
}