#include "precompiled.h"

#include "abstractcommunicator.h"
#include "logconsole.h"
#include "../src/crc.h"
#include "mainwindow.h"
#include "managerform/keysmanager.h"
#include "managerform/keyrequestdialog.h"
#include "progressdialog.h"

#include "bo/bo.h"

#include <QTime>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>


AbstractCommunicator::AbstractCommunicator(MainWindow *mainwindow, QObject *parent)
    : QObject(parent)
    , _mw(mainwindow)
    , _logconsole(NULL)
    , _nextPkt(0)
    , _wasSent(false)
    , _transactionKey(0)
    , _lastSentPktId(0)
    , _lastRecvPktId(0)
    , _wasProcessingCanceled(false)
    , _applyToFactoryArea(false)
    , _processedIterations(0)
    , _bFotaActive(false)
{
    _timerCollectReceivedData.setSingleShot(true);
    _timerNormalSentData.setSingleShot(true);
    connect(&_timerCollectReceivedData, &QTimer::timeout, this, &AbstractCommunicator::parseReceivedPkt);
    connect(&_timerNormalSentData,      &QTimer::timeout, this, &AbstractCommunicator::targetNotAnswer);
    connect(&_timerTargetDevCheck,      &QTimer::timeout, this, &AbstractCommunicator::checkConnection);
}

AbstractCommunicator::~AbstractCommunicator()
{
}

void AbstractCommunicator::addToLog(const QString &msgText,
                                    const QColor &msgTextColor)
{
    if (!_logconsole)
        return;

    _logconsole->appendTextLine(tr("Desc: "), msgText, Qt::darkGray, msgTextColor);
    _logconsole->appendSeparatorLine();
}

void AbstractCommunicator::addToLog(const QByteArray &data,
                                    const QString &prependText,
                                    const QString &description,
                                    const QColor &prependTextColor,
                                    const QColor &descriptionColor)
{
    if (!_logconsole)
        return;

    _logconsole->appendDataLine(data, prependText, prependTextColor);
    if (!description.isEmpty()) {
        _logconsole->appendTextLine(tr("Desc: "), description, Qt::darkGray, descriptionColor);
        _logconsole->appendSeparatorLine();
    }
}

bool AbstractCommunicator::isTxBufferedDataHasValidSize() const
{
    for (int i = 0; i < _txBuffer.size(); ++i) {
        /* WARNING: The next row uses magic numbers to get structure size (actually H and L bytes positions).
                    See static serialization method of any business object for more details. */
        quint16 struct_size = (quint8)_txBuffer.at(i)->at(1) << 8 | (quint8)_txBuffer.at(i)->at(2);
        if (struct_size > (getTxBufferedPktMaxSize() - SPS_PREF_N_SUFF))
            return false;
    }

    return true;
}

void AbstractCommunicator::appendToRxBuffer(int tableType, const QByteArray &data)
{
    if (_rxBuffer.contains(tableType)) { // append to existing table
        _rxBuffer.value(tableType)->append(data);
    }
    else { // insert new table
        BufferItem_ptr newTable(new QByteArray(data));
        _rxBuffer.insert(tableType, newTable);
        _rxBufferKeys.append(tableType);
    }
}

bool AbstractCommunicator::openChannel(QString &error)
{
    switch (openConcreteChannel(error)) {
    case OperationOk:
        Q_EMIT snlChannelOpened();
    case OperationShouldWait:
        return true;
    case OperationFailed:
    default:
        Q_EMIT snlChannelOpeningFailed();
        return error.isEmpty();
    }
}

void AbstractCommunicator::closeChannel()
{
    closeConcreteChannel();
    Q_EMIT snlChannelClosed();
}

void AbstractCommunicator::setDeviceToConfigurationState()
{
    resetTransactionContext();
    clearLowLevelBuffers();

    qsrand(QTime::currentTime().msecsTo(QTime(23, 59)));
    _transactionKey = qrand();

    QString msg = tr("Switching target device to <span style=\"color:purple\">CONFIGURING</span> mode...");
    fillSendBuffer(PC_CONFIGURATION_BEGIN, msg);
    sendBufferedPacket();
}

