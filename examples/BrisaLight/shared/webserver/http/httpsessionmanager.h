#ifndef HTTPSESSIONMANAGER_H
#define HTTPSESSIONMANAGER_H

#include <QThread>
#include <QMutex>
#include "httpserver.h"

namespace brisa {
namespace shared {
namespace webserver {
namespace http {

class HttpSessionManager : public QThread
{
Q_OBJECT
public:
    explicit HttpSessionManager(HttpServer *parent);

    void run();

    void addSession(int socketDescriptor);
    void releaseSession(HttpSession *);

signals:
    void newConnection(int);

private slots:
    void onNewConnection(int socketDescriptor);

private:
    HttpServer *server;
    QList<HttpSession *> pool;
    QMutex mutex;
};

}  // namespace http
}  // namespace webserver
}  // namespace shared
} // namespace Brisa

#endif // HTTPSESSIONMANAGER_H
