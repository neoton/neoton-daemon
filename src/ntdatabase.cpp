#include "ntdatabase.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTDatabase *NTDatabase::db = 0;

NTDatabase::NTDatabase(QString server, QString databaseName, QString username,
                       QString password, QObject *parent) : QObject(parent),
                       server(server), databaseName(databaseName), username(username),
                       password(password)
{
    log (QString("Initialized a database component, will connect to %1@%2").arg(databaseName).arg(server));
}

bool NTDatabase::start()
{
    database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName(server);
    database.setDatabaseName(databaseName);
    database.setUserName(username);
    database.setPassword(password);

    log ("Attempting to connect...", LL_DEBUG);

    bool ok = database.open();

    if (!ok)
    {
        log (QString("Could not connect to the database: %1").arg(database.lastError().text()), LL_ERROR);
    }
    else
    {
        log ("Successfully connected to the database! :3", LL_INFO);
    }

    return ok;
}

QList<NTEndpoint> NTDatabase::getEndpoints()
{
    log ("Getting all endpoints");
    QSqlQuery qsqGetEndpoints;
    qsqGetEndpoints.prepare("SELECT * from endpoint");

    if (!qsqGetEndpoints.exec())
    {
        log ("Could not execute this: "+qsqGetEndpoints.lastQuery(), LL_DEBUG);
        log ("Endpoint listing SQL error: "+qsqGetEndpoints.lastError().text(), LL_WARNING);
        return QList<NTEndpoint>();
    }
    else
    {
        log ("Building endpoint list");
        QList<NTEndpoint> epl;
        while (qsqGetEndpoints.next())
        {
            NTEndpoint ep;
            ep.id = qsqGetEndpoints.value("endpoint_id").toInt();
            ep.ip = qsqGetEndpoints.value("endpoint_ip").toString();
            ep.mount = qsqGetEndpoints.value("endpoint_mount").toString();
            ep.name = qsqGetEndpoints.value("endpoint_name").toString();
            ep.password = qsqGetEndpoints.value("endpoint_password").toString();
            ep.port = qsqGetEndpoints.value("endpoint_port").toInt();

            epl.append(ep);
        }

        log (QString("Built a list of %1 endpoint(s)").arg(epl.count()));
        return epl;
    }
}

QString NTDatabase::getNonceForAdmin(int userId)
{
    log (QString("Getting nonce for %1").arg(userId));
    QSqlQuery qsqGetAdminNonce;
    qsqGetAdminNonce.prepare("SELECT acct_nonce FROM account WHERE acct_id = ?");
    qsqGetAdminNonce.addBindValue(userId);

    if (!qsqGetAdminNonce.exec())
    {
        log ("Could not execute this: "+qsqGetAdminNonce.lastQuery(), LL_DEBUG);
        log ("WARNING! Could not get account nonce for user: "+qsqGetAdminNonce.lastError().text(), LL_WARNING);
        return QString();
    }
        else
    {
        if (!qsqGetAdminNonce.first())
            return QString();
        else
            return qsqGetAdminNonce.value("acct_nonce").toString();
    }
}

void NTDatabase::log(QString message, LogLevel level, QString component)
{
    NTLog::instance->log(message, level, component);
}
