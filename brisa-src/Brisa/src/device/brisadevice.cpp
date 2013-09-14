#include <QtDebug>
#include <QIODevice>

#include "upnp/shared/webserver/webfile.h"

// TODO: put this include at the begin of the file
#include "brisadevice.h"
#include "upnp/shared/ssdp/brisassdpserver.h"
#include "upnp/shared/webserver/webserver.h"

using namespace Brisa;

static const QByteArray customWebservicesPath = "/custom/";

BrisaDevice::BrisaDevice(QObject *parent) :
        QObject(parent), running(false)
{

        this->discoverNetworkAddress();
        this->buildUrlBase();
        webserver = new Webserver(QHostAddress(ipAddress), port);
        ssdp = new BrisaSSDPServer();

        QObject::connect(ssdp,
                        SIGNAL(msearchRequestReceived(QString, QString, quint16)), this,
                        SLOT(respondMSearch(QString, QString, quint16)));
}

BrisaDevice::BrisaDevice(const QString &deviceType, const QString &friendlyName,
                         const QString &manufacturer, const QString &manufacturerURL,
                         const QString &modelDescription, const QString &modelName,
                         const QString &modelNumber, const QString &modelURL,
                         const QString &serialNumber, const QString &UDN, const QString &UPC,
                         const QString &presentationURL, QObject *parent) :
    QObject(parent),
    deviceType(deviceType),
    friendlyName(friendlyName),
    manufacturer(manufacturer),
    manufacturerUrl(manufacturerURL),
    modelDescription(modelDescription),
    modelName(modelName),
    modelNumber(modelNumber),
    modelUrl(modelURL),
    serialNumber(serialNumber),
    udn(UDN),
    upc(UPC),
    presentationUrl(presentationURL),
    server("UPnPlayer Webserver UPnP/1.0 " + modelName + " " + modelNumber),
    fileAddress(QString(friendlyName).remove(QChar(' ')).append(".xml")),
    running(false)
{
    this->discoverNetworkAddress();
    this->buildUrlBase();
    this->location = urlBase + "/" + fileAddress;

    this->major = "1";
    this->minor = "0";

    webserver = new Webserver(QHostAddress(ipAddress), port);
    qDebug() << "la vai: " << ipAddress;
    ssdp = new BrisaSSDPServer();

    QObject::connect(ssdp,
                     SIGNAL(msearchRequestReceived(QString, QString, quint16)),
                     this,
                     SLOT(respondMSearch(QString, QString, quint16)));
}

BrisaDevice::BrisaDevice(const BrisaDevice &dev) :
        QObject(dev.parent()),
        iconList(dev.getIconList()),
        serviceList(dev.getServiceList()),
        embeddedDeviceList(dev.getEmbeddedDeviceList()),
        urlBase(dev.getAttribute(BrisaDevice::UrlBase)),
        deviceType(dev.getAttribute(BrisaDevice::DeviceType)),
        friendlyName(dev.getAttribute(BrisaDevice::FriendlyName)),
        manufacturer(dev.getAttribute(BrisaDevice::Manufacturer)),
        manufacturerUrl(dev.getAttribute(BrisaDevice::ManufacturerUrl)),
        modelDescription(dev.getAttribute(BrisaDevice::ModelDescription)),
        modelName(dev.getAttribute(BrisaDevice::ModelName)),
        modelNumber(dev.getAttribute(BrisaDevice::ModelNumber)),
        modelUrl(dev.getAttribute(BrisaDevice::ModelUrl)),
        serialNumber(dev.getAttribute(BrisaDevice::SerialNumber)),
        udn(dev.getAttribute(BrisaDevice::Udn)),
        upc(dev.getAttribute(BrisaDevice::Upc)),
        presentationUrl(dev.getAttribute(BrisaDevice::PresentationUrl)),
        location(dev.getAttribute(BrisaDevice::Location)),
        server(dev.getAttribute(BrisaDevice::Server)),
        ipAddress(dev.getAttribute(BrisaDevice::IpAddress)),
        port(dev.getAttribute(BrisaDevice::Port).toInt()),
        fileAddress(dev.getAttribute(BrisaDevice::FileAddress)),
        running(dev.running)
{

    this->major = dev.getAttribute(BrisaDevice::Major);
    this->minor = dev.getAttribute(BrisaDevice::Minor);

    // TODO: this is wrong! this will create a new instance in the same ip and port.
    // It is necessary to generate a new port.
    webserver = new Webserver(QHostAddress(ipAddress), port);
    ssdp = new BrisaSSDPServer();
}

