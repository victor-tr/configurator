#ifndef ABSTRACTCOMMUNICATOR_H
#define ABSTRACTCOMMUNICATOR_H

#include "QObject"
#include <QTimer>
#include <QMap>
#include <QVector>
#include "configurator_protocol.h"

class MainWindow;
class LogConsole;


class AbstractCommunicator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool processingCanceled READ wasProcessingCanceled WRITE setProcessingCanceled
               RESET unsetProcessingCanceled)
    Q_PROPERTY(bool applyToFactoryArea READ isApplyToFactoryArea WRITE setApplyToFactoryArea)
    Q_PROPERTY(int processedIterations READ processedIterations WRITE setProcessedIterations)
    Q_PROPERTY(bool fotaActive READ isFotaActive WRITE setFotaActive)

public:

    typedef QSharedPointer<QByteArray> BufferItem_ptr;

    enum ConnectionType {
        DirectConnection,
        RemoteConnection,
        SerialPortToVpnBridge
    };

    virtual ~AbstractCommunicator();

    void setLogConsole(LogConsole *logconsole) { _logconsole = logconsole; }

    void addToLog(const QString &msgText,
                  const QColor &msgTextColor);

    void addToLog(const QByteArray &data,
                  const QString &prependText = QString(""),
                  const QString &description = QString(""),
                  const QColor &prependTextColor = QColor("darkgray"),
                  const QColor &descriptionColor = QColor("black"));

    bool wasProcessingCanceled() const        { return _wasProcessingCanceled; }
    void setProcessingCanceled(bool canceled) { _wasProcessingCanceled = canceled; }
    void setProcessingCanceled()              { _wasProcessingCanceled = true; }
    void unsetProcessingCanceled()            { _wasProcessingCanceled = false; }

    bool isApplyToFactoryArea() const      { return _applyToFactoryArea; }
    void setApplyToFactoryArea(bool apply) { _applyToFactoryArea = apply; }

    bool isFotaActive() const       { return _bFotaActive; }
    void setFotaActive(bool active) { _bFotaActive = active; }

    int  processedIterations() const           { return _processedIterations; }
    void setProcessedIterations(int iteration) { _processedIterations = iteration; }

    void activateConnectionChecking()   { _timerTargetDevCheck.start(TIMER_CONNECTION_CHECK_INTERVAL); }
    void deactivateConnectionChecking() { _timerTargetDevCheck.stop(); }

    void appendToTxBuffer(const QSharedPointer<QByteArray> &data) { _txBuffer.append(data); }
    void clearTxBuffer() { _txBuffer.clear(); }
    bool isTxBufferedDataHasValidSize() const;

    void appendToRxBuffer(int tableType, const QByteArray &data);
    void clearRxBuffer() { _rxBuffer.clear(); _rxBufferKeys.clear(); }

    virtual QString lastError() const             = 0;
    virtual QStringList availableComPorts() const = 0;

    bool openChannel(QString &error);
    void closeChannel();

    void setDeviceToConfigurationState();
    void setDeviceToDisarmedState();

    void checkDeviceDbVersion();
    void beginUploading()       ;
    void beginDownloading()     ;
    void resetToDefaults()      ;
    void saveUploadedSettings() ;

    void requestKeyFromETR();
    void cancelRequestKeyFromETR();

    void requestAdcData(QWidget *parent);

private:

    void endUploading()         ;
    void interruptUploading()   ;
    void uploadNextPacket()     ;
    void endDownloading()       ;
    void interruptDownloading() ;
    void downloadNextPacket()   ;

Q_SIGNALS:

    void snlTxData();
    void snlRxData();

    void snlChannelOpeningFailed();
    void snlChannelOpened();
    void snlChannelClosed();

    void snlDeviceReadyToConfiguration();
    void snlDeviceSwitchedToDisarmedState();

    void snlNoResponseFromTargetDevice();

    void snlProcessingStarted(int iterationsQty, const QString &label);
    void snlIterationProcessed(int progress);
    void snlProcessingInterrupted();
    void snlProcessingSucceeded();

    void snlDataReceived(const QByteArray &data);
    void snlDownloadingComplete(const QByteArray &data);

    void snlTouchMemoryKeyReceived(const QByteArray &key);
    void snlAdcDataReceived(quint16 adcValue);

protected:

    enum OperationResult {
        OperationFailed,
        OperationOk,
        OperationShouldWait
    };

    explicit AbstractCommunicator(MainWindow *mainwindow, QObject *parent = 0);

    virtual OperationResult openConcreteChannel(QString &error)  = 0;
    virtual void closeConcreteChannel() = 0;

    virtual quint64 sendThroughConcreteChannel(const QByteArray &data) = 0;
    virtual QByteArray readAllThroughConcreteChannel()                 = 0;
    virtual void clearLowLevelBuffers()                                = 0;

    virtual int getTxBufferedPktMaxSize() const = 0;

    void checkAvailableRxData();
    void parseReceivedPkt()  { mainRxParser(_rxBufferedPkt); }

    virtual void mainRxParser(QByteArray &pkt);

    MainWindow *_mw;

private:

    int evaluateUploadingIterations();

    void fillSendBuffer(DbObjectCode commandCode, const QString &comment = QString());
    void fillSendBuffer(DbObjectCode dataType, const QByteArray &data,
                        bool bAppendFlag = false, const QString &comment = QString());
    void sendBufferedPacket();

    bool uploadingParser(const QByteArray &pkt);
    bool downloadingParser(const QByteArray &pkt);

    void checkConnection();
    void targetNotAnswer();

    void resetTransactionContext();
    QByteArray processDownloadedSettings();

    LogConsole *_logconsole;

    QList<BufferItem_ptr> _txBuffer;
    QMap<int, BufferItem_ptr> _rxBuffer;
    QVector<int> _rxBufferKeys;
    QByteArray  _txBufferedPkt;
    QByteArray  _rxBufferedPkt;

    QString    _commentText;

    QTimer _timerNormalSentData;
    QTimer _timerCollectReceivedData;
    QTimer _timerTargetDevCheck;

    int    _nextPkt;
    bool   _wasSent;
    quint8 _transactionKey;
    quint8 _lastSentPktId;
    quint8 _lastRecvPktId;

    // -- properties
    bool _wasProcessingCanceled;
    bool _applyToFactoryArea;
    int  _processedIterations;
    bool _bFotaActive;

};


#endif // ABSTRACTCOMMUNICATOR_H
