#ifndef MEMORYHELPER_H
#define MEMORYHELPER_H

#include <QObject>
#include <QList>
#include <QDate>

// 单词记忆信息结构体
struct WordMemoryInfo{
    double retention;   // 记忆留存率R，越小遗忘越严重
    int dayDiff;        // 距离上次复习间隔天数
    int wordId;
};

class MemoryHelper : public QObject
{
    Q_OBJECT
public:
    explicit MemoryHelper(QObject *parent = nullptr);

    // 1. 计算单词记忆稳定度S
    double calcWordStability(int wordId);

    // 2. 计算单个单词完整记忆数据
    WordMemoryInfo getWordMemoryData(int wordId, const QString& wordText, bool isStarred);

    // 3. 获取按遗忘程度排序的待复习单词ID列表
    QList<int> getReviewWordList(int maxCount = 20, int minDayGap = 1);

    // 4. 保存本次答题复习记录（答对/答错）
    void saveReviewRecord(const QString& wordText, bool isCorrect, const QString& mode);
};

#endif // MEMORYHELPER_H