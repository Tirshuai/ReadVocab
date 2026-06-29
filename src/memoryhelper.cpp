#include "memoryhelper.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <cmath>
#include <algorithm>

MemoryHelper::MemoryHelper(QObject *parent)
    : QObject(parent)
{
}

double MemoryHelper::calcWordStability(int wordId)
{
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    q.prepare(R"(
        SELECT rh.is_correct
        FROM vocabulary v
        JOIN review_history rh ON v.word = rh.word
        WHERE v.id = :wid
        ORDER BY rh.review_time ASC
    )");
    q.bindValue(":wid", wordId);
    if (!q.exec()) return 1.0;

    double S = 1.0;
    while (q.next())
    {
        bool ok = q.value("is_correct").toBool();
        if (ok)
            S *= 1.3;
        else
            S *= 0.4;
    }
    return S;
}

WordMemoryInfo MemoryHelper::getWordMemoryData(int wordId, const QString &wordText, bool isStarred)
{
    WordMemoryInfo res;
    res.wordId = wordId;
    res.retention = 1.0;
    res.dayDiff = 0;

    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    // 查询最后一次复习时间
    q.prepare("SELECT MAX(review_time) last_t FROM review_history WHERE word = :w");
    q.bindValue(":w", wordText);
    QString lastReviewTime;
    if (q.exec() && q.next())
        lastReviewTime = q.value("last_t").toString();

    QDate today = QDate::currentDate();
    double S = calcWordStability(wordId);
    int t;

    if (lastReviewTime.isEmpty())
    {
        // 无复习记录，使用首次学习日期计算间隔
        q.prepare("SELECT study_date FROM learn_record WHERE word_id = :wid");
        q.bindValue(":wid", wordId);
        q.exec();
        QDate studyDay = QDate::fromString(q.value("study_date").toString(), "yyyy-MM-dd");
        t = studyDay.daysTo(today);
    }
    else
    {
        QDate lastRevDay = QDate::fromString(lastReviewTime, "yyyy-MM-dd");
        t = lastRevDay.daysTo(today);
    }
    res.dayDiff = t;

    // 艾宾浩斯指数衰减公式
    double baseR = exp(-1.0 * t / S);
    // 着重单词加权降低留存，优先复习
    if (isStarred)
        baseR *= 0.7;
    res.retention = baseR;
    return res;
}

QList<int> MemoryHelper::getReviewWordList(int maxCount, int minDayGap)
{
    QList<WordMemoryInfo> priorityHigh;  // 满足遗忘间隔，优先复习
    QList<WordMemoryInfo> priorityLow;  // 未到遗忘间隔，兜底复习
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    // 读取所有已学单词
    q.prepare(R"(
        SELECT v.id, v.word, lr.starred
        FROM vocabulary v
        JOIN learn_record lr ON v.id = lr.word_id
        WHERE lr.is_learned = 1
    )");
    if (!q.exec()) return {};

    while (q.next())
    {
        int wid = q.value("id").toInt();
        QString wText = q.value("word").toString();
        bool star = q.value("starred").toInt() == 1;
        WordMemoryInfo info = getWordMemoryData(wid, wText, star);
        // 分层
        if (info.dayDiff >= minDayGap)
            priorityHigh.append(info);
        else
            priorityLow.append(info);
    }

    // 两层分别按留存率升序（遗忘越重越靠前）
    std::sort(priorityHigh.begin(), priorityHigh.end(), [](const WordMemoryInfo& a, const WordMemoryInfo& b){
        return a.retention < b.retention;
    });
    std::sort(priorityLow.begin(), priorityLow.end(), [](const WordMemoryInfo& a, const WordMemoryInfo& b){
        return a.retention < b.retention;
    });

    // 合并：遗忘优先在前，兜底在后
    QList<WordMemoryInfo> totalList = priorityHigh + priorityLow;

    // 截取指定数量单词
    QList<int> result;
    int takeNum = qMin(maxCount, totalList.size());
    for (int i = 0; i < takeNum; i++)
        result.append(totalList[i].wordId);
    return result;
}
void MemoryHelper::saveReviewRecord(const QString &wordText, bool isCorrect, const QString &mode)
{
    QSqlDatabase db = QSqlDatabase::database("wordconn");
    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO review_history(word, is_correct, review_time, mode)
        VALUES(:w, :ok, :t, :m)
    )");
    q.bindValue(":w", wordText);
    q.bindValue(":ok", isCorrect ? 1 : 0);
    q.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    q.bindValue(":m", mode);
    q.exec();
}