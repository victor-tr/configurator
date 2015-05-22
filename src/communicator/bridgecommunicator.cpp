#include "bridgecommunicator.h"
#include "mainwindow.h"

/*!
 * \file
 * \author Roman Gladyshev
 * \brief BridgeCommunicator::BridgeCommunicator constructor
 *
 * Creates a bridge to target device via GSM VPN network using SimCom GSM-modem
 */
BridgeCommunicator::BridgeCommunicator(MainWindow *mainwindow, QObject *parent)
    : LocalCommunicator(mainwindow, parent)
    , _currentInitStep(IBMS_UnInitialized)
    , _modemState(ModemUninitialized)
    , _bReconnect(false)
{
    _timerModemInitNotResponse.setSingleShot(true);
    connect(&_timerModemInitNotResponse, &QTimer::timeout, this, &BridgeCommunicator::interruptInitialization);

    _timerModemUnInitNotResponse.setSingleShot(true);
    connect(&_timerModemUnInitNotResponse, &QTimer::timeout, this, &BridgeCommunicator::interruptUnInitialization);
}

/*!
 * \brief A human-readable description of the last occured error
 * \return String representation of the last error occured
 */
QString BridgeCommunicator::lastError()
{
    return LocalCommunicator::lastError();
}

/*!
 * \brief Opens the serial port and starts modem initialization process
 *
 * \a error parameter can contain a human readable error description, if the method returns \c OperationFailed
 * \return \c OperationShouldWait if serial port has opened
 *          and modem initialization has started, \c OperationFailed otherwise
 */
AbstractCommunicator::OperationResult BridgeCommunicator::openConcreteChannel(QString &error)
{
    OperationResult serial_opened = LocalCommunicator::openConcreteChannel(error);

    _currentInitStep = IBMS_UnInitialized;
    if (OperationOk == serial_opened) {
        _modemState = ModemInitialization;
        initBridgeModem();
        return OperationShouldWait;
    }

    return OperationFailed;
}

void BridgeCommunicator::closeConcreteChannel()
{
    _modemState = ModemUninitialization;
    unInitBridgeModem();
}

/*!
 * Processes data packet \a pkt that is specific for BridgeCommunicator class
 * (such as responses for AT-commands from a modem, etc.). Clears \a pkt if it was processed
 * successfully.
 */
void BridgeCommunicator::mainRxParser(QByteArray &pkt)
{
    if (wasProcessingCanceled()) {
        if (ModemInitialization == _modemState)
            interruptInitialization();
        else if (ModemUninitialization == _modemState)
            interruptUnInitialization();
        return;
    }

    if (ModemInitialized == _modemState
            && IBMS_ReadyToUse == _currentInitStep)        // directly to parent data parser
        return LocalCommunicator::mainRxParser(pkt);

    QString s(pkt.data());

    if (ModemInitialization == _modemState)
    {
        if (s.contains(QString("\r\nCONNECT\r\n"))) {
            _timerModemInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            _currentInitStep = IBMS_ReadyToUse;
            _modemState = ModemInitialized;
            Q_EMIT snlIterationProcessed(IBMS_ReadyToUse);
            Q_EMIT snlChannelOpened();
        } else if (s.contains(QString("\r\nOK\r\n"))) {
            _timerModemInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            Q_EMIT snlIterationProcessed(++_currentInitStep);
            initBridgeModem();
        } else if (s.contains(QString("\r\nERROR\r\n"))) {
            _timerModemInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            interruptInitialization();
        } else if (s.startsWith('+')) { // unsolicited result codes (URC)
            addToLog(pkt, tr("Recv: "), tr("[ URC from modem ]"), Qt::blue);
        } else {
            return LocalCommunicator::mainRxParser(pkt);
        }
    }
    else if (ModemUninitialization == _modemState)
    {
        if (s.contains(QString("\r\nSHUT OK\r\n"))) {
            _timerModemUnInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            Q_EMIT snlIterationProcessed(++_currentInitStep);
            unInitBridgeModem();
        } else if (UBMS_IPSHUT == _currentInitStep && s.contains(QString("\r\nOK\r\n"))) {
            _timerModemUnInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            _currentInitStep = IBMS_UnInitialized;
            _modemState = ModemUninitialized;
            LocalCommunicator::closeConcreteChannel();
            Q_EMIT snlIterationProcessed(IBMS_MAX);
        } else if (s.contains(QString("\r\nOK\r\n"))) {
            _timerModemUnInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            Q_EMIT snlIterationProcessed(++_currentInitStep);
            unInitBridgeModem();
        } else if (s.contains(QString("\r\nERROR\r\n"))) {
            _timerModemUnInitNotResponse.stop();
            addToLog(pkt, tr("Recv: "), tr("[ AT-command response ]"), Qt::blue);
            interruptUnInitialization();
        } else if (s.startsWith('+')) { // unsolicited result codes (URC)
            addToLog(pkt, tr("Recv: "), tr("[ URC from modem ]"), Qt::blue);
        } else {
            return LocalCommunicator::mainRxParser(pkt);
        }
    }

    Q_EMIT snlRxData();
    // NOTE: here a signal can be emitted to handle the data externally
    pkt.clear();
}