BrisaDevice &BrisaDevice::operator=(const BrisaDevice &dev) {
    if (this != &dev) {
        this->iconList = dev.getIconList();
        this->serviceList = dev.getServiceList();
        this->embeddedDeviceList = dev.getEmbeddedDeviceList();
        this->major = dev.getAttribute(BrisaDevice::Major);
        this->minor = dev.getAttribute(BrisaDevice::Minor);
        this->urlBase = dev.getAttribute(BrisaDevice::UrlBase);
        this->deviceType = dev.getAttribute(BrisaDevice::DeviceType);
        this->friendlyName = dev.getAttribute(BrisaDevice::FriendlyName);
        this->manufacturer = dev.getAttribute(BrisaDevice::Manufacturer);
        this->manufacturerUrl = dev.getAttribute(BrisaDevice::ManufacturerUrl);
        this->modelDescription = dev.getAttribute(BrisaDevice::ModelDescription);
        this->modelName = dev.getAttribute(BrisaDevice::ModelName);
        this->modelNumber = dev.getAttribute(BrisaDevice::ModelNumber);
        this->modelUrl = dev.getAttribute(BrisaDevice::ModelUrl);
        this->serialNumber = dev.getAttribute(BrisaDevice::SerialNumber);
        this->udn = dev.getAttribute(BrisaDevice::Udn);
        this->upc = dev.getAttribute(BrisaDevice::Upc);
        this->presentationUrl = dev.getAttribute(BrisaDevice::PresentationUrl);
        this->location = dev.getAttribute(BrisaDevice::Location);
        this->server = dev.getAttribute(BrisaDevice::Server);
        this->ipAddress = dev.getAttribute(BrisaDevice::IpAddress);
        this->port = dev.getAttribute(BrisaDevice::Port).toInt();
        this->fileAddress = dev.getAttribute(BrisaDevice::FileAddress);
        this->running = dev.running;
    }
    return *this;
}

BrisaDevice::~BrisaDevice()
{
    if (isRunning()) {
        stop();
    }

    foreach (BrisaIcon *icon, iconList) {
        icon->deleteLater();
    }
    iconList.clear();

    qDeleteAll(this->serviceList);
    serviceList.clear();
    qDeleteAll(this->embeddedDeviceList);
    embeddedDeviceList.clear();

    ssdp->deleteLater();

    delete webserver;
}

void BrisaDevice::xmlGenerator()
{
        BrisaDeviceXMLHandler handler;
        handler.xmlGenerator(this, &descriptionFile);
}

void BrisaDevice::setAttribute(xmlTags key, const QString &v)
{
        switch (key) {
        case Major:
                this->major = v;
                break;
        case Minor:
                this->minor = v;
                break;
        case UrlBase:
                this->urlBase = v;
                break;
        case DeviceType:
                this->deviceType = v;
                break;
        case FriendlyName:
                this->friendlyName = v;
                this->fileAddress = this->friendlyName.remove(QChar(' '));
                this->fileAddress.append(".xml");
                break;
        case Manufacturer:
                this->manufacturer = v;
                break;
        case ManufacturerUrl:
                this->manufacturerUrl = v;
                break;
        case ModelDescription:
                this->modelDescription = v;
                break;
        case ModelName:
                this->modelName = v;
                break;
        case ModelNumber:
                this->modelNumber = v;
                break;
        case ModelUrl:
                this->modelUrl = v;
                break;
        case SerialNumber:
                this->serialNumber = v;
                break;
        case Udn:
                this->udn = v;
                break;
        case Upc:
                this->upc = v;
                break;
        case PresentationUrl:
                this->presentationUrl = v;
                break;
        case FileAddress:
                this->fileAddress = v;
                break;
        case Location:
                this->location = v;
                break;
        case Server:
                this->server = v;
                break;
        case IpAddress:
                this->ipAddress = v;
                break;
        case Port:
                this->port = v.toInt();
                break;
        case Running:
                this->running = !this->running;
                break;
        }
}

