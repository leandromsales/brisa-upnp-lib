#include "light.h"
#include "ui_light.h"
#include <QDebug>

using namespace Brisa;

Widget::Widget(QWidget *parent) :
    QWidget(parent), binaryLight(DEVICE_TYPE, DEVICE_FRIENDLY_NAME,
            DEVICE_MANUFACTURER, DEVICE_MANUFACTURER_URL,
            DEVICE_MODEL_DESCRIPTION, DEVICE_MODEL_NAME, DEVICE_MODEL_NUMBER,
            DEVICE_MODEL_URL, DEVICE_SERIAL_NUMBER, getCompleteUuid()) {
    //   ../../../../
    setupUi(this);
    contSplashScreen = 0;


    // BRisa Device
    SwitchPower *switchPower = new SwitchPower();
#ifdef Q_OS_UNIX
    splash
            = new QSplashScreen(
                    QPixmap(
                            "../../pixmaps/brisa/BrisaSplashScreenBinaryLight.png"));

    switchPower->setDescriptionFile("SwitchPower-scpd.xml");
    icon1.addPixmap(QPixmap(QString::fromUtf8(
            "../../pixmaps/brisa/onlight1.png")),
            QIcon::Normal, QIcon::Off);

    icon2.addPixmap(
            QPixmap(
                    QString::fromUtf8(
                            "../../pixmaps/brisa/offlight1.png")),
            QIcon::Normal, QIcon::Off);

#else
    splash
            = new QSplashScreen(
                    QPixmap(
                            "../../../pixmaps/brisa/BrisaSplashScreenBinaryLight.png"));

    switchPower->setDescriptionFile("../SwitchPower-scpd.xml");
    icon1.addPixmap(QPixmap(QString::fromUtf8(
            "../../../pixmaps/brisa/onlight1.png")),
            QIcon::Normal, QIcon::Off);

    icon2.addPixmap(
            QPixmap(
                    QString::fromUtf8(
                            "../../../pixmaps/brisa/offlight1.png")),
            QIcon::Normal, QIcon::Off);

#endif
    binaryLight.addService(switchPower);
    binaryLight.start();

    this->status
            = binaryLight.getServiceByType(
                    "urn:schemas-upnp-org:service:SwitchPower:1")->getVariable(
                    "Status");
    this->target
            = binaryLight.getServiceByType(
                    "urn:schemas-upnp-org:service:SwitchPower:1")->getVariable(
                    "Target");

    connect(status, SIGNAL(changed(BrisaStateVariable *)), this,
            SLOT(statechanged(BrisaStateVariable *)));

    this->statechanged(status);

}

Widget::~Widget() {
}

void Widget::statechanged(BrisaStateVariable *var) {
    if (var->getValue().toBool()) {
        pushButton1->setIcon(icon1);
    } else {
        pushButton1->setIcon(icon2);
    }
}

void Widget::on_pushButton1_toggled() {
    lampclicked();
}

void Widget::lampclicked() {
    if (pushButton1->isChecked()) {
        pushButton1->setIcon(icon2);
        status->setAttribute(BrisaStateVariable::Value, "0");
        target->setAttribute(BrisaStateVariable::Value, "0");
    } else {
        pushButton1->setIcon(icon1);
        status->setAttribute(BrisaStateVariable::Value, "1");
        target->setAttribute(BrisaStateVariable::Value, "1");
    }
}

void Widget::processSplashScreen() {
    Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;

    switch (contSplashScreen) {
    case 1:
        splash->showMessage(QObject::tr("Setting up Brisa Configuration..."),
                topRight, Qt::blue);
        break;
    case 2:
        splash->showMessage(QObject::tr("Creating Brisa Configuration..."),
                topRight, Qt::blue);
        break;
    }

    if (contSplashScreen > 2) {
        this->show();
        splash->finish(this);
        emit timeStop();
        delete splash;
    }
    contSplashScreen++;
}
