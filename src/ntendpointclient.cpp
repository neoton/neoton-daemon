#include "ntendpointclient.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTEndpointClient::NTEndpointClient(QObject *parent)
{
    cType = CT_ENDPOINT;
    sEndpoint.id = 0;
}

void NTEndpointClient::setEndpoint(NTEndpoint ep)
{
    sEndpoint = ep;
}


NTEndpoint NTEndpointClient::endpoint()
{
    return sEndpoint;
}


void NTEndpointClient::onSocketMessage(QString message)
{
    log ("A message from Endpoint client: "+message);
    emit newCommandReceived(message);
}
