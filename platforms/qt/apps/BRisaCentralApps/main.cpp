#include <QQmlApplicationEngine>
#include <QApplication>
#include <QQmlContext>

#include "bcajson.h"
#include "bcadevice.h"
#include "FolderCompressor.h"

#ifdef QT_WEBVIEW_WEBENGINE_BACKEND
#include <QtWebEngine>
#endif // QT_WEBVIEW_WEBENGINE_BACKEND

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    #ifdef QT_WEBVIEW_WEBENGINE_BACKEND
        QtWebEngine::initialize();
    #endif // QT_WEBVIEW_WEBENGINE_BACKEND

    QQmlApplicationEngine engine;

    QQmlContext *ctxt = engine.rootContext();

    BRisaApplicationManager *manager = new BRisaApplicationManager(engine);

    QDir dir("../BRisaCentralApps/apps");
    QStringList listApps = dir.entryList();

    for(int i = 2; i < listApps.size(); i++) {

        BCAJson json(dir.absoluteFilePath(listApps[i]) + "/description.json");
        QString icon = "file:///" + dir.absoluteFilePath(listApps[i]) + "/icon.png";

        FolderCompressor compressor;
        qDebug() << "Compressão : " << compressor.compressFolder(dir.absoluteFilePath(listApps[i]), dir.absoluteFilePath(listApps[i]) + "/" + listApps[i] + ".compe");

        QString url = "file:///" + dir.absoluteFilePath(listApps[i]);

        manager->addApp(new BRisaApplication(icon, listApps[i], url, json.toBRisaApp()));
    }

    BCADevice *bca = new BCADevice(manager);
    bca->printAllApps();

    ctxt->setContextProperty("manager", manager);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
