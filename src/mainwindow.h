#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QStateMachine>


#define LOCAL_DB_FILE_PATH  "./armor.db"
#define ETALON_DB_FILE_PATH ":/armor.db"
#define ETALON_DB_SKELETON_FILE_PATH ":/armor_skeleton.db"


namespace Ui {
class MainWindow;
} // namespase Ui

class QLabel;
class ProgressDialog;
class QFinalState;
class LogConsole;
class LedIndicator;
class AbstractCommunicator;
class QFile;


class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool configuringAllowed READ isConfiguringAllowed WRITE setConfiguringAllowed
               NOTIFY snlConfiguringAllowed)

public:

    struct ChannelSettings {
        int     currentSerialPortIdx;
        QString proxyHost;
        int     proxyPort;
        QString deviceIp;
        int     devicePort;
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    ChannelSettings currentChannelSettings() const;
    bool importSettingsFile(const QString &sourcefilename);


Q_SIGNALS:

    void snlLocalDatabaseChanged();
    void snlConfiguringAllowed(bool allowed);

protected:

    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

private Q_SLOTS:

    void stShowLogWindow();
    void stShowCalibrateADCWindow();
    void stShowFotaUploader();
    void stSetApplicationMode(QAction *menuAction);

    void stNewSettings();
    void stImportSettings();
    void stExportSettings();

    void stAbout();
    void stHelp();

    // -- AbstractCommunicator related
    void stChangeCommunicationType(QAction *action);
    void stOpenChannel();
    void stCloseChannel();

    void stReplaceDatabaseFile(const QByteArray &data);
    void stCheckDeviceDbVersion();
    void stUploadSettings();

private:

    void initStateMachine(AbstractCommunicator *communicator);
    void setupDatabase(const QString &dbfilename);
    bool overwriteLocalDatabase(const QByteArray &data);
    void replaceCommunicator(AbstractCommunicator *communicator);
    void updateWindowTitle(const QString &modeName);
    void changeLanguage(QAction *action);

    void restoreSettings();
    void saveSettings();

    bool isModified() const;
    bool isCorrectDbFileVersion(const QString &dbname);
//    bool isCorrectDownloadedDbVersion(const QByteArray &data);

    bool isConfiguringAllowed() const
    { return _bConfiguringAllowed; }

    void setConfiguringAllowed(bool allowed)
    {
        _bConfiguringAllowed = allowed;
        Q_EMIT snlConfiguringAllowed(allowed);
    }

    Ui::MainWindow *_ui;

    QLabel       *_lbl;
    LedIndicator *_ledindicator;
    LogConsole   *_logconsole;
    ProgressDialog       *_progressdialog;
    AbstractCommunicator *_communicator;

    bool _bConfiguringAllowed;

    // -- state machine
    enum State {
        S_Init,

        S_ChannelOpening,
        S_ChannelClosing,
        S_ChannelOpened,

        S_FromChannelOpenedToConfiguringIdle,
        S_FromConfiguringIdleToChannelOpened,

        S_ConfiguringIdle,
        S_ConfiguringInProgress,    // parent state

        S_DeviceDBVersionChecking,
        S_Uploading,
        S_UploadedOk,

        S_UploadingFota,
        S_reserved,

        S_Downloading,
        S_DownloadedOk,

        S_Reset,
        S_ReadKeyFromETR,

        S_Reserved,

        S_Saving,
        S_SavedOk,
        S_SavingFailed,

        S_MAX
    };

    QStateMachine *_machine;
    QList <QState *> _states;
    QFinalState *_stateFinal;

};

// TODO: add QLabel with statemachine's states indication;


#endif // MAINWINDOW_H
