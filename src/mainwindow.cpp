#include "precompiled.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QLabel>
#include <QFileDialog>
#include <QSignalMapper>

#include <QState>
#include <QFinalState>
#include <QSettings>
#include <QKeyEvent>
#include <QWindow>
#include <QProcess>

#include "configurator_protocol.h"

#include "communicatorfactory.h"
#include "remotecommunicator.h"
#include "localcommunicator.h"
#include "bridgecommunicator.h"

#include "commonsettingsmanager.h"
#include "arminggroupsmanager.h"
#include "expandersmanager.h"
#include "keysmanager.h"
#include "systemboardmanager.h"
#include "etrmanager.h"
#include "reactionsmanager.h"
#include "eventsmanager.h"
#include "behaviorpresetsmanager.h"
#include "loopsmanager.h"
#include "relaysmanager.h"
#include "ledsmanager.h"
#include "buttonsmanager.h"
#include "bellsmanager.h"

#include "bo/t_phone.h"
#include "bo/t_auxphone.h"
#include "bo/t_ipaddress.h"
#include "bo/t_systeminfo.h"
#include "bo/t_simcard.h"
#include "bo/t_key.h"
#include "bo/t_systemboard.h"
#include "bo/s_relationetrgroup.h"

#include "logconsole.h"
#include "progressdialog.h"
#include "ledindicator.h"
#include "xmlsettingsprovider.h"
#include "adccalibrator.h"
#include "fotauploader.h"

#include <QxMemLeak.h>


