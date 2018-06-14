#ifndef NTDATA_H
#define NTDATA_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QString>

enum LogLevel { // this is for internal logging
    LL_NONE,
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_DEBUG
};

struct NTEndpoint {
    int id;
    QString name;
    QString ip;
    int port;
    QString mount;
    QString password;
};

#endif // NTDATA_H
