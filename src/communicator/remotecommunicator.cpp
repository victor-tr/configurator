#include "precompiled.h"
#include "remotecommunicator.h"
#include "mainwindow.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


RemoteCommunicator::RemoteCommunicator(MainWindow *mainwindow, QObject *parent)
    : AbstractCommunicator(mainwindow, parent),
      m_bConnected(false)
{
    connect(&m_socket, &QTcpSocket::readyRead,    this, &RemoteCommunicator::checkAvailableRxData);
    connect(&m_socket, &QTcpSocket::connected,    this, &RemoteCommunicator::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &RemoteCommunicator::onDisconnected);

    connect(&m_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &RemoteCommunicator::onError);

    m_connectingTimer.setSingleShot(true);
    connect(&m_connectingTimer, &QTimer::timeout, this, &RemoteCommunicator::onConnectingTimeout);
}

AbstractCommunicator::OperationResult RemoteCommunicator::openConcreteChannel(QString &error)
{
    m_channel = _mw->currentChannelSettings();
    m_socket.connectToHost(m_channel.proxyHost, m_channel.proxyPort, QTcpSocket::ReadWrite);
    error.clear();
    m_connectingTimer.start(5000);
    return OperationShouldWait;
}

void RemoteCommunicator::closeConcreteChannel()
{
    m_socket.abort();
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.waitForDisconnected();
    m_bConnected = false;
}

void RemoteCommunicator::mainRxParser(QByteArray &pkt)
{
    if (m_bConnected) {
        if (pkt.size() < SPS_PREFIX)
            return;
        const int waitForLen = SPS_PREF_N_SUFF +
                                ((quint8)pkt.at(SPS_BYTES_QTY_H) << 8 | (quint8)pkt.at(SPS_BYTES_QTY_L));
        if (pkt.size() < waitForLen)
            return;
        return AbstractCommunicator::mainRxParser(pkt);
    }

    if (pkt.size() < 2)
        return;

    if (CONFIGURATION_PACKET_MARKER == static_cast<quint8>(pkt.at(0))) {
        m_connectingTimer.stop();
        if (pkt.at(1)) {
            m_bConnected = true;
            Q_EMIT snlChannelOpened();
            addToLog(pkt, tr("Recv: "), tr("[ Access granted ]"), Qt::blue);
        } else {
            onConnectingTimeout();
            addToLog(pkt, tr("Recv: "), tr("[ Access denied ]"), Qt::blue);
        }
    }
    else {
        addToLog(QByteArray(), tr("Read: "), pkt, Qt::blue);
    }

    Q_EMIT snlRxData();
    pkt.clear();
}

void RemoteCommunicator::onError(QAbstractSocket::SocketError e)
{
    qDebug() << "TCP socket: Error signal -" << e;

    m_connectingTimer.stop();
    bool bConnected = m_bConnected;
    closeConcreteChannel();

    switch (e) {
    case QAbstractSocket::HostNotFoundError:
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
        Q_EMIT snlChannelOpeningFailed();
        break;

    case QAbstractSocket::RemoteHostClosedError:
        if (!bConnected)
            Q_EMIT snlChannelOpeningFailed();
        break;

    default:
        break;
    }
}

void RemoteCommunicator::onConnected()
{
    qDebug() << "TCP socket: Connected signal";

    QStringList destIP = m_channel.deviceIp.split('.');
    if (destIP.size() != 4) {
        qDebug() << "Bad destination IP address";
        onConnectingTimeout();
        return;
    }

    QByteArray a;
    a.append(CONFIGURATION_PACKET_MARKER); // configurator's connecting packet marker
    a.append(destIP.at(0).toInt());
    a.append(destIP.at(1).toInt());
    a.append(destIP.at(2).toInt());
    a.append(destIP.at(3).toInt());
    a.append(m_channel.devicePort >> 8);
    a.append(m_channel.devicePort);

    sendThroughConcreteChannel(a);
    addToLog(a, tr("Send: "), tr("[ Configuring request ]"), Qt::green);
    Q_EMIT snlTxData();
    m_connectingTimer.start(5000);
}

void RemoteCommunicator::onDisconnected()
{
    qDebug() << "TCP socket: Disconnected signal";
    Q_EMIT snlChannelClosed();
}

void RemoteCommunicator::onConnectingTimeout()
{
    closeConcreteChannel();
    Q_EMIT snlChannelOpeningFailed();
}