enum MW_Tab {
    MWT_CommonSettings,
    MWT_SystemBoard,
    MWT_Expanders,
    MWT_InputDevices,
    MWT_SubUnits,
    MWT_ArmingGroups,
    MWT_Keys,
    MWT_Reactions
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _lbl(new QLabel(this))
    , _ledindicator(new LedIndicator())
    , _logconsole(new LogConsole(this))
    , _progressdialog(new ProgressDialog(this))
    , _communicator(NULL)
    , _machine(NULL)
    , _stateFinal(NULL)
{
    _ui->setupUi(this);

    _ui->btn_connect->setMinimumWidth(110);

    // -- setup status bar
    _ui->statusbar->addWidget(_lbl);
    _ui->statusbar->addPermanentWidget(_ledindicator);

    // -- main menu actions
    connect(_ui->actionNew,             &QAction::triggered, this, &MainWindow::stNewSettings);
    connect(_ui->actionExport_settings, &QAction::triggered, this, &MainWindow::stExportSettings);
    connect(_ui->actionImport_settings, &QAction::triggered, this, &MainWindow::stImportSettings);
    connect(_ui->actionViewLog,         &QAction::triggered, this, &MainWindow::stShowLogWindow);
    connect(_ui->actionCalibrate_ADC,   &QAction::triggered, this, &MainWindow::stShowCalibrateADCWindow);
    connect(_ui->actionFOTA_Uploader,   &QAction::triggered, this, &MainWindow::stShowFotaUploader);
    connect(_ui->actionAbout,           &QAction::triggered, this, &MainWindow::stAbout);
    connect(_ui->actionHelp,            &QAction::triggered, this, &MainWindow::stHelp);

    connect(_ui->actionQuit,            &QAction::triggered, this, &MainWindow::close);

    // --
//    QSignalMapper *signalmapper_2 = new QSignalMapper(this);
//    signalmapper_2->setMapping(ui->actionDirect_connection_RS_232, AbstractCommunicator::DirectConnection);
//    signalmapper_2->setMapping(ui->actionRemote_connection_LAN, AbstractCommunicator::RemoteConnection);
//    signalmapper_2->setMapping(ui->actionBridged_connection_GSM_modem, AbstractCommunicator::SerialPortToVpnBridge);

    // -- group connection modes
    QActionGroup *communicators_menu = new QActionGroup(this);
    _ui->actionDirect_connection_RS_232->setActionGroup(communicators_menu);
    _ui->actionRemote_connection_LAN->setActionGroup(communicators_menu);
    _ui->actionBridged_connection_GSM_modem_simcom->setActionGroup(communicators_menu);

    QObject::connect(communicators_menu, &QActionGroup::triggered,
                     this, &MainWindow::stChangeCommunicationType);

//    QObject::connect(ui->actionDirect_connection_RS_232, &QAction::triggered,
//                     signalmapper_2, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

//    QObject::connect(ui->actionRemote_connection_LAN, &QAction::triggered,
//                     signalmapper_2, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

//    QObject::connect(ui->actionBridged_connection_GSM_modem, &QAction::triggered,
//                     signalmapper_2, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

//    QObject::connect(signalmapper_2, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),
//                     this, &MainWindow::stCreateCommunicator);


    // -- group application modes
    QActionGroup *appModeMenuActions = new QActionGroup(this);
    appModeMenuActions->addAction(_ui->actionAdvancedMode);
    appModeMenuActions->addAction(_ui->actionSimpleMode);

    connect(appModeMenuActions, SIGNAL(triggered(QAction*)), SLOT(stSetApplicationMode(QAction*)));
    _ui->actionAdvancedMode->trigger();  // to ensure Advanced Mode of the application UI


    // -- group application languages
    QActionGroup *appLanguages = new QActionGroup(this);
    appLanguages->addAction(_ui->actionEnglish);
    appLanguages->addAction(_ui->actionRussian);

    connect(appLanguages, &QActionGroup::triggered, this, &MainWindow::changeLanguage);


    // -- setup DB connection
    QString localDatabaseFilename = QApplication::applicationDirPath() + QDir::separator() + QString(LOCAL_DB_FILE_PATH);
    if (!QFile::exists(localDatabaseFilename)) {
        QFile f(QString(ETALON_DB_FILE_PATH));
        if (!f.open(QFile::ReadOnly) || !overwriteLocalDatabase(f.readAll()))
        {
            QMessageBox::critical(this,
                                  tr("Error"),
                                  tr("Sorry\n"
                                     "Can't create local database"));
            qApp->quit();
        }
    }
    setupDatabase(localDatabaseFilename);


    // -- setup tabs
    CommonSettingsManager *commonsettings_mgr = new CommonSettingsManager();
    SystemBoardManager    *systemboard_mgr    = new SystemBoardManager();
    ExpandersManager      *expanders_mgr      = new ExpandersManager();
    EtrManager            *etr_mgr            = new EtrManager();
    KeysManager           *keys_mgr           = new KeysManager();
    ArmingGroupsManager   *arminggroups_gmgr  = new ArmingGroupsManager();

    LoopsManager   *zones_mgr   = new LoopsManager();
    RelaysManager  *relays_mgr  = new RelaysManager();
    LedsManager    *leds_mgr    = new LedsManager();
    BellsManager   *bells_mgr   = new BellsManager();
    ButtonsManager *buttons_mgr = new ButtonsManager();

    EventsManager          *events_mgr    = new EventsManager();
    ReactionsManager       *reactions_mgr = new ReactionsManager();
    BehaviorPresetsManager *behaviors_mgr = new BehaviorPresetsManager();


    // -- arming devices tab
    QTabWidget *armingDevices_TabWidget = new QTabWidget(this);

    QWidget::setTabOrder(_ui->tabWidget, armingDevices_TabWidget);

    armingDevices_TabWidget->insertTab(0, etr_mgr, tr("ETR"));
    armingDevices_TabWidget->insertTab(1, new QWidget(this), tr("Keyboards"));  // TODO: in future replace empty widget by a correct keyboard manager
    armingDevices_TabWidget->setTabEnabled(1, false);

    QWidget *armingDevicesTab = new QWidget(this);
    QGridLayout *armingDevicesTab_layout = new QGridLayout(armingDevicesTab);
    armingDevicesTab_layout->addWidget(armingDevices_TabWidget);

    // -- subdevices tab
    QTabWidget *subDevices_TabWidget = new QTabWidget(this);

    QWidget::setTabOrder(_ui->tabWidget, subDevices_TabWidget);

    subDevices_TabWidget->insertTab(0, zones_mgr,   tr("Protection Loops"));
    subDevices_TabWidget->insertTab(2, relays_mgr,  tr("Relays"));
    subDevices_TabWidget->insertTab(3, leds_mgr,    tr("Leds"));
    subDevices_TabWidget->insertTab(1, buttons_mgr, tr("Buttons"));
    subDevices_TabWidget->insertTab(4, bells_mgr,   tr("Bells"));

    QWidget *subDevicesTab = new QWidget(this);
    QGridLayout *subDevicesTab_layout = new QGridLayout(subDevicesTab);
    subDevicesTab_layout->addWidget(subDevices_TabWidget);

    // -- reaction tabs
    QTabWidget *reaction_TabWidget = new QTabWidget(this);

    QWidget::setTabOrder(_ui->tabWidget, reaction_TabWidget);

    reaction_TabWidget->insertTab(0, events_mgr,    tr("Triggers"));
    reaction_TabWidget->insertTab(1, behaviors_mgr, tr("Behaviors"));
    reaction_TabWidget->insertTab(2, reactions_mgr, tr("Reactions"));

    QWidget *reactionsTab = new QWidget(this);
    QGridLayout *reactionsTab_layout = new QGridLayout(reactionsTab);
    reactionsTab_layout->addWidget(reaction_TabWidget);


    // ---------------------------------
    _ui->tabWidget->removeTab(0);    // clear tabWidget
    _ui->tabWidget->insertTab(MWT_CommonSettings, commonsettings_mgr, tr("General Settings"));
    _ui->tabWidget->insertTab(MWT_SystemBoard,    systemboard_mgr,    tr("System Board"));
    _ui->tabWidget->insertTab(MWT_Expanders,      expanders_mgr,      tr("Expanders"));
    _ui->tabWidget->insertTab(MWT_InputDevices,   armingDevicesTab,   tr("Input Devices"));
    _ui->tabWidget->insertTab(MWT_SubUnits,       subDevicesTab,      tr("Sub-units"));
    _ui->tabWidget->insertTab(MWT_Keys,           keys_mgr,           tr("Keys"));
    _ui->tabWidget->insertTab(MWT_ArmingGroups,   arminggroups_gmgr,  tr("Groups"));
    _ui->tabWidget->insertTab(MWT_Reactions,      reactionsTab,       tr("Reactions"));

    // ---------------------------------
    // -- ALL LISTS MUST BE UPDATED
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), commonsettings_mgr, SLOT(sltUpdateData()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), systemboard_mgr,    SLOT(sltUpdateData()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), expanders_mgr,      SLOT(sltUpdateList()));

    // -- subunits tab
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), zones_mgr,          SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), relays_mgr,         SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), leds_mgr,           SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), buttons_mgr,        SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), bells_mgr,          SLOT(sltUpdateList()));

    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), etr_mgr,            SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), keys_mgr,           SLOT(sltUpdateList()));

    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), arminggroups_gmgr,  SLOT(sltUpdateList()));

    // -- reactions tab
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), events_mgr,         SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), reactions_mgr,      SLOT(sltUpdateList()));
    QObject::connect(this, SIGNAL(snlLocalDatabaseChanged()), behaviors_mgr,      SLOT(sltUpdateList()));


    // ---------------------------------
    // -- special controls
    QObject::connect(this, SIGNAL(snlConfiguringAllowed(bool)), keys_mgr, SLOT(sltActivateSpecialControls(bool)));


    // ---------------------------------
    // ANY DATA CHANGED
    QObject::connect(commonsettings_mgr, SIGNAL(snlDataChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(systemboard_mgr,    SIGNAL(snlDataChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(expanders_mgr,      SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));

    QObject::connect(zones_mgr,          SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(relays_mgr,         SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(leds_mgr,           SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(buttons_mgr,        SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(bells_mgr,          SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));

    QObject::connect(etr_mgr,            SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(keys_mgr,           SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(arminggroups_gmgr,  SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(events_mgr,         SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(reactions_mgr,      SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    QObject::connect(behaviors_mgr,      SIGNAL(snlListChanged()), this, SIGNAL(snlLocalDatabaseChanged()));
    // ---------------------------------


    _ui->actionDirect_connection_RS_232->trigger();  // to ensure Direct Connection
    restoreSettings();

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(950, sizeHint().height());

    setConfiguringAllowed(false);
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete _ui;
}

MainWindow::ChannelSettings MainWindow::currentChannelSettings() const
{
    ChannelSettings s;
    s.currentSerialPortIdx = _ui->cbox_serial_ports->currentIndex();
    s.proxyHost = _ui->le_proxy_host->text();
    s.proxyPort = _ui->sbox_proxy_port->value();
    s.deviceIp = _ui->le_device_ip->text();
    s.devicePort = _ui->sbox_device_port->value();
    return s;
}


// -- private slots
void MainWindow::stShowLogWindow()
{
    _logconsole->show();
}

void MainWindow::stShowCalibrateADCWindow()
{
    AdcCalibrator *adc_calibrator = new AdcCalibrator(_communicator, this);
    adc_calibrator->setAttribute(Qt::WA_DeleteOnClose);
    connect(adc_calibrator, &AdcCalibrator::snCalibrationDone, this, &MainWindow::snlLocalDatabaseChanged);
    adc_calibrator->open();
}

void MainWindow::stShowFotaUploader()
{
    FotaUploader *pFotaUploader = new FotaUploader(_communicator,
                                                   _states.at(S_ConfiguringIdle),
                                                   _states.at(S_UploadingFota),
                                                   this);
    pFotaUploader->setAttribute(Qt::WA_DeleteOnClose);
    pFotaUploader->open();
}

void MainWindow::stSetApplicationMode(QAction *menuAction)
{
    static QMenu *toolsMenu = _ui->menuTools;

    _ui->checkBox_factory_area->setChecked(false);

    if (menuAction == _ui->actionSimpleMode)
    {
        menuBar()->removeAction(toolsMenu->menuAction());
        _logconsole->close();
        _ui->checkBox_factory_area->setVisible(false);
    }
    else if (menuAction == _ui->actionAdvancedMode)
    {
        menuBar()->insertMenu(_ui->menuHelp->menuAction(), toolsMenu);
        _ui->checkBox_factory_area->setVisible(true);
    }
}

void MainWindow::stNewSettings()
{
    QFile sourcefile(QString(ETALON_DB_FILE_PATH));
    if (!sourcefile.open(QFile::ReadOnly) || !overwriteLocalDatabase(sourcefile.readAll()))
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Can't update local database"));
        return;
    }
    sourcefile.close();
}


#define SQLITE_DB_FILTER_TEXT QT_TRANSLATE_NOOP("MainWindow", "Database file (*.db)")
#define XML_FILE_FILTER_TEXT  QT_TRANSLATE_NOOP("MainWindow", "XML file (*.xml)")

void MainWindow::stImportSettings()
{
    QString sourcefilename = QFileDialog::getOpenFileName(this,
                                                          tr("Import settings from file"),
                                                          qApp->applicationDirPath(),
                                                          tr(SQLITE_DB_FILTER_TEXT) +
                                                          ";;" +
                                                          tr(XML_FILE_FILTER_TEXT));

    importSettingsFile(sourcefilename);
}

void MainWindow::stExportSettings()
{
    QFileDialog exportdialog(this,
                             tr("Export settings to file"),
                             qApp->applicationDirPath());

    QStringList filters;
    filters << tr(SQLITE_DB_FILTER_TEXT) << tr(XML_FILE_FILTER_TEXT);

    exportdialog.setNameFilters(filters);
    exportdialog.setAcceptMode(QFileDialog::AcceptSave);

    if (!exportdialog.exec() || !exportdialog.selectedFiles().size())
        return;

    QString savedfilename = exportdialog.selectedFiles().at(0);

    if (savedfilename.isEmpty())
        return;

    QString localDatabaseFilename = QApplication::applicationDirPath() + QDir::separator() + QString(LOCAL_DB_FILE_PATH);

    // prevent self exporting
    if (0 == QFileInfo(savedfilename).absoluteFilePath().compare(
                QFileInfo(localDatabaseFilename).absoluteFilePath()))
    {
        qDebug() << "It's the same file. Operation aborted.";
        return;
    }

    if (0 == exportdialog.selectedNameFilter().compare(tr(SQLITE_DB_FILTER_TEXT)))     // export to Sqlite db file
    {
        if (!savedfilename.endsWith(".db", Qt::CaseInsensitive))
            savedfilename.append(".db");

        if (QFile::exists(savedfilename))
            QFile::remove(savedfilename);

        bool success = QFile::copy(localDatabaseFilename, savedfilename);
        if (!success) {
            QMessageBox::critical(this,
                                  tr("Export settings"),
                                  tr("Exporting failed"));
        }
    }
    else if (0 == exportdialog.selectedNameFilter().compare(tr(XML_FILE_FILTER_TEXT)))   // export to XML file
    {
        if (!savedfilename.endsWith(".xml", Qt::CaseInsensitive))
            savedfilename.append(".xml");

        if (QFile::exists(savedfilename))
            QFile::remove(savedfilename);

        XmlSettingsProvider p(localDatabaseFilename);
        if (!p.exportSettings(savedfilename)) {
            QMessageBox::critical(this,
                                  tr("Export settings"),
                                  tr("Exporting failed"));
        }

    }
}

void MainWindow::stAbout()
{
    QMessageBox::about(this, tr("About"),
                       tr("<H2>%1</H2>"
                          "<H3>Ver. %2</H3>"
                          "<p>Based on Qt %3<br/>"
                          "Built on %4 at %5<br/>"
                          "Database version: %6</p>"
                          "<p>Roman Gladyshev &lt;remicollab@gmail.com&gt;</p>"

                          "<p>See README.TXT file at the application root directory for more details</p>"

                          "<p>© %7 ОЮА <a href='http://kratos.org.ua'><b>\"КРАТОС\"</b></a></p>")

                        .arg(qApp->applicationName()
                            , qApp->applicationVersion()
                            , QT_VERSION_STR
                            , __DATE__
                            , __TIME__
                            , DB_STRUCTURE_VERSION
                            , QString(__DATE__).right(4)
                            )
                       );

}

void MainWindow::stHelp()
{

}

void MainWindow::stChangeCommunicationType(QAction *action)
{
    if (action == _ui->actionBridged_connection_GSM_modem_simcom)
    {
        replaceCommunicator(CommunicatorCreator::instance()->createCommunicator<BridgeCommunicator>(this));
        updateWindowTitle(tr("BRIDGED CONNECTION mode"));

        _ui->lbl_serial_ports->setVisible(true);
        _ui->cbox_serial_ports->setVisible(true);

        _ui->lbl_proxy_host->setVisible(false);
        _ui->le_proxy_host->setVisible(false);
        _ui->lbl_proxy_port->setVisible(false);
        _ui->sbox_proxy_port->setVisible(false);

        _ui->lbl_device_ip->setVisible(true);
        _ui->le_device_ip->setVisible(true);
        _ui->lbl_device_port->setVisible(true);
        _ui->sbox_device_port->setVisible(true);

    }
    else if (action == _ui->actionRemote_connection_LAN)
    {
        replaceCommunicator(CommunicatorCreator::instance()->createCommunicator<RemoteCommunicator>(this));
        updateWindowTitle(tr("REMOTE CONNECTION mode"));

        _ui->lbl_serial_ports->setVisible(false);
        _ui->cbox_serial_ports->setVisible(false);

        _ui->lbl_proxy_host->setVisible(true);
        _ui->le_proxy_host->setVisible(true);
        _ui->lbl_proxy_port->setVisible(true);
        _ui->sbox_proxy_port->setVisible(true);

        _ui->lbl_device_ip->setVisible(true);
        _ui->le_device_ip->setVisible(true);
        _ui->lbl_device_port->setVisible(true);
        _ui->sbox_device_port->setVisible(true);

    }
    else
    {
        replaceCommunicator(CommunicatorCreator::instance()->createCommunicator<LocalCommunicator>(this));
        updateWindowTitle(tr("DIRECT CONNECTION mode"));

        _ui->lbl_serial_ports->setVisible(true);
        _ui->cbox_serial_ports->setVisible(true);

        _ui->lbl_proxy_host->setVisible(false);
        _ui->le_proxy_host->setVisible(false);
        _ui->lbl_proxy_port->setVisible(false);
        _ui->sbox_proxy_port->setVisible(false);

        _ui->lbl_device_ip->setVisible(false);
        _ui->le_device_ip->setVisible(false);
        _ui->lbl_device_port->setVisible(false);
        _ui->sbox_device_port->setVisible(false);

    }
}

void MainWindow::stOpenChannel()
{
    Q_ASSERT_X(_communicator, "MainWindow::stOpenChannel()",
               "Uninitialized _communicator");

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString openingError;
    if (!_communicator->openChannel(openingError)) {
         QMessageBox *msg = new QMessageBox(QMessageBox::Critical ,
                                            tr("Channel opening error"),
                                            openingError,
                                            QMessageBox::Ok,
                                            this);
        msg->setAttribute(Qt::WA_DeleteOnClose, true);
        msg->open();

        _ui->statusbar->showMessage(tr("Channel opening error"), 3000);
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::stCloseChannel()
{
    Q_ASSERT_X(_communicator, "MainWindow::stCloseChannel()",
               "Uninitialized _communicator");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    _communicator->closeChannel();
    QApplication::restoreOverrideCursor();
}

void MainWindow::stReplaceDatabaseFile(const QByteArray &data)
{
    if (data.isEmpty())
        return;

    if (overwriteLocalDatabase(data)) {
        qDebug("Settings have been downloaded successfully");
        QMessageBox *pMsgbox = new QMessageBox(QMessageBox::Information,
                                               tr("Downloading"),
                                               tr("Database successfully downloaded"),
                                               QMessageBox::Ok,
                                               this);
        pMsgbox->setAttribute(Qt::WA_DeleteOnClose);
        pMsgbox->setIconPixmap(QPixmap(":/icons/icons/green_check.png"));
        pMsgbox->open();
    } else {
        QMessageBox::critical(this,
                              tr("Downloading"),
                              tr("Can't save downloaded settings.\n"
                              "Downloading process aborted"));
    }
}

void MainWindow::stCheckDeviceDbVersion()
{
    Q_ASSERT_X(_communicator, "MainWindow::stCheckDeviceDbVersion()",
               "Uninitialized _communicator");

    _communicator->checkDeviceDbVersion();
}

void MainWindow::stUploadSettings()
{
    Q_ASSERT_X(_communicator, "MainWindow::stUploadSettings()",
               "Uninitialized _communicator");

    _communicator->clearTxBuffer();

    // -- check for any unsaved changes
    if (isModified()) {
        if  (QMessageBox::No == QMessageBox::warning(this,
                                                     tr("Uploading"),
                                                     tr("You have one or more unsaved changes.\n"
                                                     "Any of unsaved changes won't be uploaded.\n\n"
                                                     "Do you want to proceed the uploading process?"),
                                                     QMessageBox::Yes|QMessageBox::No,
                                                     QMessageBox::No
                                                     ))
        {
            Q_EMIT _communicator->snlProcessingInterrupted();
            return;
        }
    }

    // -- compress the DB file
    qx_query query("VACUUM");
    qx::dao::call_query(query);

    // -- check if the DB file version is correct
    t_SystemInfo_ptr pSysInfo(new t_SystemInfo());
    pSysInfo->_id = 1;
    qx::dao::fetch_by_id(pSysInfo);
    bool db_version_ok = (DB_STRUCTURE_VERSION == pSysInfo->_settings_ident_string);
    if (!db_version_ok) {
        if (QMessageBox::No == QMessageBox::warning(this,
                              tr("Uploading"),
                              tr("Note that the DB version is not newest and\n"
                              "can be incompatible with the device firmware.\n"
                              "Do you want to proceed?"),
                              QMessageBox::Yes|QMessageBox::No,
                              QMessageBox::No
                              ))
        {
            Q_EMIT _communicator->snlProcessingInterrupted();
            return;
        }
    }

    AbstractCommunicator::BufferItem_ptr ba;

    // -- serialize the DB file
//    QFile dbfile(qx::QxSqlDatabase::getSingleton()->getDatabaseName());
//    if (! dbfile.open(QIODevice::ReadOnly)) {
//        QMessageBox::critical(this,
//                              tr("Uploading error"),
//                              tr("The database file can not be uploaded to the device\n"
//                              "Abort")
//                              );
//        Q_EMIT _communicator->snlProcessingInterrupted();
//        return;
//    }

//    if (dbfile.size()) {
//        ba.reset( new QByteArray() );
//        ba->append(static_cast<char>(OBJ_SQLITE_DB_FILE));
//        ba->append(static_cast<char>(0));    // WARNING: magic numbers !!!
//        ba->append(static_cast<char>(1));
//        ba->append(dbfile.readAll());
//        if (ba->size() > 3)
//            _communicator->appendToTxBuffer(ba);
//    }
    // -----

    // -- serialize the tables
    ba = t_SystemBoard::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_ArmingGroup::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Etr::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Expander::fetchToByteArray(OBJ_RZE);
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Expander::fetchToByteArray(OBJ_RAE);
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Key::fetchToByteArray(OBJ_KEYBOARD_CODE);
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Key::fetchToByteArray(OBJ_TOUCHMEMORY_CODE);
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_CommonSettings::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_AuxPhone::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Zone::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Relay::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Led::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Bell::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Button::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_SimCard::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    for (SimCard i = SIM_1; i < SIM_MAX; i = (SimCard)(i + 1)) {
        ba = t_Phone::fetchToByteArray(i);
        if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

        ba = t_IpAddress::fetchToByteArray(i);
        if (!ba.isNull()) _communicator->appendToTxBuffer(ba);
    }

    ba = t_SystemInfo::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Event::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_Reaction::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = t_BehaviorPreset::fetchToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    ba = bo::fetchRelationsEtrGroupToByteArray();
    if (!ba.isNull()) _communicator->appendToTxBuffer(ba);

    // -- warn about too big structure size
    if (!_communicator->isTxBufferedDataHasValidSize()) {
            QMessageBox::critical(this,
                                  tr("Uploading error"),
                                  tr("Too big data structure size. No data will be sent to the target device\n"));
            _communicator->clearTxBuffer();
            Q_EMIT _communicator->snlProcessingInterrupted();
            return;
    }

    _communicator->beginUploading();
}


// -- private methods

#define BTN_OPENED_TEXT             QT_TRANSLATE_NOOP("MainWindow", "Disconnect")
#define BTN_CLOSED_TEXT             QT_TRANSLATE_NOOP("MainWindow", "Connect")
#define BTN_DISARMED_MODE_TEXT      QT_TRANSLATE_NOOP("MainWindow", "Configuring Mode")
#define BTN_CONFIGURATION_MODE_TEXT QT_TRANSLATE_NOOP("MainWindow", "Disarmed Mode")

void MainWindow::initStateMachine(AbstractCommunicator *communicator)
{
    // -- clean up the state machine
    if (_machine) {
        _states.clear();
        _machine->deleteLater();
    }

    // initialize new state machine
    _machine = new QStateMachine(this);

    for (int i = 0; i < S_MAX; ++i) {
        if (i <= S_ConfiguringInProgress)
            _states.append(new QState(_machine));
        else
            _states.append(new QState(_states.at(S_ConfiguringInProgress)));
    }

    QState *s = NULL;

    // -- INIT
    s = _states.at(S_Init);

    s->assignProperty(_lbl, "text", tr("Channel closed"));
    s->assignProperty(communicator, "processingCanceled", false);

    s->assignProperty(_ui->actionDirect_connection_RS_232, "enabled", true);
    s->assignProperty(_ui->actionRemote_connection_LAN, "enabled", true);
    s->assignProperty(_ui->actionBridged_connection_GSM_modem_simcom, "enabled", true);

    s->assignProperty(_ui->btn_connect, "enabled", true);
    s->assignProperty(_ui->btn_connect, "text", tr(BTN_CLOSED_TEXT));
    s->assignProperty(_ui->cbox_serial_ports, "enabled", true);
    s->assignProperty(_ui->le_proxy_host, "enabled", true);
    s->assignProperty(_ui->sbox_proxy_port, "enabled", true);
    s->assignProperty(_ui->le_device_ip, "enabled", true);
    s->assignProperty(_ui->sbox_device_port, "enabled", true);

    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", false);
    s->assignProperty(_ui->btn_ConfigurationMode, "text", tr(BTN_DISARMED_MODE_TEXT));
    s->assignProperty(_ui->btn_ConfigurationMode, "icon", QIcon(":/icons/icons/configure_4286.png"));

    s->assignProperty(_ui->btn_download, "enabled", false);
    s->assignProperty(_ui->btn_upload, "enabled", false);
    s->assignProperty(_ui->btn_saveAndRestart, "enabled", false);
    s->assignProperty(_ui->btn_reset_to_factory_defaults, "enabled", false);
    s->assignProperty(_ui->checkBox_factory_area, "enabled", false);
    s->assignProperty(_ui->actionCalibrate_ADC, "enabled", false);
    s->assignProperty(_ui->actionFOTA_Uploader, "enabled", false);
    s->assignProperty(this, "configuringAllowed", false);
    s->assignProperty(_communicator, "fotaActive", false);

    s->addTransition(_ui->btn_connect, SIGNAL(clicked()), _states.at(S_ChannelOpening));

    connect(s, &QState::propertiesAssigned, _progressdialog, &ProgressDialog::reset);


    // -- OPENING
    s = _states.at(S_ChannelOpening);

    s->assignProperty(_lbl, "text", tr("Channel opening..."));

    // block the menues while the channel is opened
    s->assignProperty(_ui->actionDirect_connection_RS_232, "enabled", false);
    s->assignProperty(_ui->actionRemote_connection_LAN, "enabled", false);
    s->assignProperty(_ui->actionBridged_connection_GSM_modem_simcom, "enabled", false);

    s->assignProperty(_ui->btn_connect, "enabled", false);
    s->assignProperty(_ui->cbox_serial_ports, "enabled", false);
    s->assignProperty(_ui->le_proxy_host, "enabled", false);
    s->assignProperty(_ui->sbox_proxy_port, "enabled", false);
    s->assignProperty(_ui->le_device_ip, "enabled", false);
    s->assignProperty(_ui->sbox_device_port, "enabled", false);

    s->addTransition(communicator, SIGNAL(snlChannelOpened()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlChannelOpeningFailed()), _states.at(S_Init));

    connect(s, &QState::propertiesAssigned, this, &MainWindow::stOpenChannel);


    // -- CLOSING
    s = _states.at(S_ChannelClosing);

    s->assignProperty(_lbl, "text", tr("Channel closing..."));

    s->assignProperty(_ui->btn_connect, "enabled", false);
    s->assignProperty(_ui->btn_connect, "text", tr(BTN_CLOSED_TEXT));
//    s->assignProperty(ui->cbox_serial_ports, "enabled", false);
//    s->assignProperty(ui->le_proxy_host, "enabled", false);
//    s->assignProperty(ui->sbox_proxy_port, "enabled", false);
//    s->assignProperty(ui->le_device_ip, "enabled", false);
//    s->assignProperty(ui->sbox_device_port, "enabled", false);

    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", false);

    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));

    connect(s, &QState::propertiesAssigned, this, &MainWindow::stCloseChannel);


    // -- OPENED
    s = _states.at(S_ChannelOpened);

    s->assignProperty(_lbl, "text", tr("Channel opened"));
    s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));

    s->assignProperty(_ui->btn_connect, "enabled", true);
    s->assignProperty(_ui->btn_connect, "text", tr(BTN_OPENED_TEXT));

    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", true);
    s->assignProperty(_ui->btn_ConfigurationMode, "text", tr(BTN_DISARMED_MODE_TEXT));
    s->assignProperty(_ui->btn_ConfigurationMode, "icon", QIcon(":/icons/icons/configure_4286.png"));

    // -- if device does not answer => switch directly from CONFIGURING IDLE to OPENED
    s->assignProperty(_ui->btn_download, "enabled", false);
    s->assignProperty(_ui->btn_upload, "enabled", false);
    s->assignProperty(_ui->btn_saveAndRestart, "enabled", false);
    s->assignProperty(_ui->btn_reset_to_factory_defaults, "enabled", false);
    s->assignProperty(_ui->checkBox_factory_area, "enabled", false);
    s->assignProperty(_ui->actionCalibrate_ADC, "enabled", false);
    s->assignProperty(_ui->actionFOTA_Uploader, "enabled", false);
    s->assignProperty(this, "configuringAllowed", false);
    s->assignProperty(_communicator, "fotaActive", false);
    // -- end block

    s->addTransition(_ui->btn_connect, SIGNAL(clicked()), _states.at(S_ChannelClosing));
    s->addTransition(_ui->btn_ConfigurationMode, SIGNAL(clicked()), _states.at(S_FromChannelOpenedToConfiguringIdle));
    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));


    // -- from OPENED to CONFIGURING IDLE
    s = _states.at(S_FromChannelOpenedToConfiguringIdle);

    s->assignProperty(_lbl, "text", tr("Switching to CONFIGURING mode..."));
    s->assignProperty(this, "cursor", QCursor(Qt::WaitCursor));
    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", false);

    s->addTransition(communicator, SIGNAL(snlDeviceReadyToConfiguration()), _states.at(S_ConfiguringIdle));
    s->addTransition(communicator, SIGNAL(snlNoResponseFromTargetDevice()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));

    connect(s, &QState::propertiesAssigned, communicator, &AbstractCommunicator::setDeviceToConfigurationState);


    // -- from CONFIGURING IDLE to OPENED
    s = _states.at(S_FromConfiguringIdleToChannelOpened);

    s->assignProperty(_lbl, "text", tr("Switching to DISARMED mode..."));

    s->assignProperty(this, "cursor", QCursor(Qt::WaitCursor));
    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", false);

    s->assignProperty(_ui->btn_download, "enabled", false);
    s->assignProperty(_ui->btn_upload, "enabled", false);
    s->assignProperty(_ui->btn_saveAndRestart, "enabled", false);
    s->assignProperty(_ui->btn_reset_to_factory_defaults, "enabled", false);
    s->assignProperty(_ui->checkBox_factory_area, "enabled", false);
    s->assignProperty(_ui->actionCalibrate_ADC, "enabled", false);
    s->assignProperty(_ui->actionFOTA_Uploader, "enabled", false);
    s->assignProperty(this, "configuringAllowed", false);

    s->addTransition(communicator, SIGNAL(snlDeviceSwitchedToDisarmedState()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlNoResponseFromTargetDevice()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));

    connect(s, &QState::propertiesAssigned, communicator, &AbstractCommunicator::setDeviceToDisarmedState);


    // -- CONFIGURING IDLE
    s = _states.at(S_ConfiguringIdle);

    s->assignProperty(_lbl, "text", tr("Ready for configuring..."));
    s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));

    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", true);
    s->assignProperty(_ui->btn_ConfigurationMode, "text", tr(BTN_CONFIGURATION_MODE_TEXT));
    s->assignProperty(_ui->btn_ConfigurationMode, "icon", QIcon(":/icons/icons/discard.png"));

    s->assignProperty(_ui->btn_download, "enabled", true);
    s->assignProperty(_ui->btn_upload, "enabled", true);
    s->assignProperty(_ui->btn_reset_to_factory_defaults, "enabled", true);
    s->assignProperty(_ui->checkBox_factory_area, "enabled", true);
    s->assignProperty(_ui->actionCalibrate_ADC, "enabled", true);
    s->assignProperty(_ui->actionFOTA_Uploader, "enabled", true);
    s->assignProperty(this, "configuringAllowed", true);
    s->assignProperty(_communicator, "fotaActive", false);

    s->addTransition(_ui->btn_ConfigurationMode, SIGNAL(clicked()), _states.at(S_FromConfiguringIdleToChannelOpened));
    s->addTransition(communicator, SIGNAL(snlNoResponseFromTargetDevice()), _states.at(S_ChannelOpened));
    s->addTransition(_ui->btn_upload, SIGNAL(clicked()), _states.at(S_DeviceDBVersionChecking));
    s->addTransition(_ui->btn_download, SIGNAL(clicked()), _states.at(S_Downloading));
    s->addTransition(_ui->btn_reset_to_factory_defaults, SIGNAL(clicked()), _states.at(S_Reset));
    s->addTransition(_ui->btn_saveAndRestart, SIGNAL(clicked()), _states.at(S_Saving));
    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));

    /* before closing set the device to DISARMED */
    s->addTransition(qApp, SIGNAL(lastWindowClosed()), _states.at(S_FromConfiguringIdleToChannelOpened));

    KeysManager *km = qobject_cast<KeysManager *>(_ui->tabWidget->widget(MWT_Keys));
    if (km)
        s->addTransition(km, SIGNAL(snlRequestKeyFromDevice()), _states.at(S_ReadKeyFromETR));

    connect(s, &QState::propertiesAssigned, communicator, &AbstractCommunicator::activateConnectionChecking);
    connect(s, &QState::exited, communicator, &AbstractCommunicator::deactivateConnectionChecking);



    // -- CONFIGURING IN PROGRESS (parent for all following states)
    s = _states.at(S_ConfiguringInProgress);

    s->assignProperty(communicator, "processingCanceled", false);

    s->assignProperty(_ui->btn_ConfigurationMode, "enabled", false);

    s->assignProperty(_ui->btn_download, "enabled", false);
    s->assignProperty(_ui->btn_upload, "enabled", false);
    s->assignProperty(_ui->btn_reset_to_factory_defaults, "enabled", false);
    s->assignProperty(_ui->checkBox_factory_area, "enabled", false);
    s->assignProperty(this, "configuringAllowed", false);

    s->addTransition(communicator, SIGNAL(snlNoResponseFromTargetDevice()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlChannelClosed()), _states.at(S_Init));

    /* before closing set the device to DISARMED */
    s->addTransition(qApp, SIGNAL(lastWindowClosed()), _states.at(S_FromConfiguringIdleToChannelOpened));

    connect(s, &QState::exited, _progressdialog, &ProgressDialog::reset);   // hide progress dialog


    // -- Device DB version checking
    s = _states.at(S_DeviceDBVersionChecking);

    s->assignProperty(_ui->btn_saveAndRestart, "enabled", false);

    s->addTransition(communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_Uploading));
    s->addTransition(communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::entered, this, &MainWindow::stCheckDeviceDbVersion);


    // -- UPLOADING
    s = _states.at(S_Uploading);

    s->addTransition(communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_UploadedOk));
    s->addTransition(communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::entered, this, &MainWindow::stUploadSettings);
    connect(s, &QState::exited, communicator, &AbstractCommunicator::clearTxBuffer);  // clear TX buffer


    // -- UPLOADED OK
    s = _states.at(S_UploadedOk);

    s->assignProperty(_ui->btn_saveAndRestart, "enabled", true);

    s->addTransition(s, SIGNAL(propertiesAssigned()), _states.at(S_ConfiguringIdle));


    // -- UPLOADING FOTA
    s = _states.at(S_UploadingFota);

    s->assignProperty(_communicator, "fotaActive", true);

    s->addTransition(communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_ChannelOpened));
    s->addTransition(communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::exited, communicator, &AbstractCommunicator::clearTxBuffer);


    // --
    s = _states.at(S_reserved);


    // -- DOWNLOADING
    s = _states.at(S_Downloading);

    s->addTransition(communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_DownloadedOk));
    s->addTransition(communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::propertiesAssigned, communicator, &AbstractCommunicator::clearRxBuffer); // clear RX buffer
    connect(s, &QState::propertiesAssigned, communicator, &AbstractCommunicator::beginDownloading);


    // -- DOWNLOADED OK
    s = _states.at(S_DownloadedOk);

    s->addTransition(s, SIGNAL(propertiesAssigned()), _states.at(S_ConfiguringIdle));


    // -- RESET
    s = _states.at(S_Reset);

    s->addTransition(_communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_ConfiguringIdle));
    s->addTransition(_communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::entered, _communicator, &AbstractCommunicator::resetToDefaults);


    // -- READ KEY FROM ETR
    s = _states.at(S_ReadKeyFromETR);

    s->addTransition(_communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_ConfiguringIdle));
    s->addTransition(_communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::entered, _communicator, &AbstractCommunicator::requestKeyFromETR);


    // --
    s = _states.at(S_Reserved);




    // -- SAVING
    s = _states.at(S_Saving);

    s->assignProperty(_ui->btn_saveAndRestart, "enabled", false);

    s->addTransition(_communicator, SIGNAL(snlProcessingSucceeded()), _states.at(S_SavedOk));
    s->addTransition(_communicator, SIGNAL(snlProcessingInterrupted()), _states.at(S_ConfiguringIdle));

    connect(s, &QState::entered, _communicator, &AbstractCommunicator::saveUploadedSettings);

    // -- SAVED OK
    s = _states.at(S_SavedOk);

    s->addTransition(s, SIGNAL(propertiesAssigned()), _states.at(S_FromConfiguringIdleToChannelOpened));


    // -- SAVING FAILED
    s = _states.at(S_SavingFailed);



    // -- FINAL
    _stateFinal = new QFinalState(_machine);

    s->addTransition(qApp, SIGNAL(aboutToQuit()), _stateFinal);


    _machine->setInitialState(_states.at(S_Init));
    _machine->start();
}