/*!
 * \brief Sequentially initializes the modem step-by-step
 */
void BridgeCommunicator::initBridgeModem()
{
    QByteArray at_command(1, '\r');

    switch (_currentInitStep) {
    case IBMS_UnInitialized:
        Q_EMIT snlProcessingStarted(IBMS_ReadyToUse, tr("Initializing modem...")); // show the progress dialog
        at_command.prepend("ATE0");
        _timerModemInitNotResponse.start(1000);
        break;

    case IBMS_ATE0:
        _bReconnect = false;
        at_command.prepend("ATI");
        break;

    case IBMS_ATI:
        at_command.prepend("AT+CGATT=1");
        break;

    case IBMS_CGATT:
        at_command.prepend("AT+CIPCSGP=1,\"vpnl.kyivstar.net\",\"1\",\"1\"");
        break;

    case IBMS_CIPCSGP:
//        at_command.prepend("AT+CDNSORIP=0"); // WARNING: this string for SIM-300 ONLY
        at_command.prepend("AT");   // this string for SIM-900 (skip current step)
        break;

    case IBMS_CDNSORIP:
        at_command.prepend("AT+CIPMODE=1");
        break;

    case IBMS_CIPMODE:
        /*  +CIPCCFG: <NmRetry>,<WaitTm * 200 ms>,<SendSz>,<esc>
            +CIPCCFG: <3-8>,<2-10>,<256-1024>,<0,1>     */
        at_command.prepend("AT+CIPCCFG=5,6,1024,1");
        break;

    case IBMS_CIPCCFG:
        at_command.prepend( QString("AT+CIPSTART=\"UDP\",\"%1\",\"%2\"")
                                .arg(_mw->currentChannelSettings().deviceIp)
                                .arg(_mw->currentChannelSettings().devicePort)
                                .toLatin1()
                           );
        break;

    case IBMS_WaitForConnect:
        _timerModemInitNotResponse.start(20000);
        return;

    default:
        return; // modem initialization finished
    }

    sendThroughConcreteChannel(at_command);
    addToLog(at_command, tr("Send: "), tr("[ AT-command ]"), Qt::green);
    Q_EMIT snlTxData();

    if (IBMS_UnInitialized != _currentInitStep)
        _timerModemInitNotResponse.start(10000);
}

/*!
 * \brief Resets the modem state to uninitialized. Emit snlProcessingInterrupted() signal
 */
void BridgeCommunicator::interruptInitialization()
{
    // if we are trying to connect and no response from the modem was received =>
    // we can try to switch off modem's transparent mode
    if (IBMS_UnInitialized == _currentInitStep) {
        _bReconnect = true;
        _modemState = ModemUninitialization;
        _currentInitStep = IBMS_ReadyToUse;
        unInitBridgeModem();
        return;
    }

    _currentInitStep = IBMS_UnInitialized;
    _modemState = ModemUninitialized;
    LocalCommunicator::closeConcreteChannel();
    Q_EMIT snlChannelOpeningFailed();      // back to init state of statemachine
}

void BridgeCommunicator::unInitBridgeModem()
{
    QByteArray at_command;

    switch (_currentInitStep) {
    case IBMS_ReadyToUse:
        Q_EMIT snlProcessingStarted(IBMS_MAX, tr("Uninitializing modem..."));
        at_command.append("+++");
        break;

    case UBMS_EscapeSequence:
        at_command.append("AT+CIPSHUT\r");
        break;

    case UBMS_IPSHUT:
        if (_bReconnect) {
            _bReconnect = false;
            _modemState = ModemInitialization;
            _currentInitStep = IBMS_UnInitialized;
            initBridgeModem();
            return;
        }
        else {
            at_command.append("AT+CGATT=0\r");
        }
        break;

    default:
        return;
    }

    sendThroughConcreteChannel(at_command);
    addToLog(at_command, tr("Send: "), tr("[ AT-command ]"), Qt::green);
    Q_EMIT snlTxData();
    _timerModemUnInitNotResponse.start(5000);
}

void BridgeCommunicator::interruptUnInitialization()
{
    _currentInitStep = IBMS_UnInitialized;
    _modemState = ModemUninitialized;
    LocalCommunicator::closeConcreteChannel();

    if (_bReconnect)
        Q_EMIT snlChannelOpeningFailed();
}

