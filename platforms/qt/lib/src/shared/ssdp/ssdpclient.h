/*
 * Universidade Federal de Campina Grande
 * Centro de Engenharia Elétrica e Informática
 * Laboratório de Sistemas Embarcados e Computação Pervasiva
 * BRisa Project / BRisa-Qt - http://brisa.garage.maemo.org
 * Filename: brisassdpClient.h
 * Created:
 * Description: Defines BrisaSSDPClient class.
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

#ifndef SSDPCLIENT_H
#define SSDPCLIENT_H

#include <QObject>
#include <QMap>

#include "../../upnp/brisaglobal.h"
#include "../../upnp/controlpoint/udplistener.h"

namespace brisa {
namespace shared {
namespace ssdp {

/*!
 *  \class Brisa::BrisaSSDPClient brisassdpclient.h BrisaUpnp/BrisaSSDPClient
 *  \brief SSDP stack implementantion for UPnP control points.
 *
 *  Create a new BrisaSSCPClient and call "start()" to connect to the multicast
 *  group and start listening to ssdp notification messages.
 *
 *  When BrisaSSDPClient receives a notification message it emits \a "newDeviceEvent()"
 *  in case of "ssdp:alive" and \a "removedDeviceEvent" in case of "ssdp:byebye".
 *  Other ssdp messages will be ignored.
 */
class BRISA_UPNP_EXPORT SSDPClient: public QObject {
Q_OBJECT

public:
    /*!
     *  Constructs a BrisaSSCPClient with the given parent.
     */
    SSDPClient(QObject *parent = 0);

    /*!
     *  Destroys the client.
     *
     *  Stops the client if it's running.
     */
    virtual ~SSDPClient();

public slots:
    /*!
     *
     *  Connects to the MultiCast group and starts the client.
     *
     *  \sa isRunning(), stop()
     */
    void start();

    /*!
     *  Stops the client.
     *
     *  \sa isRunning(), start()
     */
    void stop();

    /*!
     *  Checks if the client is running
     *
     *  \return true if is running
     */
    bool isRunning() const;
    QMap<QString, QString> getMapFromMessage(QString message);

signals:
    /*!
     *  \fn void BrisaSSDPClient::newDeviceEvent(const QString &usn, const QString &location,
     *                                           const QString &st, const QString &ext,
     *                                           const QString &server, const QString &cacheControl)
     *
     *  This signal is emitted when the client receives a "ssdp:alive" message from a device joining
     *  the network
     *
     *  \sa removedDeviceEvent()
     */
    void newDeviceEvent(const QString &usn, const QString &location,
            const QString &st, const QString &ext, const QString &server,
            const QString &cacheControl);

    /*!
     *  \fn void BrisaSSDPClient::removedDeviceEvent(const QString &usn)
     *
     *  This signal is emitted when the client receives a "ssdp:byebye" message from a device leaving the
     *  network
     *
     *  \sa newDeviceEvent()
     */
    void removedDeviceEvent(const QString &usn);
    void messageReceived(const QString &);

private slots:
    /*!
     *  \internal
     *  Receives UDP datagrams from a QUdpSocket.
     */
    void datagramReceived();

    /*!
     *  \internal
     *  Parses the UDP datagram received from "datagramReceived()".
     *
     *  \param datagram datagram
     */
    void notifyReceived(const QMap<QString, QString> &message);

private:
    bool running;

    upnp::controlpoint::UdpListener *udpListener;
};

}
}  // namespace shared
}

#endif /* _SSDPCLIENT_H */
