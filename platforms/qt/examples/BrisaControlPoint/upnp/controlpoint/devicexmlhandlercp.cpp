/*
 * Universidade Federal de Campina Grande
 * Centro de Engenharia Elétrica e Informática
 * Laboratório de Sistemas Embarcados e Computação Pervasiva
 * BRisa Project / BRisa-Qt - http://brisa.garage.maemo.org
 * Filename: brisadevicexmlhandlercp.cpp
 * Created:
 * Description: Implementation of BrisaDeviceParserContext, BrisaDeviceXMLHandlerCP and
 * ServiceFetcher classes.
 * Authors: Name <email> @since 2009
 *
 *
 * Copyright (C) <2009> <Embbeded Systems and Pervasive Computing Laboratory>
 *
 * BRisa-Qt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "devicexmlhandlercp.h"
#include "cpdevice.h"

#define PORT_INDEX 2

namespace brisa {
namespace upnp {
namespace controlpoint {

DeviceXMLHandlerCP::~DeviceXMLHandlerCP() {
}

void DeviceXMLHandlerCP::parseDevice(Device *device,
        QTemporaryFile *tmp)
{
    QDomDocument document("Device");
    document.setContent(tmp);
    QDomElement element = document.documentElement();
    if (element.tagName() != "root")
        return;
    QDomNode n = element.firstChild();
    while (!n.isNull()) {
        element = n.toElement();
        if (element.tagName() == "specVersion") {
            QString major = element.elementsByTagName("major").at(0).toElement().text();
            QString minor = element.elementsByTagName("minor").at(0).toElement().text();
            if (major.isEmpty() || minor.isEmpty())
                return;
            device->setAttribute(Device::Major, major);
            device->setAttribute(Device::Minor, minor);
        } else if (element.tagName() == "device") {
            parseDevice(device, element);
        }
        n = n.nextSibling();
    }
}

void DeviceXMLHandlerCP::parseDevice(Device *device, QDomElement &element)
{
    QString udn = element.elementsByTagName("UDN").at(0).toElement().text();

    //qDebug() << "\n<UDN_XML>\n\t" << udn <<  "\n</UDN_XML>\n ";

    QString friendlyName = element.elementsByTagName("friendlyName").at(0).toElement().text();
    QString deviceType = element.elementsByTagName("deviceType").at(0).toElement().text();
    QString manufacturer = element.elementsByTagName("manufacturer").at(0).toElement().text();
    QString manufacturerURL = element.elementsByTagName("manufacturerURL").at(0).toElement().text();
    QString modelName = element.elementsByTagName("manufacturer").at(0).toElement().text();
    QString modelNumber = element.elementsByTagName("modelNumber").at(0).toElement().text();
    QString modelDescription= element.elementsByTagName("modelDescription").at(0).toElement().text();
    QString modelURL = element.elementsByTagName("modelURL").at(0).toElement().text();
    QString serialNumber = element.elementsByTagName("serialNumber").at(0).toElement().text();
    device->setAttribute(Device::Udn, udn);
    device->setAttribute(Device::FriendlyName, friendlyName);
    device->setAttribute(Device::DeviceType, deviceType);
    device->setAttribute(Device::Manufacturer, manufacturer);
    device->setAttribute(Device::ManufacturerUrl, manufacturerURL);
    device->setAttribute(Device::ModelName, modelName);
    device->setAttribute(Device::ModelNumber, modelNumber);
    device->setAttribute(Device::ModelDescription, modelDescription);
    device->setAttribute(Device::ModelUrl, modelURL);
    device->setAttribute(Device::SerialNumber, serialNumber);

    QDomNodeList serviceList = element.elementsByTagName("serviceList");
    if (serviceList.size() > 0) {
        QDomNodeList services = serviceList.at(0).toElement().elementsByTagName("service");
        for (int i = 0; i < services.size(); i++) {

            QDomElement serviceElement = services.at(i).toElement();
            Service *service = parseService(serviceElement);

            validateURLBase(device);

            //Fixes location error: "http://192.168.1.1:1900/http://192.168.1.1:1900/wfc.xml" and "/ipc.xml"
            QString location("");
            QUrl scpdUrl = service->getAttribute(Service::ScpdUrl);

            if (scpdUrl.isValid() && !scpdUrl.isRelative())
                location = scpdUrl.toString();
            else if (scpdUrl.isValid() && scpdUrl.isRelative()) {
                if (device->getAttribute(Device::UrlBase).endsWith("/") == true && scpdUrl.toString().startsWith("/") == true) {
                    QString temp = scpdUrl.toString();
                    location = device->getAttribute(Device::UrlBase) + temp.remove(0, 1);
                } else if (device->getAttribute(Device::UrlBase).endsWith("/") == false && scpdUrl.toString().startsWith("/") == false)
                    location = device->getAttribute(Device::UrlBase) + "/" + scpdUrl.toString();
                else
                    location = device->getAttribute(Device::UrlBase) + scpdUrl.toString();
            }

            ServiceFetcher f(service, location);

            if (!f.fetch()) {
                device->addService(service);
            } else {
                // TODO handle error
                qWarning() << "Failed to download service XML.";
            }
        }
    }

    QDomNodeList iconList = element.elementsByTagName("iconList");
    if (iconList.size() > 0) {
        QDomNodeList icons = iconList.at(0).toElement().elementsByTagName("icon");
        for (int i = 0; i < icons.size(); i++) {
            QDomElement iconElement = icons.at(0).toElement();
            Icon *icon = parseIcon(iconElement);
            device->addIcon(icon);
        }
    }

    QDomNodeList deviceList = element.firstChildElement("deviceList").childNodes();

    for (int i = 0; i < deviceList.size(); i++) {
        Device *embDevice = new Device(device);
        embDevice->setAttribute(Device::UrlBase, device->getAttribute(Device::UrlBase));
        QDomElement deviceElement = deviceList.at(i).toElement();
        parseDevice(embDevice, deviceElement);
        if(!embDevice->getAttribute(Device::FriendlyName).isEmpty())
            device->addDevice(embDevice);
    }
}

Service *DeviceXMLHandlerCP::parseService(QDomElement &element)
{
    QString serviceType = element.elementsByTagName("serviceType").at(0).toElement().text();
    QString serviceId = element.elementsByTagName("serviceId").at(0).toElement().text();
    QString controlURL = element.elementsByTagName("controlURL").at(0).toElement().text();
    QString eventSubURL = element.elementsByTagName("eventSubURL").at(0).toElement().text();
    QString SCPDURL = element.elementsByTagName("SCPDURL").at(0).toElement().text();

    return new Service(serviceType, serviceId, SCPDURL, controlURL, eventSubURL, "");
}

void DeviceXMLHandlerCP::validateURLBase(Device *device)
{
    QStringList urlBase = device->getAttribute(Device::UrlBase).split(":");
    if (urlBase.size() > PORT_INDEX) {
        QString port = urlBase[PORT_INDEX];
        QString newPort = "";
        quint8 index = 0;
        while (port[index].isDigit()) {
            newPort.append(port[index]);
            index++;
        }
        urlBase[PORT_INDEX] = newPort;
        while (urlBase.size() > PORT_INDEX + 1) urlBase.pop_back();
    }
    device->setAttribute(Device::UrlBase, urlBase.join(":"));
}

Icon *DeviceXMLHandlerCP::parseIcon(QDomElement &element)
{
    QString mimetype = element.elementsByTagName("mimetype").at(0).toElement().text();
    QString width = element.elementsByTagName("width").at(0).toElement().text();
    QString height = element.elementsByTagName("height").at(0).toElement().text();
    QString depth = element.elementsByTagName("depth").at(0).toElement().text();
    QString url = element.elementsByTagName("url").at(0).toElement().text();
    return new Icon(mimetype, width, height, depth, url);
}

}
}
}