void MainWindow::setupDatabase(const QString &dbfilename)
{
    qx::QxSqlDatabase::getSingleton()->setTraceSqlQuery(false);

    qx::QxSqlDatabase::getSingleton()->setDriverName("QSQLITE");
    qx::QxSqlDatabase::getSingleton()->setHostName("localhost");
    qx::QxSqlDatabase::getSingleton()->setDatabaseName(dbfilename);
    qx::QxSqlDatabase::getSingleton()->setUserName("root");
    qx::QxSqlDatabase::getSingleton()->setPassword("");

    /* update db name for current connection */
    QSqlDatabase db = qx::QxSqlDatabase::getDatabase();
    if (db.isOpen())
        db.close();
    db.setDatabaseName(dbfilename);
    if (!db.open()) {
        qDebug() << "DB Connection opening error: " << db.lastError().text();
    }
}

bool MainWindow::overwriteLocalDatabase(const QByteArray &data)
{
    QString dbfilename = QApplication::applicationDirPath() + QDir::separator() + QString(LOCAL_DB_FILE_PATH);

    QFile dbfile(dbfilename);
    bool success = dbfile.open(QIODevice::WriteOnly|QIODevice::Truncate)
            && (dbfile.write(data) == data.size());
    dbfile.close();

    setupDatabase(dbfilename);
    Q_EMIT snlLocalDatabaseChanged();

    return success;
}

