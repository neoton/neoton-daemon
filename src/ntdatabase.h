#ifndef NTDATABASE_H
#define NTDATABASE_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "ntdata.h"
#include "ntlog.h"

class NTDatabase : public QObject
{
    Q_OBJECT
public:
    explicit NTDatabase(QString server, QString databaseName, QString username,
                        QString password, QObject *parent = 0);

    static NTDatabase *db;

    bool start();
    QList<NTEndpoint> getEndpoints();
    QString getNonceForAdmin (int userId);


private:
    QString server;
    QString databaseName;
    QString username;
    QString password;

    QSqlDatabase database;

    void log (QString message, LogLevel level = LL_DEBUG, QString component = "dbase");

signals:

public slots:
};

#endif // NTDATABASE_H
