#include "ntserver.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTServer::NTServer(QString config, QObject *parent) : QObject(parent), config(config)
{
    if (!QFile::exists(config))
    {
        if (config.isEmpty())
            config = "<none>";

        printf ("Configuration file %s does not exist. Neoton can't operate without a config file. Exiting.\n",
                config.toUtf8().data());
        exit(-1);
    }

    loadConfig(config);

    NTLog::instance = new NTLog(logFile, (LogLevel)outputLogLevel);
    log ("Welcome to Neoton Control Server!", LL_INFO);
    log (QString("Neoton Server Version is %1").arg(NEOTON_VERSION));


    ///
    /// WARNING! WARNING! WARNING!
    /// THIS PART IS EXTREMELY DESTRUCTIVE!
    /// IT PERFORMS A GENOCIDE OF PROCESSES THAT WERE RAN BEFORE
    /// PLEASE DO NOT USE NEOTON WITH OTHER SERVICES THAT USE
    /// THE SAME PROCESS NAME AS SPECIFIED IN `liquidsoap_path` CONFIG VARIABLE!
    /// OTHERWISE IT WILL CAUSE DATA LOSS AND NUCLEAR EXPLOSION!
    ///
    /// YOU WERE WARNED!
    ///

    QString processImageName = QFileInfo(stationAppPath).fileName();

    #ifdef __linux__
        log ("A Linux platform detected, cleaning up old processes if exist...", LL_WARNING);
        system(QString("killall -9 %1").arg(processImageName).toUtf8().data());
    #elif _WIN32
        log ("A Windows platform detected, cleaning up old processes if exist...", LL_WARNING);
        system(QString("taskkill /IM %1 /F /T").arg(processImageName).toUtf8().data());
    #else
        log ("The platform Neoton runs in is unknown, process cleaning is your business.", LL_WARNING);
    #endif

    // (or should we use pidfiles instead?..)

    log ("Setting up the database connection...", LL_INFO);
    NTDatabase::db = new NTDatabase(dbServer, dbDatabase, dbUsername, dbPassword);
    if (NTDatabase::db->start())
        log ("Database connection established successfully.");
    else
    {
        log ("Could not set up a database connection!", LL_ERROR);
        exit(2);
    }

    log (QString("Server will listen on %1 port.").arg(serverPort), LL_INFO);
    log ("Creating server instance...");

    server = new QWebSocketServer(QString("Neoton/%1").arg(NEOTON_VERSION),
                                  QWebSocketServer::NonSecureMode,
                                  this);

    log ("Starting the server");
    if (!server->listen(QHostAddress::Any, serverPort))
    {
        log (QString("Could not start the server! Check if the port %1 isn't taken by another app or another Neoton instance.")
                     .arg(serverPort), LL_ERROR);
        exit(1);
    }

    connect (server, SIGNAL(newConnection()), this, SLOT(onNewClientConnection()));
    log (QString("Server is listening on port %1").arg(serverPort), LL_INFO);

    log ("Loading endpoints info from database...");
    if (loadEndpointsFromDatabase())
    {
        log ("Loaded successfully, creating Liquidsoap station instances...");
        createStationInstances();
    }
    else
        log ("Could not load endpoint information!", LL_WARNING);
}

NTServer::~NTServer()
{

}

NTEndpoint NTServer::endpointById(int id)
{
    for (int i = 0; i < dbEndpoints.count(); i++)
    {
        if (dbEndpoints.at(i).id == id)
            return dbEndpoints.at(i);
    }

    NTEndpoint stub;
    stub.id = -1;
    return stub;
}

NTEndpoint NTServer::endpointById(QString id, NTAbstractClient *client)
{
    bool valueCorrect;
    int endpointId = id.toInt(&valueCorrect);

    NTEndpoint ep;
    ep.id = -1;

    if (!valueCorrect || endpointId <= 0)
    {
        if (client != NULL)
        {
            client->sendCommand("ERROR 999 #Bad endpoint ID, bye.");
            client->close();
        }

        return ep;
    }

    ep = endpointById(endpointId);
    if (ep.id == -1)
    {
        if (client != NULL)
        {
            client->sendCommand("ERROR 999 #Bad endpoint ID, bye.");
            client->close();
        }
    }

    return ep;
}

NTStationProcess *NTServer::getEspFor(int endpointId)
{
    for (int i = 0; i < endpointStationProcesses.count(); i++)
    {
        if (endpointStationProcesses.at(i)->endpoint() == endpointId)
            return endpointStationProcesses.at(i);
    }

    return NULL;
}

