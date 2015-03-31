#ifndef SSPD_H
#define SSPD_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QList>
#include <QHostAddress>
#include <QStringList>
#include <QMap>

#include "ssdpprotocol.h"

//#define SSDP_ADDR "239.255.255.250"
//#define SSDP_PORT 1900

namespace brisa {
namespace shared {
namespace ssdp {

/*!
 * This class implements SSDP - Simple Service Discovery Protocol
 */
class SSDP: public QObject {
Q_OBJECT

public:
	explicit SSDP(int interval = 1, QObject *parent = 0);
	~SSDP();

    /*!
     * Start SSDP service
     */
    void start();

    /*!
     * Stop SSDP service
     */
	void stop();
	void doubleDiscover(const QString &type = QString("ssdp:all"),
			const QString &mx = QString("1"), const QString &userAgent =
					QString("UPnPlayer UPnP/1.1"));

	QMap<QString, QString> getMapFromMessage(QString message);

private:
    /*!
     * Create and bind a new UDP socket. Also create a pair signal/slot to deal with multicast datagram received.
     */
	void init();

	QTimer *discoverTimer;
	QUdpSocket *udpSocket;
	int interval;

private slots:
	void datagramReceived();

public slots:
	void discover(const QString &type = QString("ssdp:all"), const QString &mx =
			QString("1"),
			const QString &userAgent = QString("UPnPlayer UPnP/1.1"));

signals:
	void messageReceived(const QString &);
	void setupEvent(const QString &);

};

}
}
}

#endif // SSDP_H
