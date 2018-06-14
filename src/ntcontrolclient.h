#ifndef NTCONTROLCLIENT_H
#define NTCONTROLCLIENT_H

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

class NTControlClient : public NTAbstractClient
{
     Q_OBJECT
public:
    NTControlClient();

    void setUserId (int uid);
    int userId();

private:
    int _userId;

public slots:
    void onSocketMessage(QString message);
};

#endif // NTCONTROLCLIENT_H
