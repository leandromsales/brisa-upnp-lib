#include "bcadevice.h"
#include "functions.h"

using namespace brisa::upnp;

BCADevice::BCADevice(QQmlApplicationEngine &engine, QByteArray dirPath) : Device(DEVICE_TYPE, DEVICE_FRIENDLY_NAME,
                                   DEVICE_MANUFACTURER, DEVICE_MANUFACTURER_URL,
                                   DEVICE_MODEL_DESCRIPTION, DEVICE_MODEL_NAME, DEVICE_MODEL_NUMBER,
                                   DEVICE_MODEL_URL, DEVICE_SERIAL_NUMBER, getCompleteUuid())
{
    m_appManager = new BRisaApplicationManager(engine, dirPath);
    connect(m_appManager,SIGNAL(listWasUpdated()),
            this,SLOT(onListOfAppsUpdated()));
    addAllWebFiles();
    engine.rootContext()->setContextProperty("manager", m_appManager);


    qDebug() << "+++++++++++++++++++++++++++++++++++++++++";
    qDebug() << QString::number(getAttribute(Port),16);
    qDebug() << "+++++++++++++++++++++++++++++++++++++++++";
    Functions *functions = new Functions(m_appManager,getAttribute(IpAddress)+":"+getAttribute(Port));
    functions->setDescriptionFile(":/src/functions.xml");

    this->addService(functions);
    this->start();

    this->listApps = this->getServiceByType("urn:schemas-upnp-org:service:Functions:1")->getVariable("ListApps");
    this->appInfo = this->getServiceByType("urn:schemas-upnp-org:service:Functions:1")->getVariable("AppInfo");
    this->app = this->getServiceByType("urn:schemas-upnp-org:service:Functions:1")->getVariable("App");
}

void BCADevice::onListOfAppsUpdated()
{
    addAllWebFiles();
}

void BCADevice::addAllWebFiles()
{
    foreach (QObject *o, m_appManager->get_apps()) {
        BRisaApplication *bApp = (BRisaApplication *) o;
        this->addFile(bApp->compePath(),"apps/" + bApp->get_title());
    }
}

QString BCADevice::parseHexInt(QString hex)
{
    int decNum = 0;
    for(int i=0;i<hex.length();i++) {

    }
}




