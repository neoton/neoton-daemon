#ifndef NTABSTRACTCLIENT_H
#define NTABSTRACTCLIENT_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QString>
#include <QWebSocket>

#include "ntlog.h"
#include "ntdata.h"

class NTAbstractClient : public QObject
{
    Q_OBJECT
public:

    enum ClientType {
        CT_ABSTRACT,
        CT_ENDPOINT,
        CT_CONTROL
    };

    explicit NTAbstractClient(QObject *parent = 0);

    void setSocket(QWebSocket *s);
    void sendCommand (QString command);
    void setAuthorized (bool auth);
    void setChallengeNonce (QString nonce);

    bool authorized();
    QString challengeNonce();

    void close();

    ClientType clientType();

private:


protected:
    QWebSocket *sock;
    ClientType cType;

    bool isAuthorized;
    QString chNonce;
    void log (QString message, LogLevel level = LL_DEBUG);

signals:

private slots:
    void onSocketMessage (QString message);
    void onSocketDisconnect();

public slots:

signals:
    void newCommandReceived(QString);
    void disconnected();
};

#endif // NTABSTRACTCLIENT_H
