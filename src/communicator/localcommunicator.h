#ifndef LOCALCOMMUNICATOR_H
#define LOCALCOMMUNICATOR_H

#include "abstractcommunicator.h"
#include <QtSerialPort/QSerialPort>


#define M80_UART_MAX_SEND_SIZE 1024 // can be <= 1024, but when it is greater than 500 -> many errors


class LocalCommunicator : public AbstractCommunicator
{
    Q_OBJECT

public:

    explicit LocalCommunicator(MainWindow *mainwindow, QObject *parent = 0);
    ~LocalCommunicator();

    QString lastError() const;
    QStringList availableComPorts() const;

protected:

    OperationResult openConcreteChannel(QString &error);
    void closeConcreteChannel()                                { closePort(); }

    quint64 sendThroughConcreteChannel(const QByteArray &data) { return _serial->write(data); }
    QByteArray readAllThroughConcreteChannel()                 { return _serial->readAll(); }
    void clearLowLevelBuffers()                                { _serial->clear(); }

    int getTxBufferedPktMaxSize() const                        { return M80_UART_MAX_SEND_SIZE; }

private:

    virtual QSerialPort::BaudRate baudrate() const { return QSerialPort::Baud115200; }

    bool openPort(const QSerialPortInfo &info);
    void closePort() { _serial->close(); }
    void onPortError(QSerialPort::SerialPortError e);

    QSerialPort *_serial;
    mutable QList<QSerialPortInfo> _portInfoList;

};

#endif // LOCALCOMMUNICATOR_H