void AbstractCommunicator::setDeviceToDisarmedState()
{
    QString msg = tr("Switching target device to <span style=\"color:purple\">DISARMED</span> mode...");
    fillSendBuffer(PC_CONFIGURATION_END, msg);
    sendBufferedPacket();
}

void AbstractCommunicator::checkDeviceDbVersion()
{
    QByteArray db_version_str(DB_STRUCTURE_VERSION);
    fillSendBuffer(PC_CHECK_DEVICE_DB_VERSION, db_version_str, false, tr("Check device's DB version..."));
    sendBufferedPacket();
}

void AbstractCommunicator::beginUploading()
{
    _nextPkt = 0;
    fillSendBuffer(PC_UPLOAD_SETTINGS_START, tr("Start uploading..."));
    sendBufferedPacket();
}

inline void AbstractCommunicator::endUploading()
{
    _nextPkt = 0;
    if (isFotaActive()) fillSendBuffer(PC_UPLOAD_FOTA_FILE_NORMAL_END, tr("Start updating firmware..."));
    else fillSendBuffer(PC_UPLOAD_SETTINGS_NORMAL_END, tr("Check uploading result..."));
    sendBufferedPacket();
}

inline void AbstractCommunicator::interruptUploading()
{
    _nextPkt = 0;
    fillSendBuffer(PC_UPLOAD_SETTINGS_FAILED, tr("Uploading INTERRUPTED"));
    sendBufferedPacket();
}

void AbstractCommunicator::uploadNextPacket()
{
    static bool bAppendToFile = false;

    if (wasProcessingCanceled()) {
        bAppendToFile = false;
        interruptUploading();
        return;
    }

    if (_nextPkt < _txBuffer.size()) {

        QByteArray *curBuff = _txBuffer.at(_nextPkt).data();

        static DbObjectCode type = SEPARATOR_1;
        static quint8 size_hbyte = 0;
        static quint8 size_lbyte = 0;

        if (! bAppendToFile) {
            // WARNING: used magic numbers here
            type = static_cast<DbObjectCode>(curBuff->at(0));  // get TYPE byte
            size_hbyte = (quint8)curBuff->at(1);
            size_lbyte = (quint8)curBuff->at(2);
            curBuff->remove(0, 3);
        }

        const quint16 real_struct_size = size_hbyte << 8 | size_lbyte;  // get current structure size
        const quint16 possible_structures_to_send = (getTxBufferedPktMaxSize() - SPS_PREF_N_SUFF) / real_struct_size;
        const quint16 possible_bytes_to_send = possible_structures_to_send * real_struct_size;

        QByteArray d(curBuff->left(possible_bytes_to_send));
        curBuff->remove(0, possible_bytes_to_send);

        const bool tempAppendFlag = bAppendToFile;

        if (!curBuff->size()) {
            _nextPkt++;
            bAppendToFile = false;
        } else {
            bAppendToFile = true;   // tell to the device that it's needed to append next data block to existing file
        }

        fillSendBuffer(type, d, tempAppendFlag, tr("[ Settings Data ]"));
        sendBufferedPacket();
    } else {
        endUploading();
    }
}

void AbstractCommunicator::beginDownloading()
{
    fillSendBuffer(PC_DOWNLOAD_SETTINGS_START, tr("Start downloading..."));
    sendBufferedPacket();
}

void AbstractCommunicator::resetToDefaults()
{
    fillSendBuffer(PC_RESTORE_FACTORY_SETTINGS, tr("Reset the device to factory default state..."));
    sendBufferedPacket();
}

void AbstractCommunicator::saveUploadedSettings()
{
    fillSendBuffer(PC_CONFIGURATION_SAVE_AS_WORKING, tr("Save last uploaded configuration..."));
    sendBufferedPacket();
}

void AbstractCommunicator::requestKeyFromETR()
{
    int uin = KeysManager::getCurrentETRUIN();
    if (0 == uin)
        return;

    QByteArray keyValuePkt(TMVPKT_MAX, '\0');

    keyValuePkt[TMVPKT_ETR_UIN_H] = uin >> 8;
    keyValuePkt[TMVPKT_ETR_UIN_L] = uin;

    fillSendBuffer(PC_READ_LAST_TOUCHMEMORY_CODE, keyValuePkt, false, tr("Request a key from ETR..."));
    sendBufferedPacket();
}

