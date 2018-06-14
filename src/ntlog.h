#ifndef NTLOG_H
#define NTLOG_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QDateTime>
#include <cstdio>

#include "ntdata.h"

class NTLog : public QObject
{
    Q_OBJECT
public:

    NTLog(const QString &file, LogLevel verbosity) :
        QObject(), file(file), verbosity(verbosity)
    {
        if (!file.isEmpty() && file != "stdout")
        {
            logFile.setFileName(file);
            if (!logFile.open(QIODevice::Append))
            {
                printf ("Could not open %s for logs, will write to stdout instead!\n", file.toUtf8().data());
                writeStdout = true;
                return;
            }
                else
            {
                printf ("All further log messages will be written to the log file %s.\n",
                        file.toUtf8().data());

                writeStdout = false;

                QTextStream(&logFile) << QString("[*] --- NEW LOG SECTION STARTED [%1] ---\n")
                                         .arg(QDateTime::currentDateTime().toString("dd.MM.yy@hh:mm:ss:zzz"));
            }
        }
        else
            writeStdout = true;
    }

    ~NTLog() {

        log ("Logging stopped.", LL_INFO, "logsv");

        if (!writeStdout)
            logFile.close();
    }


    static NTLog *instance;

    bool writeStdout;
    QFile logFile;

    void log(QString message, LogLevel logLevel, QString component);

private:
    QString file;
    LogLevel verbosity;

};


#endif // NTLOG_H
