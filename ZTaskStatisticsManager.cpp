//===================================================
#include "ZTaskStatisticsManager.h"
#include "ZTrainingManager.h"

#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

//===================================================
ZTaskStatisticsManager::ZTaskStatisticsManager(QObject *parent)
    : QObject{parent}
{

}
//===================================================
void ZTaskStatisticsManager::zp_connectToTrainingManager(ZTrainingManager* manager)
{
    zv_trainingManager = manager;
    connect(zv_trainingManager, &ZTrainingManager::zg_symbolPressed,
            this, &ZTaskStatisticsManager::zp_updateStatistics);
    connect(zv_trainingManager, &ZTrainingManager::zg_taskStateChanged,
            this, &ZTaskStatisticsManager::zp_onTaskStateChange);

}
//===================================================
QMap<QString, WordStatistics> ZTaskStatisticsManager::zp_statistics() const
{
    return zv_statistics;
}
//===================================================
void ZTaskStatisticsManager::zp_updateStatistics(QString pressedSymbol)
{
    if(zv_trainingManager->zp_isWrong())
    {
        zv_errorCount++;
        return;
    }

    static QRegularExpression re("[\\s\\t]");
    if(re.match(pressedSymbol).hasMatch())
    {
        zh_resetCurrentWordStatistics();
        return;
    }

    zv_currentWord.append(pressedSymbol);

    // if new line or current symbol is space
    if(zv_trainingManager->zp_currentSymbolIndex() == 0  ||
            zv_trainingManager->zp_currentSymbolIndex() == zv_trainingManager->zp_currentLine().count())
    {
        zh_registerCurrentWordStatistics();
        zh_resetCurrentWordStatistics();
    }
    else if(re.match(zv_trainingManager->zp_currentSymbol()).hasMatch())
    {
        zh_registerCurrentWordStatistics();
    }
}
//===================================================
void ZTaskStatisticsManager::zh_resetCurrentWordStatistics()
{
    zv_wordStartTimeMark = QDateTime::currentMSecsSinceEpoch();
    zv_errorCount = 0;
    zv_currentWord = QString();
}
//===================================================
void ZTaskStatisticsManager::zh_registerCurrentWordStatistics()
{
    if(zv_currentWord.isEmpty())
    {
        return;
    }

    qint64 wordDurationMsec =  QDateTime::currentMSecsSinceEpoch() - zv_wordStartTimeMark;

    zv_statistics[zv_currentWord].typingCount = zv_statistics[zv_currentWord].typingCount + 1;
    zv_statistics[zv_currentWord].errorCount = zv_statistics[zv_currentWord].errorCount + zv_errorCount;
    zv_statistics[zv_currentWord].durationMsec = zv_statistics[zv_currentWord].durationMsec + wordDurationMsec;

    qDebug() << zv_currentWord
             << "DUR:" << zv_statistics[zv_currentWord].durationMsec/1000.0
             << "SP:" << 60000.0 * zv_currentWord.count() / (zv_statistics[zv_currentWord].durationMsec / zv_statistics[zv_currentWord].typingCount)
             << "spm" << "err:" << zv_statistics[zv_currentWord].errorCount;

}
//===================================================
void ZTaskStatisticsManager::zp_onTaskStateChange(ZTrainingManager::TASK_STATE previous,
                                                  ZTrainingManager::TASK_STATE current)
{
    if(current == ZTrainingManager::TS_PAUSED)
    {
        qDebug() << "TASK STATE PAUSED" << current;
        if(previous == ZTrainingManager::TS_ACTIVE)
        {
            zv_pauseStartTimeMark = QDateTime::currentMSecsSinceEpoch();
        }
    }
    else if(current == ZTrainingManager::TS_ACTIVE)
    {
        qDebug() << "TASK STATE ACTIVE" << current;
        if(previous == ZTrainingManager::TS_PAUSED)
        {
            qint64 pauseDuration = QDateTime::currentMSecsSinceEpoch() - zv_pauseStartTimeMark;
            zv_wordStartTimeMark = zv_wordStartTimeMark + pauseDuration;
            zv_taskStartTimeMark = zv_taskStartTimeMark + pauseDuration;
        }
        else
        {
            zh_resetCurrentWordStatistics();
            zv_statistics.clear();
        }
    }
    else if(current == ZTrainingManager::TS_COMPLETED)
    {
        qDebug() << "TASK STATE COMPLETED" << current;
        //zh_registerCurrentWordStatistics();
        //zh_resetCurrentWordStatistics();
    }
    else // TS_INACTIVE
    {
        zh_resetCurrentWordStatistics();
    }
}
//===================================================