void AbstractCommunicator::cancelRequestKeyFromETR()
{
    fillSendBuffer(PC_LAST_TOUCHMEMORY_CODE_FAILED, tr("Cancel KEY request"));
    sendBufferedPacket();
}

void AbstractCommunicator::requestAdcData(QWidget *parent)
{
    fillSendBuffer(PC_READ_ADC, tr("Request ADC data..."));
    sendBufferedPacket();

    // -- show busy progress
    ProgressDialog *pd = new ProgressDialog(parent);
    pd->setAttribute(Qt::WA_DeleteOnClose);
    pd->setCancelButton(0);
    connect(this, &AbstractCommunicator::snlAdcDataReceived, pd, &ProgressDialog::reset);
    connect(this, &AbstractCommunicator::snlNoResponseFromTargetDevice, pd, &ProgressDialog::reset);
    pd->stInitProgressDialog(0, tr("Wait for ADC value..."));
    pd->open();
}

inline void AbstractCommunicator::endDownloading()
{
    fillSendBuffer(PC_DOWNLOAD_SETTINGS_NORMAL_END, tr("Downloading is OK..."));
    sendBufferedPacket();
}

inline void AbstractCommunicator::interruptDownloading()
{
    fillSendBuffer(PC_DOWNLOAD_SETTINGS_FAILED, tr("Downloading INTERRUPTED"));
    sendBufferedPacket();
}

void AbstractCommunicator::downloadNextPacket()
{
    if (wasProcessingCanceled()) {
        interruptDownloading();
        return;
    }

    fillSendBuffer(PC_DOWNLOAD_SETTINGS_ACK, tr("[ ACK ]"));
    sendBufferedPacket();
}

// -- protected
void AbstractCommunicator::checkAvailableRxData()
{
    _rxBufferedPkt.append(readAllThroughConcreteChannel());
    parseReceivedPkt();
}

// -- privates
int AbstractCommunicator::evaluateUploadingIterations()
{
    int iters = 0;

    for (int i = 0; i < _txBuffer.size(); ++i) {
        QSharedPointer<QByteArray> tempArray = _txBuffer.at(i);

        /* WARNING: The next row uses magic numbers to get structure size (actually H and L bytes positions).
                    See static serialization method of any business object for more details. */
        const quint16 real_struct_size = (quint8)tempArray->at(1) << 8 | (quint8)tempArray->at(2);  // get current structure size

        const quint16 possible_structures_to_send = (getTxBufferedPktMaxSize() - SPS_PREF_N_SUFF) / real_struct_size;
        const quint16 possible_bytes_to_send = possible_structures_to_send * real_struct_size;

        int buffsize = tempArray->size() - 3;   // WARNING: used magic number (the size of current byte-array's header)
        iters += (buffsize % possible_bytes_to_send) ? (buffsize / possible_bytes_to_send + 1) : (buffsize / possible_bytes_to_send);
    }

    return iters;
}

void AbstractCommunicator::fillSendBuffer(DbObjectCode commandCode, const QString &comment)
{
    _txBufferedPkt.fill('\0', SPS_PREF_N_SUFF);  // resize the buffer and then fill it by 0 char

    _txBufferedPkt.data()[SPS_MARKER] = static_cast<char>(CONFIGURATION_PACKET_MARKER);
    _txBufferedPkt.data()[SPS_PKT_KEY] = _transactionKey;
    _txBufferedPkt.data()[SPS_FILECODE] = commandCode;

    _commentText = comment;
    _wasSent = false;
}

void AbstractCommunicator::fillSendBuffer(DbObjectCode dataType, const QByteArray &data,
                                          bool bAppendFlag, const QString &comment)
{
    _txBufferedPkt.fill('\0', SPS_PREF_N_SUFF);
    _txBufferedPkt.insert(SPS_PREFIX, data);

    _txBufferedPkt.data()[SPS_MARKER] = static_cast<char>(CONFIGURATION_PACKET_MARKER);
    _txBufferedPkt.data()[SPS_PKT_KEY] = _transactionKey;
    _txBufferedPkt.data()[SPS_FILECODE] = dataType;
    _txBufferedPkt.data()[SPS_APPEND_FLAG] = bAppendFlag ? 1 : 0;
    _txBufferedPkt.data()[SPS_BYTES_QTY_H] = data.size() >> 8;
    _txBufferedPkt.data()[SPS_BYTES_QTY_L] = data.size();

    _commentText = comment;
    _wasSent = false;
}