void MainWindow::replaceCommunicator(AbstractCommunicator *communicator)
{
//    qDebug() << communicator;

    if (_communicator)
        _communicator->deleteLater();

    _communicator = communicator;

    initStateMachine(communicator);

    communicator->setLogConsole(_logconsole);
    _ui->cbox_serial_ports->clear();
    _ui->cbox_serial_ports->addItems(communicator->availableComPorts());

    connect(_ui->checkBox_factory_area, &QCheckBox::toggled,
            _communicator, &AbstractCommunicator::setApplyToFactoryArea);

    connect(_progressdialog, &QProgressDialog::canceled,
            communicator, static_cast<void (AbstractCommunicator::*)()>(
                &AbstractCommunicator::setProcessingCanceled)
                );

    connect(communicator, &AbstractCommunicator::snlProcessingStarted,
            _progressdialog, &ProgressDialog::stInitProgressDialog);
    connect(communicator, &AbstractCommunicator::snlIterationProcessed,
            _progressdialog, &ProgressDialog::stUpdateProgressDialog);

    connect(communicator, &AbstractCommunicator::snlDownloadingComplete,
            this, &MainWindow::stReplaceDatabaseFile);

    connect(communicator, &AbstractCommunicator::snlTxData,
            _ledindicator, &LedIndicator::indicateTx);
    connect(communicator, &AbstractCommunicator::snlRxData,
            _ledindicator, &LedIndicator::indicateRx);
}