QString BrisaDevice::getAttribute(xmlTags key) const
{
        switch (key) {
        case Major:
                return major;
                break;
        case Minor:
                return minor;
                break;
        case UrlBase:
                return urlBase;
                break;
        case DeviceType:
                return deviceType;
                break;
        case FriendlyName:
                return friendlyName;
                break;
        case Manufacturer:
                return manufacturer;
                break;
        case ManufacturerUrl:
                return manufacturerUrl;
                break;
        case ModelDescription:
                return modelDescription;
                break;
        case ModelName:
                return modelName;
                break;
        case ModelNumber:
                return modelNumber;
                break;
        case ModelUrl:
                return modelUrl;
                break;
        case SerialNumber:
                return serialNumber;
                break;
        case Udn:
                return udn;
                break;
        case Upc:
                return upc;
                break;
        case PresentationUrl:
                return presentationUrl;
                break;
        case FileAddress:
                return fileAddress;
                break;
        case Location:
                return location;
                break;
        case Server:
                return server;
                break;
        case IpAddress:
                return ipAddress;
                break;
        case Port:
                return QString(port);
                break;
        case Running:
                return (this->running) ? "TRUE" : "FALSE";
                break;
        default:
                return "";
                break;
        }
}

void BrisaDevice::addIcon(const QString &mimetype, const QString &width,
                const QString &height, const QString &depth, const QString &url)
{
        BrisaIcon *iconSwap = new BrisaIcon(mimetype, width, height, depth, url);
        iconList.append(iconSwap);
}

void BrisaDevice::addService(const QString &serviceType,
                const QString &serviceId, const QString &SCPDURL,
                const QString &controlURL, const QString &eventSubURL)
{
        BrisaService *serviceSwap = new BrisaService(serviceType, serviceId,
                        SCPDURL, controlURL, eventSubURL, this->urlBase);
    this->addService(serviceSwap);
}

void BrisaDevice::addService(BrisaService *servdev)
{
        serviceList.append(servdev);
    servdev->setUdn(this->udn);
}

void BrisaDevice::addEmbeddedDevice(const QString &deviceType,
                const QString &friendlyName, const QString &manufacturer,
                const QString &manufacturerURL, const QString &modelDescription,
                const QString &modelName, const QString &modelNumber,
                const QString &modelURL, const QString &serialNumber,
                const QString &UDN, const QString &UPC, const QString &presentationURL)
{
        BrisaDevice *deviceSwap = new BrisaDevice(deviceType, friendlyName,
                        manufacturer, manufacturerURL, modelDescription, modelName,
                        modelNumber, modelURL, serialNumber, UDN, UPC, presentationURL);
        embeddedDeviceList.append(deviceSwap);
}

void BrisaDevice::addEmbeddedDevice(BrisaDevice *newEmbeddedDevice)
{
        embeddedDeviceList.append(newEmbeddedDevice);
}

BrisaService *BrisaDevice::getServiceById(const QString &serviceId)
{
        foreach (BrisaService *s, this->serviceList)
                {
                        if (s->getAttribute(BrisaService::ServiceId) == serviceId)
                                return s;
                }

        return 0;
}

BrisaService *BrisaDevice::getServiceByType(const QString &serviceType)
{
        foreach (BrisaService *s, this->serviceList)
                {
                        if (s->getAttribute(BrisaService::ServiceType) == serviceType)
                                return s;
                }

        return 0;
}

void BrisaDevice::start()
{
        if (isRunning()) {
                qDebug() << "BrisaDevice already running!";
                return;
        }

        xmlGenerator();
        startWebServer();
        ssdp->start();
        doNotify();
        running = true;
}

int BrisaDevice::threadsNumber() const
{
    return webserver->threadsNumber();
}

void BrisaDevice::setThreadsNumber(int n)
{
    webserver->setThreadsNumber(n);
}

void BrisaDevice::stop()
{
    if (!isRunning()) {
        qDebug() << "BrisaDevice already stopped!";
        return;
    }

    doByeBye();
    ssdp->stop();
    running = false;
}

