#ifndef REMOTECOMMUNICATOR_H
#define REMOTECOMMUNICATOR_H

#include "abstractcommunicator.h"
#include "mainwindow.h"
#include <QTcpSocket>


#define MAX_REMOTE_CONF_PKT_SIZE    160


class RemoteCommunicator : public AbstractCommunicator
{
    Q_OBJECT

public:

    explicit RemoteCommunicator(MainWindow *mainwindow, QObject *parent = 0);
    ~RemoteCommunicator() {}

    QString lastError() const             { return m_socket.errorString(); }
    QStringList availableComPorts() const { return QStringList(); }

protected:

    OperationResult openConcreteChannel(QString &error);
    void closeConcreteChannel();

    quint64 sendThroughConcreteChannel(const QByteArray &data) { return m_socket.write(data); }
    QByteArray readAllThroughConcreteChannel()                 { return m_socket.readAll(); }
    void clearLowLevelBuffers()                                {;}

    int getTxBufferedPktMaxSize() const { return MAX_REMOTE_CONF_PKT_SIZE; }

    void mainRxParser(QByteArray &pkt);

private:

    void onError(QAbstractSocket::SocketError e);
    void onConnected();
    void onDisconnected();
    void onConnectingTimeout();

    QTcpSocket m_socket;
    MainWindow::ChannelSettings m_channel;
    bool m_bConnected;
    QTimer m_connectingTimer;

};

#endif // REMOTECOMMUNICATOR_H




//Лунный Кот: q - 7
//Лунный Кот: xyz - 799
//Лунный Кот: 0xFC ip1 ip2 ip3 ip4
//Лунный Кот: 0xFC LenH LenL ... data ...
//Лунный Кот: 0xFC 0x01 - all sent OK
//Лунный Кот: 0xFC 0x00 - sending failure
