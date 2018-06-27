#ifndef NTSERVER_H
#define NTSERVER_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QFileInfo>
#include <QSettings>
#include <QFile>
#include <QList>


#include "ntlog.h"
#include "ntdata.h"
#include "ntauth.h"
#include "ntabstractclient.h"
#include "ntendpointclient.h"
#include "ntcontrolclient.h"
#include "ntstationprocess.h"
#include "ntdatabase.h"

#define NEOTON_VERSION "0.2.1"

class NTServer : public QObject
{
    Q_OBJECT
public:
    explicit NTServer(QString config, QObject *parent = 0);
    ~NTServer();

private:
    QString config;
    LogLevel outputLogLevel;
    QString logFile;

    QString stationAppPath;
    QString stationAppConfigDir;
    bool respawnProcessesOnDeath;
    bool respawnOnlyOnBadDeath;

    QString dbServer;
    QString dbDatabase;
    QString dbUsername;
    QString dbPassword;

    QWebSocketServer *server;
    int serverPort;

    QList<NTEndpointClient *> endpoints;
    QList<NTControlClient *> controls;
    QList<NTEndpoint> dbEndpoints;
    QList<NTStationProcess *> endpointStationProcesses;

    /// Endpoints Information management
    NTEndpoint endpointById(int id);
    NTEndpoint endpointById(QString id, NTAbstractClient *client = NULL);

    /// Endpoint Station Processes management
    NTStationProcess *getEspFor(int endpointId);
    bool createEspFor(int endpointId);
    bool stopEspFor(int endpointId, bool forced = false);
    void restartEspFor(int endpointId);
    void killAllEsp(bool forRestart = false);

    /// Endpoint Clients management
    NTEndpointClient *getEndpointClientFor(int endpointId);

    void createStationInstances();
    void correctStationInstances();

    void log(QString message, LogLevel logLevel = LL_DEBUG, QString component = "ncore");
    void loadConfig(QString configFile);
    bool loadEndpointsFromDatabase();


signals:

private slots:
    void onNewClientConnection();
    void onAbstractClientDisconnect();
    void onControlClientDisconnect();
    void onEndpointClientDisconnect();

    void onGenericClientCommand(QString message);
    void onEndpointCommand(QString message);
    void onControlCommand(QString message);

    void onEndpointProcessStart();
    void onEndpointProcessDeath(int exitCode, bool needsToRespawn);

public slots:
    void onServerExit();

};

#endif // NTSERVER_H