bool NTServer::createEspFor(int endpointId)
{
    if (getEspFor(endpointId) != NULL)
    {
        log (QString("An endpoint station process already exists for %1; stop it first").arg(endpointId));
        return false;
    }

    log (QString("Creating a new endpoint station process instance for endpoint #%1").arg(endpointId));

    NTStationProcess *ntsp = new NTStationProcess(endpointId, stationAppPath,
                                                  QString("%1/endpoint_%2.liq").arg(stationAppConfigDir)
                                                  .arg(endpointId));

    connect(ntsp, SIGNAL(processDead(int, bool)), this, SLOT(onEndpointProcessDeath(int, bool)));
    connect(ntsp, SIGNAL(processStarted()), this, SLOT(onEndpointProcessStart()));
    ntsp->start();

    endpointStationProcesses.append(ntsp);

    return true;
}

bool NTServer::stopEspFor(int endpointId, bool forced)
{
    for (int i = 0; i < endpointStationProcesses.count(); i++)
    {
        if (endpointStationProcesses.at(i)->endpoint() == endpointId)
        {
            endpointStationProcesses.at(i)->stop(forced);
            return true;
        }
    }

    return false;
}

void NTServer::restartEspFor(int endpointId)
{
    NTStationProcess *ntsp = getEspFor(endpointId);
    NTEndpointClient *ep = getEndpointClientFor(endpointId);

    if (ep != NULL)
    {
        ep->sendCommand("STOP"); // to make it reconnect to the audio stream later
    }

    ntsp->setNeedsRespawn(true);
    ntsp->stop();
}

void NTServer::killAllEsp(bool forRestart)
{
    for (int i = 0; i < endpointStationProcesses.count(); i++)
    {
        endpointStationProcesses.at(i)->setNeedsRespawn(forRestart);
        endpointStationProcesses.at(i)->stop(true);
    }
}

NTEndpointClient *NTServer::getEndpointClientFor(int endpointId)
{
    for (int i = 0; i < endpoints.count(); i++)
    {
        if (endpoints.at(i)->endpoint().id == endpointId)
            return endpoints.at(i);
    }

    return NULL;
}

void NTServer::log(QString message, LogLevel logLevel, QString component)
{
    NTLog::instance->log(message, logLevel, component);
}

void NTServer::loadConfig(QString configFile)
{
    QSettings settings(configFile, QSettings::IniFormat);

    settings.beginGroup("server");
    outputLogLevel = (LogLevel)settings.value("log_level", LL_DEBUG).toInt();
    if (outputLogLevel > 4)
        outputLogLevel = LL_DEBUG;

    logFile = settings.value("log_file", "stdout").toString();
    serverPort = settings.value("server_port", 1337).toInt();
    settings.endGroup();

    settings.beginGroup("environment");
    stationAppPath = settings.value("liquidsoap_path", "").toString();
    stationAppConfigDir = settings.value("liquidsoap_config_dir", "/etc/liquidsoap").toString();
    respawnProcessesOnDeath = settings.value("respawn", false).toBool();
    respawnOnlyOnBadDeath = settings.value("respawn_on_crash", false).toBool();
    settings.endGroup();

    settings.beginGroup("database");
    dbServer = settings.value("server", "localhost").toString();
    dbDatabase = settings.value("database", "neoton").toString();
    dbUsername = settings.value("username", "neoton").toString();
    dbPassword = settings.value("password", "AWW_YISS").toString();
    settings.endGroup();
}

bool NTServer::loadEndpointsFromDatabase()
{
    dbEndpoints = NTDatabase::db->getEndpoints();

    // This will actualize endpoint settings (mount, port, password, etc)
    // for existing endpoint connections
    for (int i = 0; i < dbEndpoints.count(); i++)
    {
        NTEndpoint ep = dbEndpoints.at(i);
        NTEndpointClient *epc = getEndpointClientFor(ep.id);

        if (epc != NULL)
            epc->setEndpoint(ep);
    }

    return dbEndpoints.count() > 0;
}

void NTServer::createStationInstances()
{
    for (int i = 0; i < dbEndpoints.count(); i++)
    {
        createEspFor(dbEndpoints.at(i).id);
    }
}

// This method is called when the list of endpoints
// has been reloaded from database.
// It checks the match between existing instances and endpoints
// and runs/stops the endpoint station processes according to
// endpoint list.
/// This method is very expensive, please don't call it frequently.
void NTServer::correctStationInstances()
{
    for (int i = 0; i < dbEndpoints.count(); i++)
    {
        if (getEspFor(dbEndpoints.at(i).id) == NULL)
           createEspFor(dbEndpoints.at(i).id);
    }

    for (int i = 0; i < endpointStationProcesses.count(); i++)
    {
        if (endpointById(endpointStationProcesses.at(i)->endpoint()).id == -1)
            endpointStationProcesses.at(i)->stop(true);
    }
}

