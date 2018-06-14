#include "ntcontrolclient.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTControlClient::NTControlClient()
{
    cType = CT_CONTROL;
}

void NTControlClient::setUserId(int uid)
{
    _userId = uid;
}

int NTControlClient::userId()
{
    return _userId;
}

void NTControlClient::onSocketMessage(QString message)
{
    log ("A message from Control client: "+message);
    emit newCommandReceived(message);
}
