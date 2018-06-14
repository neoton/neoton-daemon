#ifndef NTENDPOINTCLIENT_H
#define NTENDPOINTCLIENT_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QString>
#include <QWebSocket>

#include "ntabstractclient.h"
#include "ntlog.h"
#include "ntdata.h"


class NTEndpointClient : public NTAbstractClient
{
     Q_OBJECT
public:
    explicit NTEndpointClient(QObject *parent = 0);

    NTEndpoint endpoint();
    void setEndpoint(NTEndpoint ep);

    ClientType getClientType();

private:
    NTEndpoint sEndpoint;

public slots:
    void onSocketMessage(QString message);

};

#endif // NTENDPOINTCLIENT_H
