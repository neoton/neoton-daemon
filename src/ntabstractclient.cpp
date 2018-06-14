#include "ntabstractclient.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTAbstractClient::NTAbstractClient(QObject *parent) : QObject(parent)
{
    cType = CT_ABSTRACT;
}

void NTAbstractClient::setSocket(QWebSocket *s)
{
    sock = s;
    connect (sock, SIGNAL(textMessageReceived(QString)), this, SLOT(onSocketMessage(QString)));
    connect (sock, SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));
}

void NTAbstractClient::sendCommand(QString command)
{
    if (sock->isValid())
        sock->sendTextMessage(command);
}

void NTAbstractClient::setAuthorized(bool auth)
{
    isAuthorized = auth;
}

void NTAbstractClient::setChallengeNonce(QString nonce)
{
    chNonce = nonce;
}

bool NTAbstractClient::authorized()
{
    return isAuthorized;
}

QString NTAbstractClient::challengeNonce()
{
    return chNonce;
}

void NTAbstractClient::close()
{
    sock->close();
}

NTAbstractClient::ClientType NTAbstractClient::clientType()
{
    return cType;
}

void NTAbstractClient::log(QString message, LogLevel level)
{
    QString component;

    switch (cType)
    {
        case CT_ENDPOINT:
            component = "endpt";
            break;

        case CT_CONTROL:
            component = "cntrl";
        break;

        default:
            component = "aclnt";
            break;
    }
    NTLog::instance->log(message, level, component);
}

void NTAbstractClient::onSocketMessage(QString message)
{
    log (QString("New message in abstract socket: %1").arg(message), LL_WARNING);
}

void NTAbstractClient::onSocketDisconnect()
{
    log ("Socket disconnected.");
    emit disconnected();
}