void AbstractCommunicator::sendBufferedPacket()
{
    if (_wasSent) {
        _txBufferedPkt.data()[SPS_PKT_REPEAT]++;
    } else {
        _txBufferedPkt.data()[SPS_PKT_REPEAT] = 0;
        ++_lastSentPktId;
    }

    _txBufferedPkt.data()[SPS_PKT_INDEX] = _lastSentPktId + _txBufferedPkt[SPS_PKT_REPEAT];
    _txBufferedPkt[_txBufferedPkt.size() - 1] = static_cast<char>(evaluateCRC(_txBufferedPkt, _txBufferedPkt.size() - 1));

    int sent_bytes = sendThroughConcreteChannel(_txBufferedPkt);

    if (sent_bytes < _txBufferedPkt.size()) {
        QString error = tr("Error. Really sent %1 byte(s). Should have sent %2 byte(s).")
                          .arg(sent_bytes)
                          .arg(_txBufferedPkt.size());
        addToLog(error, Qt::red);

        if (-1 == sent_bytes)
            addToLog(lastError(), Qt::red);
    } else {
        QString desc = QString(_txBufferedPkt[SPS_PKT_REPEAT]
                                    ? tr("<span style=\"color:olive\">[REPEAT] </span>")
                                    : "")
                                .append(_commentText);
        addToLog(_txBufferedPkt, tr("Send: "), desc, Qt::green);
    }

    //
    _wasSent = true;

    if (_timerTargetDevCheck.isActive())
        _timerTargetDevCheck.start();   // if the timer is active this interval need to be reset at every data sending attempt !!!

    if (!_timerNormalSentData.isActive())
        _timerNormalSentData.start(TIMER_NORMAL_SENT_INTERVAL);

    Q_EMIT snlTxData();
}

