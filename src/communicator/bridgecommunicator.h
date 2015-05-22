#ifndef BRIDGECOMMUNICATOR_H
#define BRIDGECOMMUNICATOR_H

#include "localcommunicator.h"


#define M80_UDP_PKT_MAX_SIZE 512

class BridgeCommunicator : public LocalCommunicator
{
    Q_OBJECT

public:

    BridgeCommunicator(MainWindow *mainwindow, QObject *parent = 0);
    ~BridgeCommunicator() { ; }

    QString lastError();

private:

    enum InitBridgeModemStep {
        IBMS_UnInitialized,
        IBMS_ATE0,
        IBMS_ATI,
        IBMS_CGATT,
        IBMS_CIPCSGP,
        IBMS_CDNSORIP,
        IBMS_CIPMODE,
        IBMS_CIPCCFG,
        IBMS_WaitForConnect,
        IBMS_ReadyToUse,

        UBMS_EscapeSequence,
        UBMS_IPSHUT,

        IBMS_MAX
    };

    enum ModemState {
        ModemUninitialized,
        ModemInitialization,
        ModemInitialized,
        ModemUninitialization
    };

    OperationResult openConcreteChannel(QString &error);
    void closeConcreteChannel();

    QSerialPort::BaudRate baudrate() const { return QSerialPort::Baud9600; }

    int getTxBufferedPktMaxSize() const { return M80_UDP_PKT_MAX_SIZE; }

    void mainRxParser(QByteArray &pkt);

    void initBridgeModem();
    void interruptInitialization();

    void unInitBridgeModem();
    void interruptUnInitialization();

    quint8 _currentInitStep;
    ModemState _modemState;

    QTimer _timerModemInitNotResponse;
    QTimer _timerModemUnInitNotResponse;

    bool _bReconnect;
};

#endif // BRIDGECOMMUNICATOR_H