void BrisaDevice::startWebServer()
{
        buildWebServerTree();
        this->webserver->start();
}

void BrisaDevice::buildWebServerTree()
{
    descriptionFile.open();
    webserver->addService(fileAddress.toUtf8(),
                          new WebFile(descriptionFile.fileName(), this));
    descriptionFile.close();

    foreach(BrisaService *service, serviceList) {
        service->buildWebServiceTree(webserver);
    }
}

void BrisaDevice::respondMSearch(const QString &st, const QString &senderIp,
                quint16 senderPort)
{
        QString sDate = QDate::currentDate().toString("dd/MM/yyyy");

        if (st == "ssdp:all") {
                respondMSearchAll(senderIp, senderPort, sDate);
        } else if (st == "upnp:rootdevice") {
                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                server, "upnp:rootdevice", udn + "::upnp:rootdevice");
        } else if (st.contains("uuid:")) {

                QString embeddedUdn;
                QString embeddedType;

                if (st == udn)
                        ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                        server, "upnp:rootdevice", udn + "::upnp:rootdevice");

                for (int k = 0; k < embeddedDeviceList.size(); k++) {
                        embeddedUdn = embeddedDeviceList[k]->getAttribute(BrisaDevice::Udn);
                        embeddedType = embeddedDeviceList[k]->getAttribute(
                                        BrisaDevice::DeviceType);
                        if (st == embeddedUdn)
                                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate,
                                                location, server, embeddedUdn, embeddedUdn);
                }
        } else if (st.contains(":device:")) {

                QString embeddedUdn;
                QString embeddedType;

                if (st == deviceType)
                        ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                        server, "upnp:rootdevice", udn + "::upnp:rootdevice");

                for (int k = 0; k < embeddedDeviceList.size(); k++) {
                        embeddedUdn = embeddedDeviceList[k]->getAttribute(BrisaDevice::Udn);
                        embeddedType = embeddedDeviceList[k]->getAttribute(
                                        BrisaDevice::DeviceType);
                        if (st == embeddedType)
                                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate,
                                                location, server, embeddedUdn, embeddedUdn);
                }
        } else if (st.contains(":service:")) {

                QString serviceType;
                QString serviceUdn;

                for (int j = 0; j < serviceList.size(); j++) {
                        serviceType = serviceList[j]->getAttribute(
                                        BrisaService::ServiceType);
                        serviceUdn = udn + "::" + serviceType;
                        if (st == serviceType)
                                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate,
                                                location, server, serviceUdn, serviceType);
                }
        }
}

void BrisaDevice::respondMSearchAll(const QString &senderIp,
                quint16 senderPort, const QString &sDate)
{
        QString serviceType;
        QString serviceUdn;
        QString embeddedUdn;
        QString embeddedType;

        //MSearch response for root device
        ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location, server,
                        "upnp:rootdevice", udn + "::upnp:rootdevice");
        ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location, server,
                        udn, udn);
        ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location, server,
                        udn + "::" + deviceType, deviceType);

        //MSearch response for embedded devices
        for (int k = 0; k < embeddedDeviceList.size(); k++) {
                embeddedUdn = embeddedDeviceList[k]->getAttribute(BrisaDevice::Udn);
                embeddedType = embeddedDeviceList[k]->getAttribute(
                                BrisaDevice::DeviceType);
                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                server, embeddedUdn, embeddedUdn);
                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                server, embeddedUdn + "::" + embeddedType, embeddedType);
        }

        //MSearch response for services
        for (int j = 0; j < serviceList.size(); j++) {
                serviceType = serviceList[j]->getAttribute(BrisaService::ServiceType);
                serviceUdn = udn + "::" + serviceType;
                ssdp->respondMSearch(senderIp, senderPort, "1800", sDate, location,
                                server, serviceUdn, serviceType);
        }
}

QList<BrisaIcon*> BrisaDevice::getIconList() const
{
        return this->iconList;
}

QList<BrisaDevice*> BrisaDevice::getEmbeddedDeviceList() const
{
        return this->embeddedDeviceList;
}

QList<BrisaService*> BrisaDevice::getServiceList() const
{
        return this->serviceList;
}