void NTServer::onNewClientConnection()
{
    while (server->hasPendingConnections())
    {
        log ("A new client connected", LL_INFO);
        log ("A new client connected, setting its default state to Generic. Further commands will determine its state.");
        QWebSocket *sock = server->nextPendingConnection();
        connect (sock, SIGNAL(textMessageReceived(QString)), this, SLOT(onGenericClientCommand(QString)));
    }
}

void NTServer::onAbstractClientDisconnect()
{
    log ("Abstract client disconnected, ignoring");
    NTAbstractClient *client = (NTAbstractClient *)QObject::sender();
    client->deleteLater();
}

void NTServer::onControlClientDisconnect()
{
    NTControlClient *client = (NTControlClient *)QObject::sender();;

    log ("Control client disconnected, removing it from the list");
    controls.removeAt(controls.indexOf(client));

    client->deleteLater();
}

void NTServer::onEndpointClientDisconnect()
{
     NTEndpointClient *client = (NTEndpointClient *)QObject::sender();;

    log (QString("Endpoint client #%1 disconnected! Is it okay?").arg(client->endpoint().id), LL_WARNING);
    endpoints.removeAt(endpoints.indexOf(client));

    client->deleteLater();
}

void NTServer::onGenericClientCommand(QString message)
{
    QWebSocket *sock = (QWebSocket *)QObject::sender();

    if (message == "ENDPOINT")
    {
        log ("A new endpoint client came here!");

        disconnect(sock, SIGNAL(textMessageReceived(QString)), this, SLOT(onGenericClientCommand(QString)));
        NTEndpointClient *client = new NTEndpointClient();
        client->setSocket(sock);
        connect(client, SIGNAL(newCommandReceived(QString)), this, SLOT(onEndpointCommand(QString)));
        connect(client, SIGNAL(disconnected()), this, SLOT(onEndpointClientDisconnect()));

        QString nonce = NTAuth::randomString();
        client->setChallengeNonce(nonce);

        endpoints.append(client);

        client->sendCommand(QString("AUTH %1").arg(nonce));
    }
    else if (message == "CONTROL")
    {
        log ("A new control client came here!");

        disconnect(sock, SIGNAL(textMessageReceived(QString)), this, SLOT(onGenericClientCommand(QString)));
        NTControlClient *client = new NTControlClient();
        client->setSocket(sock);
        connect(client, SIGNAL(newCommandReceived(QString)), this, SLOT(onControlCommand(QString)));
        connect(client, SIGNAL(disconnected()), this, SLOT(onControlClientDisconnect()));

        QString nonce = NTAuth::randomString();
        client->setChallengeNonce(nonce);

        controls.append(client);

        client->sendCommand(QString("AUTH %1").arg(nonce));
    }
        else
    {
        sock->sendTextMessage("ERROR 999 #Bad initialization command, ENDPOINT or CONTROL are accepted.");
        sock->close();
        sock->deleteLater();
    }
}

void NTServer::onEndpointCommand(QString message)
{
    NTEndpointClient *client = (NTEndpointClient *)QObject::sender();

    log ("Endpoint command: "+message);
    QStringList commands = message.split(" ", QString::SkipEmptyParts);

    if(commands.count() == 0)
        return;

    if (commands[0] == "AUTH")
    {
        if (commands.length() < 3)
        {
            client->sendCommand("ERROR 999 #Bad syntax or some parameters are missing.");
            client->close();
        }

        NTEndpoint ep = endpointById(commands[1], (NTAbstractClient *)client);
        if (ep.id == -1)
            return;

        if (getEndpointClientFor(ep.id) != NULL)
        {
            client->sendCommand("ERROR 200 #Some client is already connected as endpoint #"+QString::number(ep.id));
            client->close();
            return;
        }

        client->setEndpoint(ep);

        QString challenge = NTAuth::sha256(client->challengeNonce()+
                                           NTAuth::sha256(ep.password));
        if (commands[2] != challenge)
        {
            log (QString ("Auth mismatch, our side is %1, client's is %2").arg(challenge).arg(commands[2]));
            client->sendCommand("AUTH ERROR #Incorrect password, check your configuration.");
            client->close();
        }
        else
        {
            log (QString("Endpoint client #%1 passed the auth process, sending parameters").arg(client->endpoint().id));
            client->setAuthorized(true);
            client->sendCommand("AUTH OK");
            client->sendCommand(QString("STREAM %1 %2").arg(client->endpoint().mount).arg(client->endpoint().port));
            client->sendCommand("PLAY");
        }

        return;
    }
}