void MainWindow::updateWindowTitle(const QString &modeName)
{
    setWindowTitle(qApp->applicationName() + QString(" - ") + modeName);
}

void MainWindow::changeLanguage(QAction *action)
{
    static QAction *lastAction = 0;
    if (lastAction && action != lastAction) {
        if (QMessageBox::Cancel == QMessageBox::information(this,
                                 tr("Change language"),
                                 tr("The application language will be changed\n"
                                    "after the application restart"),
                                 QMessageBox::Ok|QMessageBox::Cancel,
                                 QMessageBox::Cancel))
        {
            lastAction->setChecked(true);
            return;
        }

        if (close()) QProcess::startDetached(qApp->applicationFilePath());
    }
    lastAction = action;
}

void MainWindow::restoreSettings()
{
    QSettings s(QApplication::applicationDirPath() + QDir::separator() + QString("settings.ini"), QSettings::IniFormat);
    restoreGeometry(s.value("mw").toByteArray());
    //setWindowState(static_cast<Qt::WindowState>(s.value("mwstate").toInt()));
    _logconsole->setWindowGeometry(s.value("log").toByteArray());
    _logconsole->setLogAsHex(s.value("ashex").toBool());

    // channel settings
    _ui->le_proxy_host->setText(s.value("ph").toString());
    _ui->sbox_proxy_port->setValue(s.value("pp").toInt());
    _ui->le_device_ip->setText(s.value("di").toString());
    _ui->sbox_device_port->setValue(s.value("dp").toInt());

    // language
    if (0 == s.value("lang", "en").toString().compare("ru"))
        _ui->actionRussian->trigger();
    else
        _ui->actionEnglish->trigger();
}