void AbstractCommunicator::mainRxParser(QByteArray &pkt)
{
    if (pkt.size() < SPS_PREF_N_SUFF)
        return;

    if (!_timerCollectReceivedData.isActive())
        _timerCollectReceivedData.start(500);   // start max wait interval to recv complete pkt

    const int bodyLen = (quint8)pkt.at(SPS_BYTES_QTY_H) << 8 | (quint8)pkt.at(SPS_BYTES_QTY_L);
    if (pkt.size() < SPS_PREF_N_SUFF + bodyLen)
        return;

    _timerCollectReceivedData.stop();
//    pkt.append(readAllThroughConcreteChannel());

    Q_EMIT snlRxData();

    while (pkt.size() >= SPS_PREF_N_SUFF) {
        quint16 datasize = 0;

        if ((quint8)pkt.at(SPS_MARKER) == CONFIGURATION_PACKET_MARKER)
        {
            datasize = (quint8)pkt[SPS_BYTES_QTY_H] << 8 | (quint8)pkt[SPS_BYTES_QTY_L];

            QByteArray temp_arr = pkt.left(SPS_PREF_N_SUFF + datasize);
            pkt.remove(0, SPS_PREF_N_SUFF + datasize);

            addToLog(temp_arr, tr("Recv: "), "", Qt::blue);

            if (!checkCRC(temp_arr, temp_arr.size())) {
                addToLog(tr("Bad CRC"), Qt::red);
                pkt.clear();
                return;
            }

            if ((quint8)temp_arr.at(SPS_PKT_KEY) == _transactionKey) {
                quint8 temp_pktId = (quint8)temp_arr.at(SPS_PKT_INDEX) -
                                        (quint8)temp_arr.at(SPS_PKT_REPEAT);
                if (temp_pktId == _lastSentPktId &&
                        (temp_pktId > _lastRecvPktId ||
                         /* note that touch memory key packet will have repeated SPS_PKT_INDEX */
                         PC_LAST_TOUCHMEMORY_CODE_OK == (quint8)temp_arr.at(SPS_FILECODE) ||
                         PC_LAST_TOUCHMEMORY_CODE_FAILED == (quint8)temp_arr.at(SPS_FILECODE)))
                {
                    // -- normal pkt
                    _timerNormalSentData.stop();
                    _lastRecvPktId = temp_pktId;

                    switch ((quint8)temp_arr.at(SPS_FILECODE)) {
                    case PC_CONFIGURATION_BEGIN:
                        addToLog(tr("Device is in CONFIGURING mode now"), Qt::magenta);
                        Q_EMIT snlDeviceReadyToConfiguration();
                        break;

                    case PC_CONFIGURATION_END:
                        addToLog(tr("Device is in DISARMED mode now"), Qt::magenta);
                        Q_EMIT snlDeviceSwitchedToDisarmedState();
                        break;

                    case PC_CONFIGURATION_DEVICE_ARMED:
                        addToLog(tr("Device is in ARMED mode now. Disarm all groups before configuring."), Qt::red);
                        Q_EMIT snlProcessingInterrupted();
                        break;

                    case PC_CONFIGURATION_CHECK_TARGET:
                        addToLog(tr("= DEVICE OK ="), Qt::black);
                        break;

                    case PC_CONFIGURATION_SAVE_AS_WORKING:
                        addToLog(tr("Device settings saved successfully"), Qt::green);
                        Q_EMIT snlProcessingSucceeded();
                        break;

                    case PC_RESTORE_FACTORY_SETTINGS:
                        addToLog(tr("Device settings restored to factory defaults"), Qt::black);
                        Q_EMIT snlProcessingSucceeded();
                        break;

                    case PC_READ_LAST_TOUCHMEMORY_CODE:
                    {
                        addToLog(tr("Wait for a touch memory key..."), Qt::black);
                        Q_EMIT snlProcessingSucceeded();
                        KeyRequestDialog *d = new KeyRequestDialog(_mw);
                        connect(this, &AbstractCommunicator::snlTouchMemoryKeyReceived,
                                d, &KeyRequestDialog::sltSetKey);
                        connect(d, &KeyRequestDialog::snlListChanged,
                                _mw, &MainWindow::snlLocalDatabaseChanged);
                        connect(d, &KeyRequestDialog::snlRequestCancel,
                                this, &AbstractCommunicator::cancelRequestKeyFromETR);
                        d->open();
                        break;
                    }

                    case PC_LAST_TOUCHMEMORY_CODE_FAILED:
                        addToLog(tr("Key reading failed"), Qt::red);
                        Q_EMIT snlTouchMemoryKeyReceived(QByteArray());
                        Q_EMIT snlProcessingSucceeded();
                        break;

                    case PC_LAST_TOUCHMEMORY_CODE_OK:
                    {
                        addToLog(tr("Key read successfully"), Qt::green);
                        char *key_string = temp_arr.data();
                        QByteArray key(&key_string[SPS_PREFIX + TMVPKT_KEY_H],      // key data
                                       temp_arr.at(SPS_PREFIX + TMVPKT_KEY_SIZE));  // key size
                        Q_EMIT snlTouchMemoryKeyReceived(key);
                        Q_EMIT snlProcessingSucceeded();
                        break;
                    }

                    case PC_READ_ADC:
                    {
                        addToLog(tr("ADC data received"), Qt::black);
                        quint16 adc_value = (quint8)temp_arr.at(SPS_PREFIX + ADCPKT_VALUE_H) << 8 |
                                            (quint8)temp_arr.at(SPS_PREFIX + ADCPKT_VALUE_L);
                        Q_EMIT snlAdcDataReceived(adc_value);
                        break;
                    }

                    default:
                    {
                        bool ret = false;
                        ret = ret || uploadingParser(temp_arr);
                        ret = ret || downloadingParser(temp_arr);

                        if (!ret) {
                            addToLog(tr("[ Regular data ]"), Qt::darkGray);
                            // NOTE: here a signal can be emitted to handle the data externally
                        }
                        break;
                    }
                    }

                    // -- display iterations of the progress dialog
                    //qDebug() << "total:" << _iterations << "current: " << _currentIteration;

                } else if (temp_pktId <= _lastRecvPktId) {
                    // -- repeated pkt index
                    addToLog(tr("Repeated packet. Will be ignored."), Qt::red);
                } else {
                    // -- bad pkt index
                    addToLog(tr("Ubnormal packet ID. Will be ignored."), Qt::red);
                }
            } else {
                // -- other bad packets
                addToLog(tr("Packet is not recognized. Will be ignored."), Qt::red);
            }

        }
        else
        {
            addToLog(pkt, tr("Recv: "), "[ ??? ]", Qt::blue);
            // NOTE: here a signal can be emitted to handle the data externally
            pkt.clear();
        }
    }
}

