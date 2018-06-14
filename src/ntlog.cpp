#include "ntlog.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTLog *NTLog::instance = 0;

void NTLog::log(QString message, LogLevel logLevel, QString component)
{
    if (logLevel > verbosity)
        return;

   QString logLevelCode;

   switch (logLevel)
    {
        case LL_DEBUG   : logLevelCode = "DBG"; break;
        case LL_INFO    : logLevelCode = "INF"; break;
        case LL_WARNING : logLevelCode = "WRN"; break;
        case LL_ERROR   : logLevelCode = "ERR"; break;
        case LL_NONE    : return;
    }

    if (writeStdout)
    {
        printf ("[%s] <%s> %s: %s\n",
                QDateTime::currentDateTime().toString("dd.MM.yy@hh:mm:ss:zzz").toUtf8().data(),
                logLevelCode.toUtf8().data(),
                component.toUtf8().data(),
                message.toUtf8().data());
    }
        else
    {
       QTextStream(&logFile) << QString("[%1] <%2> %3: %4\n")
                                .arg(QDateTime::currentDateTime().toString("dd.MM.yy@hh:mm:ss:zzz"))
                                .arg(logLevelCode)
                                .arg(component)
                                .arg(message);

    }

}