void MainWindow::saveSettings()
{
    QSettings s(QApplication::applicationDirPath() + QDir::separator() + QString("settings.ini"), QSettings::IniFormat);
    s.setValue("mw", saveGeometry());
    //s.setValue("mwstate", static_cast<int>(windowState()));
    s.setValue("log", _logconsole->saveGeometry());
    s.setValue("ashex", _logconsole->isLogAsHex());

    // channel settings
    s.setValue("ph", _ui->le_proxy_host->text());
    s.setValue("pp", _ui->sbox_proxy_port->value());
    s.setValue("di", _ui->le_device_ip->text());
    s.setValue("dp", _ui->sbox_device_port->value());

    // language
    s.setValue("lang", _ui->actionRussian->isChecked() ? "ru" : "en");
}

inline bool MainWindow::isModified() const
{
    bool ismodified = false;

    ismodified = ismodified || CommonSettingsManager::isModified();
    ismodified = ismodified || SystemBoardManager::isModified();
    ismodified = ismodified || ExpandersManager::isModified();

    ismodified = ismodified || LoopsManager::isModified();
    ismodified = ismodified || RelaysManager::isModified();
    ismodified = ismodified || LedsManager::isModified();
    ismodified = ismodified || BellsManager::isModified();
    ismodified = ismodified || ButtonsManager::isModified();

    ismodified = ismodified || EtrManager::isModified();

    ismodified = ismodified || KeysManager::isModified();
    ismodified = ismodified || ArmingGroupsManager::isModified();

    ismodified = ismodified || EventsManager::isModified();
    ismodified = ismodified || ReactionsManager::isModified();
    ismodified = ismodified || BehaviorPresetsManager::isModified();

    return ismodified;
}

