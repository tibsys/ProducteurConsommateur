#include <QApplication>
#include <QQmlApplicationEngine>
#include "ProducteurController.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    qmlRegisterType<ProducteurController>("ProducteurConsommateur", 1, 0, "Producteur");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;        

    return app.exec();
}
