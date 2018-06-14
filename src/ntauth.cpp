#include "ntauth.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

NTAuth::NTAuth(QObject *parent) : QObject(parent)
{
    srand (time(NULL));
}

int NTAuth::rangedRand(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

QString NTAuth::sha256(QString text)
{
    return QString(QCryptographicHash::hash(QByteArray(text.toUtf8()), QCryptographicHash::Sha256).toHex());
}

QString NTAuth::randomString(int length)
{
    QString alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890-_=+";
    int alphabetLen  = alphabet.length() - 1;

    QString res;

    for (int i = 0; i < length; i++)
    {
        res += alphabet.at(rangedRand(0, alphabetLen));
    }

    return res;
}