bool MainWindow::isCorrectDbFileVersion(const QString &dbname)
{
    bool bCorrectVersion = false;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbname);
        if (!db.open())
            return false;
        QString col_name("settings_ident_string");
        QSqlQuery query_db_version(QString("SELECT %1 FROM t_SystemInfo").arg(col_name));
        if (query_db_version.exec() && query_db_version.first())
        {
            const QString db_version_str(query_db_version.value(col_name).toString());
            bCorrectVersion = 0 == db_version_str.compare(DB_STRUCTURE_VERSION, Qt::CaseInsensitive);
        }
    }

    QString connName = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(connName);
    return bCorrectVersion;
}

//bool MainWindow::isCorrectDownloadedDbVersion(const QByteArray &data)
//{
//    if (!data.startsWith("SQLite format 3")) {
//        QMessageBox::critical(this,
//                              tr("Downloading"),
//                              tr("Downloaded data is not valid database file"));
//        return false;
//    }

//    QString tempname("./downloaded~");
//    QFile f(tempname);
//    if (!f.open(QIODevice::WriteOnly|QIODevice::Truncate) ||
//            !(f.write(data) == data.size()))
//        return false;
//    f.close();

//    bool bCorrectVersion = isCorrectDbFileVersion(tempname);

//    if (!bCorrectVersion) {
//        QMessageBox::critical(this,
//                              tr("Downloading"),
//                              tr("Bad database version\n"
//                                 "Aborted"));
//    }

