#ifndef NTSTATIONPROCESS_H
#define NTSTATIONPROCESS_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "ntlog.h"

class NTStationProcess : public QObject
{
    Q_OBJECT
public:
    explicit NTStationProcess(int endpointId, QString appPath, QString liqFilePath, QObject *parent = 0);

    void start();
    void stop(bool forced = false);

    void setNeedsRespawn(bool need);
    bool isNeedToRespawn();

    int endpoint();

private:
    int endpointId;

    bool isRunning;
    bool iNeedToRespawn;

    QString appPath;
    QString liqFilePath;

    QProcess *stationProcess;

    void log (QString message, LogLevel level = LL_DEBUG);

private slots:
    void onProcessStart();
    void onProcessFinish(int exitCode);

signals:
    void processDead(int, int, bool);
    void processStarted();

public slots:
};

#endif // NTSTATIONPROCESS_H