void NTServer::onControlCommand(QString message)
{
    NTControlClient *client = (NTControlClient *)QObject::sender();

    log ("Wow, a Control command: "+message);

    QStringList commands = message.split(" ", QString::SkipEmptyParts);

    if (commands[0] == "AUTH")
    {
        if (commands.length() < 3)
        {
            client->sendCommand("ERROR 999 #Bad syntax or some parameters missing.");
            client->close();
        }

        bool valueCorrect;
        int userId = commands[1].toInt(&valueCorrect);

        if (!valueCorrect || userId <= 0)
        {
            client->sendCommand("ERROR 999 #Bad user ID, bye.");
            client->close();
            return;
        }

        client->setUserId(userId);

        QString nonce = NTDatabase::db->getNonceForAdmin(userId);
        if (nonce.isEmpty())
        {
            client->sendCommand("ERROR 999 #Bad user ID or user is not authorized, bye.");
            client->close();
            return;
        }


        QString challenge = NTAuth::sha256(client->challengeNonce()+
                                           NTAuth::sha256(nonce));
        if (commands[2] != challenge)
        {
            log (QString ("Auth mismatch, our side is %1, client's is %2").arg(challenge).arg(commands[2]));
            client->sendCommand("AUTH ERROR #Incorrect nonce, probably your session is expired");
            client->close();
        }
            else
        {
            log (QString("Control client #%1 passed the auth process").arg(client->userId()));
            client->setAuthorized(true);
            client->sendCommand("AUTH OK");
        }

        return;
    }

    if (commands[0] == "RESTART")
    {
        if (commands.length() < 2)
        {
            client->sendCommand("ERROR 999 #Bad syntax or some parameters are missing.");
            client->close();
        }

        if (commands[1] == "*")
        {
            killAllEsp(true);
            client->sendCommand("RESTART OK #Restarted all the running instances");
            return;
        }

        NTEndpoint ep = endpointById(commands[1]);
        if (ep.id == -1)
        {
            client->sendCommand("RESTART ERROR #No such endpoint, probably needs to reload endpoint list.");
            return;
        }

        restartEspFor(ep.id);
        client->sendCommand(QString("RESTART OK #Restarted instance for endpoint #%1").arg(ep.id));

        return;
    }

    if (commands[0] == "RELOAD")
    {
        if (loadEndpointsFromDatabase())
        {
            correctStationInstances(); // probably needs to be configured from admin panel
            client->sendCommand("RELOAD OK");
        }
        else
            client->sendCommand("RELOAD ERROR #No endpoints loaded, probably there are no endpoints in the database");

        return;
    }
}

void NTServer::onEndpointProcessStart()
{
    NTStationProcess *ntsp = (NTStationProcess *)QObject::sender();
    NTEndpointClient *ep = getEndpointClientFor(ntsp->endpoint());


    if (ep != NULL)
    {
        // actualize settings for the endpoint
        ep->sendCommand(QString("STREAM %1 %2").arg(ep->endpoint().mount).arg(ep->endpoint().port));
        ep->sendCommand("PLAY"); // resume playing
    }
}

void NTServer::onEndpointProcessDeath(int exitCode, bool needsToRespawn)
{
    NTStationProcess *ntsp = (NTStationProcess *)QObject::sender();

    int endpointId = ntsp->endpoint();

    log (QString ("Ow, a process of endpoint #%1 has dead with code %2...").arg(endpointId).arg(exitCode), LL_WARNING);

    endpointStationProcesses.removeAt(endpointStationProcesses.indexOf(ntsp));
    ntsp->deleteLater();

    if (needsToRespawn)
    {
        log ("Oh, this little ESP needs to respawn ASAP!");
        createEspFor(endpointId);
    }
        else
    if (respawnProcessesOnDeath && exitCode != 0xf291) // 0xf291 is a Qt's magic number that indicates internally killed processes
    {
        if (respawnOnlyOnBadDeath)
        {
            if (exitCode == 0)
                log ("We need to respawn only crashed processes.");
            else
                createEspFor(endpointId);
        }
        else
            createEspFor(endpointId);
    }
    else
        log ("Good night, sweet process.");
}

void NTServer::onServerExit()
{
    log ("AW SHISH! Exiting!");
    killAllEsp();
}