void BrisaDevice::doByeBye()
{
        QString serviceType;
        QString serviceUdn;
        QString embeddedUdn;
        QString embeddedType;

        //Notify for root device
        ssdp->doByeBye(udn + "::upnp:rootdevice", "upnp:rootdevice");
        ssdp->doByeBye(udn, udn);
        ssdp->doByeBye(udn + "::" + deviceType, deviceType);

        //Notify for embedded devices
        foreach(BrisaDevice* embeddedDevice, embeddedDeviceList)
                {
                        embeddedUdn = embeddedDevice->getAttribute(BrisaDevice::Udn);
                        embeddedType
                                        = embeddedDevice->getAttribute(BrisaDevice::DeviceType);
                        ssdp->doByeBye(embeddedUdn, embeddedUdn);
                        ssdp->doByeBye(embeddedUdn + "::" + embeddedType, embeddedType);
                }

        //Notify for services
        for (int j = 0; j < serviceList.size(); j++) {
                serviceType = serviceList[j]->getAttribute(BrisaService::ServiceType);
                serviceUdn = udn + "::" + serviceType;
                ssdp->doByeBye(serviceUdn, serviceType);
        }
}

void BrisaDevice::doNotify()
{
        QString serviceType;
        QString serviceUdn;
        QString embeddedUdn;
        QString embeddedType;

        //Notify for root device
        ssdp->doNotify(udn + "::upnp:rootdevice", location, "upnp:rootdevice",
                        server, "1800");
        ssdp->doNotify(udn, location, udn, server, "1800");
        ssdp->doNotify(udn + "::" + deviceType, location, deviceType, server,
                        "1800");

        //Notify for embedded devices
        foreach(BrisaDevice* embeddedDevice, embeddedDeviceList) {
                embeddedUdn = embeddedDevice->getAttribute(BrisaDevice::Udn);
                embeddedType = embeddedDevice->getAttribute(
                                BrisaDevice::DeviceType);
                ssdp->doNotify(embeddedUdn, location, embeddedUdn, server, "1800");
                ssdp->doNotify(embeddedUdn + "::" + embeddedType, location,
                                embeddedType, server, "1800");
        }

        //Notify for services
        for (int j = 0; j < serviceList.size(); j++) {
                serviceType = serviceList[j]->getAttribute(BrisaService::ServiceType);
                serviceUdn = udn + "::" + serviceType;
                ssdp->doNotify(serviceUdn, location, serviceType, server, "1800");
        }
}

bool BrisaDevice::isRunning()
{
        return this->running;
}

void BrisaDevice::clear()
{
        iconList.clear();
        serviceList.clear();
        embeddedDeviceList.clear();

        this->major.clear();
        this->minor.clear();
        this->urlBase.clear();
        this->deviceType.clear();
        this->friendlyName.clear();
        this->manufacturer.clear();
        this->manufacturerUrl.clear();
        this->modelDescription.clear();
        this->modelName.clear();
        this->modelNumber.clear();
        this->modelUrl.clear();
        this->serialNumber.clear();
        this->udn.clear();
        this->upc.clear();
        this->presentationUrl.clear();

        this->location.clear();
        this->server.clear();
        this->ipAddress.clear();

        this->attribute.clear();
        this->fileAddress.clear();

        port = 0;
}

void BrisaDevice::buildUrlBase()
{
        QString sPort;
        sPort.setNum(this->port);
        this->urlBase = "http://" + ipAddress + ":" + sPort;
}

void BrisaDevice::discoverNetworkAddress()
{
    this->port = getPort();
    this->ipAddress = getValidIP();

    //qDebug() << "Brisa Device: Acquired Address " << this->ipAddress-> << ":"
      //<< this->port;

    qDebug() << "Brisa Device: Acquired Address " << this->ipAddress + ":" +
                QString::number(this->port);
}

QByteArray BrisaDevice::addWebservice(QByteArray pathSuffix,
                                      WebService *service)
{
    if (pathSuffix.startsWith('/'))
        pathSuffix.remove(0, 1);
    QByteArray path = customWebservicesPath + pathSuffix;
    webserver->addService(path, service);
    return path;
}

void BrisaDevice::removeWebservice(const QByteArray &path)
{
    if (path.startsWith(customWebservicesPath)) {
        webserver->removeService(path);
    }
}