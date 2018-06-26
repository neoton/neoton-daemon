#include "ntstationprocess.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTStationProcess::NTStationProcess(int endpointId, QString appPath, QString liqFilePath, QObject *parent) : QObject(parent),
    endpointId(endpointId), appPath(appPath), liqFilePath(liqFilePath)
{
    log (QString("Created a new instance of station process handler, process image %1, config file %2").arg(appPath).arg(liqFilePath), LL_INFO);

    isRunning = false;
    iNeedToRespawn = false;

    QStringList args;
    args << liqFilePath;

    stationProcess = new QProcess(this);
    stationProcess->setProgram(appPath);
    stationProcess->setArguments(args);

    connect(stationProcess, SIGNAL(started()), this, SLOT(onProcessStart()));
    connect(stationProcess, SIGNAL(finished(int)), this, SLOT(onProcessFinish(int)));
}

void NTStationProcess::stop(bool forced)
{
    if (!isRunning)
    {
        log ("Nothing to stop, process is already dead.", LL_WARNING);
        return;
    }

    if (forced)
    {
        log ("Killing process");
        log (QString("Forcing to kill station process %1").arg(stationProcess->processId()), LL_WARNING);
        stationProcess->kill();
    }
        else
    {
        log (QString("Trying to stop station process %1").arg(stationProcess->processId()), LL_INFO);
        stationProcess->terminate();
    }
}

void NTStationProcess::setNeedsRespawn(bool need)
{
    iNeedToRespawn = need;
}

bool NTStationProcess::isNeedToRespawn()
{
    return iNeedToRespawn;
}

int NTStationProcess::endpoint()
{
    return endpointId;
}

void NTStationProcess::start()
{
    if (isRunning)
    {
        log ("Process is already running!", LL_WARNING);
        return;
    }

    log ("Starting the process...");
    stationProcess->start();
}

void NTStationProcess::log(QString message, LogLevel level)
{
    NTLog::instance->log(message, level, "sproc");
}

void NTStationProcess::onProcessStart()
{
    isRunning = true;
    log (QString("Process spawning succeeded, PID is %1").arg(stationProcess->processId()));
}

void NTStationProcess::onProcessFinish(int exitCode)
{
    if (!isRunning)
    {
        log ("WARNING: onProcessFinish() is emitted on a dead process!", LL_WARNING);
        return;
    }

    isRunning = false;
    switch (exitCode)
    {
        case 0 :
            log (QString("Process %1 exited normally, code 0, PID %1").arg(stationProcess->processId()));
            break;

        case 0xf291 : // Thanks Qt for this freaking magic number
            log (QString("Process %1 is killed internally").arg(stationProcess->processId()));
            break;

        default :
            log (QString("WARNING: Process %1 finished abnormally, exit code is %2")
                 .arg(stationProcess->processId()).arg(exitCode),
                 LL_WARNING);
            break;
    }

    emit processDead(endpointId, exitCode, iNeedToRespawn);
}
