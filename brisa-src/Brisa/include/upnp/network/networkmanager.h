#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QList>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QUdpSocket>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "upnp/shared/ssdp/ssdpudpsocket.h"

class NetworkManager : public QObject {
	//TODO Verificar a necessidade desta classe.
	//Aparentemente ela foi criada apenas para conectar multicast, na interface correta, problema atualmente inexistente

    Q_OBJECT

    public:
        explicit NetworkManager(QObject *parent = 0);
        ~NetworkManager();
        QList<QNetworkInterface> getUpIPv4Interfaces();
        QList<QHostAddress> getAddresses(QNetworkInterface *);
        QList<SSDPUdpSocket*> enableMulticast(const QString &, quint16);
        QList<SSDPUdpSocket*> enableMulticast(const QString &, quint16, const QNetworkInterface &);
        QList<SSDPUdpSocket*> enableMulticast(const QString &, quint16, QList<QNetworkInterface>);
        QList<SSDPUdpSocket*> enableMulticastAny(const QString &, quint16);
        QList<SSDPUdpSocket*> enableMulticastAny(const QString &, quint16, const QNetworkInterface &);
        QList<SSDPUdpSocket*> enableMulticastAny(const QString &, quint16, QList<QNetworkInterface>);

    private:

    private slots:

    signals:

};
#endif // NETWORKMANAGER_H