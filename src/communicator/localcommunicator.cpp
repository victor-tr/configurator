#include "precompiled.h"
#include "localcommunicator.h"
#include "mainwindow.h"
#include "configurator_protocol.h"

#include <QtSerialPort/QSerialPortInfo>

#include <QxMemLeak.h>


LocalCommunicator::LocalCommunicator(MainWindow *mainwindow, QObject *parent)
    : AbstractCommunicator(mainwindow, parent),
      _serial(new QSerialPort(this))
{
    connect(_serial, &QSerialPort::readyRead, this, &LocalCommunicator::checkAvailableRxData);
    connect(_serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &LocalCommunicator::onPortError);
}

LocalCommunicator::~LocalCommunicator()
{
}

QString LocalCommunicator::lastError() const
{
    return _serial->errorString();
}

QStringList LocalCommunicator::availableComPorts() const
{
    _portInfoList = QSerialPortInfo::availablePorts();

    QStringList list;
    foreach (const QSerialPortInfo &info, _portInfoList) {
        //if (!info.isBusy()) {
            list << info.portName().append(" -- ").append(info.description())
//             << info.description()
//             << info.manufacturer()
//             << info.systemLocation()
//             << QString::number(info.vendorIdentifier())
//             << QString::number(info.productIdentifier())
                ;
        //}
    }

    return list;
}

LocalCommunicator::OperationResult LocalCommunicator::openConcreteChannel(QString &error)
{
    if (_portInfoList.isEmpty()) {
        error.clear();
        return OperationFailed;
    }

    int portIdx = _mw->currentChannelSettings().currentSerialPortIdx;
    bool ret = openPort(_portInfoList.at(portIdx));

    if (!ret) {
        error = tr("Can't open serial port: %1\n"
                   "Error: %2")
                       .arg(_portInfoList.at(portIdx).portName())
                       .arg(_serial->errorString());
    }

    return ret ? OperationOk : OperationFailed;
}


// -- private
inline bool LocalCommunicator::openPort(const QSerialPortInfo &info)
{
    _serial->setPort(info);

    bool ret = _serial->open(QSerialPort::ReadWrite);

    ret = ret && _serial->setBaudRate(baudrate());
    ret = ret && _serial->setDataBits(QSerialPort::Data8);
    ret = ret && _serial->setParity(QSerialPort::NoParity);
    ret = ret && _serial->setStopBits(QSerialPort::OneStop);
    ret = ret && _serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!ret) {
        closePort();
        return false;
    }

    _serial->clear();
    return true;
}

void LocalCommunicator::onPortError(QSerialPort::SerialPortError e)
{
    qDebug() << "SerialPortError: " << e;

#if defined(Q_OS_LINUX)
    if (QSerialPort::ResourceError == e)
#elif defined(Q_OS_WIN)
    if (QSerialPort::UnknownError == e ||
            QSerialPort::ResourceError == e ||
            QSerialPort::PermissionError == e)
#endif
    {
        if (_serial->isOpen()) {
            closePort();
            Q_EMIT snlChannelClosed();
        }
    }
}