//    f.remove();
//    return bCorrectVersion;
//}

bool MainWindow::importSettingsFile(const QString &sourcefilename)
{
    if (!QFile::exists(sourcefilename) || sourcefilename.isEmpty())
        return false;

    // -- prevent importing the working DB file
    QString localDatabaseFilename = QApplication::applicationDirPath() + QDir::separator() + QString(LOCAL_DB_FILE_PATH);
    QString working_fullPath(QFileInfo(localDatabaseFilename).absoluteFilePath());
    QString source_fullPath(QFileInfo(sourcefilename).absoluteFilePath());
    if (0 == working_fullPath.compare(source_fullPath, Qt::CaseInsensitive)) {
        qDebug() << "It's the same file. Operation aborted.";
        return true;
    }

    QFile sourcefile(sourcefilename);
    if (!sourcefile.open(QFile::ReadOnly)) {
        QMessageBox::critical(this,
                              tr("Import settings"),
                              tr("Can't open database file: %1").arg(sourcefilename));
        return false;
    }

    QByteArray arr;

    if (sourcefilename.endsWith(".db", Qt::CaseInsensitive))     // import from Sqlite db file
    {
        if (!isCorrectDbFileVersion(sourcefilename)) {
            QMessageBox::critical(this,
                                  tr("Import settings"),
                                  tr("Bad database version\n"
                                     "Aborted"));
            return false;
        }

        arr = sourcefile.readAll();
        sourcefile.close();

        if (!arr.startsWith("SQLite format 3")) {
            QMessageBox::critical(this,
                                  tr("Import settings"),
                                  tr("The file is not valid database file"));
            return false;
        }
    }
    else if (sourcefilename.endsWith(".xml", Qt::CaseInsensitive))   // import from XML file
    {
        QString localDatabaseFilename = QApplication::applicationDirPath() + QDir::separator() + QString(LOCAL_DB_FILE_PATH);
        XmlSettingsProvider p(localDatabaseFilename, this);
        try {
            if (!p.importSettings(sourcefilename, &arr))
                return false;
        }
        catch (const XMLImportError &e) {
            QMessageBox::critical(this,
                                  tr("Import settings"),
                                  tr(e.what()));
            return false;
        }
    }

    if (!overwriteLocalDatabase(arr)) {
        QMessageBox::critical(this,
                              tr("Import settings"),
                              tr("Can't update local database"));
        return false;
    }

    return true;
}

void MainWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
//        QWindowStateChangeEvent *sc_event = static_cast<QWindowStateChangeEvent *>(e);

        // hook: manipulate with the behavior of the log window

#if defined(Q_OS_WIN)
        if (isMinimized()) {
            _logconsole->hide();
        }
        else if (isActiveWindow()) {
            _logconsole->showIfNeeded();
            activateWindow();
        }
#else   // !defined(Q_OS_WIN)
    ;
#endif

    }

    QMainWindow::changeEvent(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!isModified() || (QMessageBox::Yes == QMessageBox::warning(this,
                                                     tr("Quit"),
                                                     tr("You have one or more unsaved changes.\n"
                                                     "All unsaved changes will be lost.\n\n"
                                                     "Do you want to quit?"),
                                                     QMessageBox::Yes|QMessageBox::No,
                                                     QMessageBox::No
                                                     )))
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}