bool AbstractCommunicator::uploadingParser(const QByteArray &pkt)
{
    switch ((quint8)pkt.at(SPS_FILECODE)) {
    case PC_CHECK_DEVICE_DB_VERSION:
    {
        addToLog(tr("Device's DB version: OK"), Qt::black);
        Q_EMIT snlProcessingSucceeded();
        break;
    }

    case PC_UPLOAD_SETTINGS_START:
        addToLog(tr("Uploading STARTED"), Qt::black);
        _processedIterations = 0;
        Q_EMIT snlProcessingStarted(evaluateUploadingIterations(), tr("Uploading to the device..."));
        uploadNextPacket();
        break;

    case PC_UPLOAD_SETTINGS_NORMAL_END:
        addToLog(tr("Uploading OK"), Qt::darkGreen);
        Q_EMIT snlProcessingSucceeded();
        break;

    case PC_UPLOAD_SETTINGS_FAILED:
        addToLog(tr("Uploading aborted"), Qt::red);
        Q_EMIT snlProcessingInterrupted();
        break;

    case PC_UPLOAD_SETTINGS_ACK:
        addToLog(tr("[ ACK ]"), Qt::black);
        Q_EMIT snlIterationProcessed(_processedIterations++);
        uploadNextPacket();
        break;

    case PC_UPLOAD_FOTA_FILE_NORMAL_END:
        addToLog(tr("Firmware updating process started"), Qt::darkGreen);
        Q_EMIT snlProcessingSucceeded();
        break;

    default:
        return false;
    }

    return true;
}

bool AbstractCommunicator::downloadingParser(const QByteArray &pkt)
{
    const int dataType = (quint8)pkt.at(SPS_FILECODE);

    switch (dataType) {
    case PC_DOWNLOAD_SETTINGS_START:
    {
        addToLog(tr("Downloading STARTED"), Qt::black);
        _processedIterations = 0;
        int dl_iters = (quint8)pkt.at(SPS_PREFIX) << 8 | (quint8)pkt.at(SPS_PREFIX + 1);
        /* check DB version */
        QLatin1String db_version_str((char*)(pkt.data() + SPS_PREFIX + 2));
        if (0 != QString(DB_STRUCTURE_VERSION).compare(db_version_str)) {
            interruptDownloading();
            QMessageBox *pMsgbox = new QMessageBox(QMessageBox::Critical,
                                                   tr("Downloading..."),
                                                   tr("Invalid DB version found:\n"
                                                      "%1\n\n"
                                                      "Downloading aborted")
                                                        .arg(db_version_str),
                                                   QMessageBox::Ok,
                                                   _mw);
            pMsgbox->setAttribute(Qt::WA_DeleteOnClose);
            pMsgbox->open();
        }
        else {
            Q_EMIT snlProcessingStarted(dl_iters, tr("Downloading from the device..."));
            downloadNextPacket();
        }
        break;
    }

    case PC_DOWNLOAD_SETTINGS_NORMAL_END:
        addToLog(tr("Downloading OK"), Qt::darkGreen);
        Q_EMIT snlDownloadingComplete(processDownloadedSettings());
        Q_EMIT snlProcessingSucceeded();
        break;

    case PC_DOWNLOAD_SETTINGS_FAILED:
        addToLog(tr("Downloading aborted"), Qt::red);
        Q_EMIT snlProcessingInterrupted();
        break;

    case PC_DOWNLOAD_SETTINGS_NACK:
        addToLog(tr("[ NACK ]"), Qt::black);
        break;

    case OBJ_SQLITE_DB_FILE:
    default:
    {
        addToLog(tr("[ Settings Data ]"), Qt::black);
        int datasize = pkt.size() - SPS_PREF_N_SUFF;
        appendToRxBuffer(dataType, pkt.mid(SPS_PREFIX, datasize));
        Q_EMIT snlIterationProcessed(_processedIterations++);
        downloadNextPacket();
        break;
    }

    }

    return true;
}

