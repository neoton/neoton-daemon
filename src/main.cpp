#include <QCoreApplication>
#include <QCommandLineParser>
#include <QString>
#include <QStringList>
#include <QObject>
#include "ntserver.h"

/*

    This is Neoton, a public broadcasting system
       (c) Asterleen ~ https://asterleen.com
    Licensed under BSD 3-Clause License, see LICENSE

*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << "c" << "config", "Configuration file", "config"));
    parser.process(a);

    NTServer neotond(parser.value("config"));

    int ret = a.exec();

    neotond.onServerExit();

    return ret;
}
