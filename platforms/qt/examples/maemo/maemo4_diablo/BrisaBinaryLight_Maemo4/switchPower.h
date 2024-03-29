#ifndef _SWITCHPOWER_H_
#define _SWITCHPOWER_H_

#define SERVICE_TYPE "urn:schemas-upnp-org:service:SwitchPower:1"
#define SERVICE_ID "SwitchPower"
#define SERVICE_XML_PATH "/SwitchPower/SwitchPower-scpd.xml"
#define SERVICE_CONTROL "/SwitchPower/control"
#define SERVICE_EVENT_SUB "/SwitchPower/eventSub"

#include <BrisaUpnp/BrisaDevice>
#include <QtDebug>
#include <QtCore>
#include <QObject>

using namespace Brisa;

// The Actions
class GetStatus : public BrisaAction
{
    public:
        GetStatus(BrisaService *service) : BrisaAction("GetStatus", service) {}

    private:
        QMap<QString, QString> run(const QMap<QString, QString> &inArguments)
        {
            Q_UNUSED(inArguments)

            if (!(this->getService() && this->getService()->getVariable("Status")))
                qDebug() << this->getName() << "action couldn't find Service or StateVariable";

            QMap<QString, QString> outArgs;
            outArgs.insert("ResultStatus", this->getService()
                                           ->getVariable("Status")
                                           ->getAttribute(BrisaStateVariable::Value));
            return outArgs;
        }
};

class GetTarget : public BrisaAction
{
    public:
        GetTarget(BrisaService *service) : BrisaAction("GetTarget", service) {}

    private:
        QMap<QString, QString> run(const QMap<QString, QString> &inArguments)
        {
            Q_UNUSED(inArguments)

            if (!(this->getService() && this->getService()->getVariable("Target")))
                qDebug() << this->getName() << "action couldn't find Service or StateVariable";

            QMap<QString, QString> outArgs;
            outArgs.insert("RetTargetValue", this->getService()
                                             ->getVariable("Target")
                                             ->getAttribute(BrisaStateVariable::Value));
            return outArgs;
        }
};

class SetTarget : public BrisaAction
{
    Q_OBJECT

    public:
        SetTarget(BrisaService *service) : BrisaAction("SetTarget", service) {}

    private:
        QMap<QString, QString> run(const QMap<QString, QString> &inArguments)
        {
            if (!(this->getService() && this->getService()->getVariable("Target")))
                qDebug() << this->getName() << "action couldn't find Service or StateVariable";

            this->getService()->getVariable("Target")->setAttribute(BrisaStateVariable::Value,
                                                                    inArguments["NewTargetValue"]);
            this->getService()->getVariable("Status")->setAttribute(BrisaStateVariable::Value,
                                                                    inArguments["NewTargetValue"]);
            qDebug() << "Light switched" << inArguments["NewTargetValue"];

            QMap<QString, QString> outArgs;
            return outArgs;
        }
};

// The Service
class SwitchPower : public BrisaService
{
    Q_OBJECT

    public:
        SwitchPower() : BrisaService(SERVICE_TYPE,
                                     SERVICE_ID,
                                     SERVICE_XML_PATH,
                                     SERVICE_CONTROL,
                                     SERVICE_EVENT_SUB)
        {
            qDebug() << "Adding actions to binaryLight";
            addAction(new SetTarget(this));
            qDebug() << "Adding actions to binaryLight";
            addAction(new GetTarget(this));
            qDebug() << "Adding actions to binaryLight";
            addAction(new GetStatus(this));
            qDebug() << "Adding actions to binaryLight";
        }
};

#endif /* _SWITCHPOWER_H_ */
