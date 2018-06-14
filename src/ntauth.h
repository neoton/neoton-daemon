#ifndef NTAUTH_H
#define NTAUTH_H

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <cstdlib>
#include <ctime>

class NTAuth : public QObject
{
    Q_OBJECT
public:
    explicit NTAuth(QObject *parent = 0);

    static int rangedRand (int min, int max);
    static QString sha256 (QString text);
    static QString randomString (int length = 64);

signals:

public slots:
};

#endif // NTAUTH_H