void AbstractCommunicator::checkConnection()
{
    fillSendBuffer(PC_CONFIGURATION_CHECK_TARGET, tr("Checking target device..."));
    sendBufferedPacket();
}

void AbstractCommunicator::targetNotAnswer()
{
    if ((_txBufferedPkt[SPS_PKT_REPEAT] + 1) < SEND_ATTEMPTS_QTY) {
        sendBufferedPacket();
    } else {
        _timerNormalSentData.stop();
        addToLog(tr("Target device is not connected"), Qt::red);
        resetTransactionContext();
        Q_EMIT snlNoResponseFromTargetDevice();
    }
}

void AbstractCommunicator::resetTransactionContext()
{
    _lastSentPktId = 0;
    _lastRecvPktId = 0;
    _wasSent = false;
    _transactionKey = 0;
}

QByteArray AbstractCommunicator::processDownloadedSettings()
{
    QString tempname = QApplication::applicationDirPath() + QDir::separator() + QLatin1String("./downloaded~");

    // -- prepare a database skeleton
    QByteArray ba;
    if (QFile::exists(tempname) && !QFile::remove(tempname))
        return ba;
    if (!QFile::copy(QString(ETALON_DB_SKELETON_FILE_PATH), tempname) ||
            !QFile::setPermissions(tempname, QFile::ReadOwner|QFile::WriteOwner))
        return ba;

    // -- fill the database
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(tempname);
        if (!db.open())
            return ba;

        qx::QxSession session(db);
        Q_UNUSED(session);

        // -- tune the DB skeleton
        qx_query query("INSERT INTO s_codecs ('id', 'name') VALUES (0, 'ARMOR')");
        QSqlError error = qx::dao::call_query(query, &db);
        if (error.isValid())
            return ba;

        while (_rxBuffer.size()) {
            const int tableType = _rxBufferKeys.takeFirst();
            QSharedPointer<QByteArray> pData = _rxBuffer.take(tableType);
            qDebug() << tableType;
            switch (tableType) {
            case OBJ_ARMING_GROUP:
            {
                t_ArmingGroup::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_TOUCHMEMORY_CODE:
            case OBJ_KEYBOARD_CODE:
            {
                t_Key::insertFromByteArray(tableType, pData, &db);
                break;
            }
            case OBJ_ZONE:
            {
                t_Zone::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_RELAY:
            {
                t_Relay::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_LED:
            {
                t_Led::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_BELL:
            {
                t_Bell::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_BUTTON:
            {
                t_Button::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_COMMON_SETTINGS:
            {
                t_CommonSettings::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_SYSTEM_INFO:
            {
                t_SystemInfo::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_IPLIST_SIM1:
            case OBJ_IPLIST_SIM2:
            {
                SimCard card = OBJ_IPLIST_SIM1 == tableType ? SIM_1 : SIM_2;
                t_IpAddress::insertFromByteArray(card, pData, &db);
                break;
            }
            case OBJ_PHONELIST_SIM1:
            case OBJ_PHONELIST_SIM2:
            {
                SimCard card = OBJ_PHONELIST_SIM1 == tableType ? SIM_1 : SIM_2;
                t_Phone::insertFromByteArray(card, pData, &db);
                break;
            }
            case OBJ_PHONELIST_AUX:
            {
                t_AuxPhone::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_SIM_CARD:
            {
                t_SimCard::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_EVENT:
            {
                t_Event::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_BEHAVIOR_PRESET:
            {
                t_BehaviorPreset::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_REACTION:
            {
                t_Reaction::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_RELATION_ETR_AGROUP:
            {
                bo::insertRelationsEtrGroupFromByteArray(pData, &db);
                break;
            }
            case OBJ_ETR:
            {
                t_Etr::insertFromByteArray(pData, &db);
                break;
            }
            case OBJ_RZE:
            case OBJ_RAE:
            {
                t_Expander::insertFromByteArray(tableType, pData, &db);
                break;
            }
            case OBJ_MASTERBOARD:
            {
                t_SystemBoard::insertFromByteArray(pData, &db);
                break;
            }
            default:
                break;
            }
        }
    }

    QString connName = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(connName);

    // -- serialize the database file
    QFile f(tempname);
    if (f.open(QFile::ReadOnly)) {
        ba = f.readAll();
        f.close();
    }
    f.remove();
    return ba;
}
