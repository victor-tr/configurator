#include "precompiled.h"
#include "commonsettingsmanager.h"
#include "bo/t_auxphone.h"
#include "bo/t_ipaddress.h"
#include "bo/t_simcard.h"

#include "ui_commonsettingsmanager.h"

#include "configurator_protocol.h"

#include <QTimer>
#include <QPushButton>
#include <QxMemLeak.h>


bool CommonSettingsManager::_bModified = false;


CommonSettingsManager::CommonSettingsManager(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::CommonSettingsManager())
{
    _ui->setupUi(this);

    // NOTE: disable the checkboxes "Allow SMS" at Common Settings tab
    _ui->chbx_sim1_allow_SMS->setEnabled(false);
    _ui->chbx_sim2_allow_SMS->setEnabled(false);

    // -- setup input data validation
//    QRegExpValidator ip_validator(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"), this);
    QRegExp ip_regexp("\\b(([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\b");
    QRegExpValidator *ip_validator = new QRegExpValidator(ip_regexp, this);
    _ui->le_ip1->setValidator(ip_validator);
    _ui->le_ip1_2->setValidator(ip_validator);
    _ui->le_ip2->setValidator(ip_validator);
    _ui->le_ip2_2->setValidator(ip_validator);

    QRegExp phone_regexp("(\\+380)?\\d{9}|\\d{3,10}");
    QRegExpValidator *phone_validator = new QRegExpValidator(phone_regexp, this);
    _ui->le_dtmf_phone1->setValidator(phone_validator);
    _ui->le_dtmf_phone2->setValidator(phone_validator);
    _ui->le_dtmf_phone1_2->setValidator(phone_validator);
    _ui->le_dtmf_phone2_2->setValidator(phone_validator);
    _ui->le_aux_phone_1->setValidator(phone_validator);
    _ui->le_aux_phone_2->setValidator(phone_validator);
    _ui->le_aux_phone_3->setValidator(phone_validator);
    _ui->le_aux_phone_4->setValidator(phone_validator);
    _ui->le_aux_phone_5->setValidator(phone_validator);

    _ui->le_apn_1->setMaxLength(APN_LEN);
    _ui->le_apn_2->setMaxLength(APN_LEN);

    _ui->le_login_sim1->setMaxLength(CREDENTIALS_LEN);
    _ui->le_login_sim2->setMaxLength(CREDENTIALS_LEN);
    _ui->le_password_sim1->setMaxLength(CREDENTIALS_LEN);
    _ui->le_password_sim2->setMaxLength(CREDENTIALS_LEN);


    // --
    _ui->cmbox_debugLevel->addItems(QStringList() << "---" << "1" << "2" << "3" << "4" << "5" <<
                                    "6" << "7" << "8");

    _ui->cmbox_dtmfTxRate->addItems(QStringList()
                                    << DTMF_TX_RATE_SLOW
                                    << DTMF_TX_RATE_MID
                                    << DTMF_TX_RATE_FAST);

    connect(_ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(sltApply()));
    connect(_ui->buttonBox, SIGNAL(rejected()), this, SLOT(sltCancel()));

    // -- connect the CommonSettings object's fields
    connect(_ui->chbx_aux_phone1_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone1_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone2_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone2_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone3_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone3_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone4_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone4_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone5_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_aux_phone5_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));

    connect(_ui->le_aux_phone_1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_aux_phone_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_aux_phone_3, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_aux_phone_4, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_aux_phone_5, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));

    connect(_ui->chbx_sim1_allow_SMS, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_sim2_allow_SMS, SIGNAL(clicked()), SLOT(sltUpdateUI()));

    connect(_ui->cmbox_debugLevel, SIGNAL(activated(int)), SLOT(sltUpdateUI()));
    connect(_ui->cmbox_codec, SIGNAL(activated(int)), SLOT(sltUpdateUI()));
    connect(_ui->cmbox_dtmfTxRate, SIGNAL(activated(int)), SLOT(sltUpdateUI()));

    connect(_ui->groupBox_sim1, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->groupBox_sim2, SIGNAL(clicked()), SLOT(sltUpdateUI()));

    connect(_ui->le_apn_1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_apn_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_dtmf_phone1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_dtmf_phone2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_dtmf_phone1_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_dtmf_phone2_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_ip1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_ip2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_ip1_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_ip2_2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_login_sim1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_login_sim2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_password_sim1, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_password_sim2, SIGNAL(textChanged(QString)), SLOT(sltUpdateUI()));

    connect(_ui->rdbtn_sim1_prefer_DTMF, SIGNAL(toggled(bool)), SLOT(sltUpdateUI()));
    connect(_ui->rdbtn_sim1_prefer_GPRS, SIGNAL(toggled(bool)), SLOT(sltUpdateUI()));
    connect(_ui->rdbtn_sim2_prefer_DTMF, SIGNAL(toggled(bool)), SLOT(sltUpdateUI()));
    connect(_ui->rdbtn_sim2_prefer_GPRS, SIGNAL(toggled(bool)), SLOT(sltUpdateUI()));

    connect(_ui->spinBox_dtmf_attempts, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    connect(_ui->spinBox_dtmf_attempts_2, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->spinBox_entry_delay, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    connect(_ui->spinBox_arming_delay, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->spinBox_gprs_attempts, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    connect(_ui->spinBox_gprs_attempts_2, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->spinBox_port_sim1, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    connect(_ui->spinBox_port_sim2, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->spinBox_localport_sim1, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    connect(_ui->spinBox_localport_sim2, SIGNAL(valueChanged(int)), SLOT(sltUpdateUI()));
    //

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateData()));
}

CommonSettingsManager::~CommonSettingsManager()
{
    delete _ui;
}

bool CommonSettingsManager::isModified()
{
    return _bModified;
}

void CommonSettingsManager::fillForm()
{
    if (_common_settings->_simcards_list.size() >= 2)  // UI form have 2 SIM cards
    {
        // --------------------------------------------------------------------------------
        // SIM1
        t_SimCard_ptr sc1 = _common_settings->_simcards_list.getByKey(1);

        _ui->groupBox_sim1->setChecked(sc1->_usable);

        if (sc1->_prefer_gprs)
            _ui->rdbtn_sim1_prefer_GPRS->setChecked(true);
        else
            _ui->rdbtn_sim1_prefer_DTMF->setChecked(true);

        _ui->chbx_sim1_allow_SMS->setChecked(sc1->_allow_sms);
        _ui->spinBox_dtmf_attempts->setValue(sc1->_voicecall_attempts_qty);
        _ui->spinBox_gprs_attempts->setValue(sc1->_gprs_attempts_qty);

        _ui->spinBox_port_sim1->setValue(sc1->_udp_dest_port);
        _ui->spinBox_localport_sim1->setValue(sc1->_udp_local_port);

        _ui->le_apn_1->setText(sc1->_apn);
        _ui->le_login_sim1->setText(sc1->_login);
        _ui->le_password_sim1->setText(sc1->_password);

        for (int i = 0; i < sc1->_phoneNumbers_list.size(); ++i) {
            t_Phone_ptr number = sc1->_phoneNumbers_list.getByIndex(i);
            switch (number->_idx) {
            case 1:
                _ui->le_dtmf_phone1->setText(number->_phone);
                break;
            case 2:
                _ui->le_dtmf_phone2->setText(number->_phone);
                break;
            }
        }

        for (int i = 0; i < sc1->_ipAddresses_list.size(); ++i) {
            t_IpAddress_ptr address = sc1->_ipAddresses_list.getByIndex(i);
            switch (address->_idx) {
            case 1:
                _ui->le_ip1->setText(address->_ip);
                break;
            case 2:
                _ui->le_ip2->setText(address->_ip);
                break;
            }
        }

        // --------------------------------------------------------------------------------
        // SIM2
        t_SimCard_ptr sc2 = _common_settings->_simcards_list.getByKey(2);

        _ui->groupBox_sim2->setChecked(sc2->_usable);

        if (sc2->_prefer_gprs)
            _ui->rdbtn_sim2_prefer_GPRS->setChecked(true);
        else
            _ui->rdbtn_sim2_prefer_DTMF->setChecked(true);

        _ui->chbx_sim2_allow_SMS->setChecked(sc2->_allow_sms);
        _ui->spinBox_dtmf_attempts_2->setValue(sc2->_voicecall_attempts_qty);
        _ui->spinBox_gprs_attempts_2->setValue(sc2->_gprs_attempts_qty);

        _ui->spinBox_port_sim2->setValue(sc2->_udp_dest_port);
        _ui->spinBox_localport_sim2->setValue(sc2->_udp_local_port);

        _ui->le_apn_2->setText(sc2->_apn);
        _ui->le_login_sim2->setText(sc2->_login);
        _ui->le_password_sim2->setText(sc2->_password);

        for (int i = 0; i < sc2->_phoneNumbers_list.size(); ++i) {
            t_Phone_ptr number = sc2->_phoneNumbers_list.getByIndex(i);
            switch (number->_idx) {
            case 1:
                _ui->le_dtmf_phone1_2->setText(number->_phone);
                break;
            case 2:
                _ui->le_dtmf_phone2_2->setText(number->_phone);
                break;
            }
        }

        for (int i = 0; i < sc2->_ipAddresses_list.size(); ++i) {
            t_IpAddress_ptr address = sc2->_ipAddresses_list.getByIndex(i);
            switch (address->_idx) {
            case 1:
                _ui->le_ip1_2->setText(address->_ip);
                break;
            case 2:
                _ui->le_ip2_2->setText(address->_ip);
                break;
            }
        }
    }

    // --------------------------------------------------------------------------------
    // common
    t_AuxPhoneX *aux_phones = &_common_settings->_aux_phones_list;

    for (int i = 0; i < aux_phones->size(); ++i) {
        t_AuxPhone_ptr number = aux_phones->getByIndex(i);
        switch (number->_id) {
        case 1:
            _ui->le_aux_phone_1->setText(number->_phone);
            _ui->chbx_aux_phone1_sim1->setChecked(number->_allowed_simcard & 1);
            _ui->chbx_aux_phone1_sim2->setChecked(number->_allowed_simcard & 2);
            break;
        case 2:
            _ui->le_aux_phone_2->setText(number->_phone);
            _ui->chbx_aux_phone2_sim1->setChecked(number->_allowed_simcard & 1);
            _ui->chbx_aux_phone2_sim2->setChecked(number->_allowed_simcard & 2);
            break;
        case 3:
            _ui->le_aux_phone_3->setText(number->_phone);
            _ui->chbx_aux_phone3_sim1->setChecked(number->_allowed_simcard & 1);
            _ui->chbx_aux_phone3_sim2->setChecked(number->_allowed_simcard & 2);
            break;
        case 4:
            _ui->le_aux_phone_4->setText(number->_phone);
            _ui->chbx_aux_phone4_sim1->setChecked(number->_allowed_simcard & 1);
            _ui->chbx_aux_phone4_sim2->setChecked(number->_allowed_simcard & 2);
            break;
        case 5:
            _ui->le_aux_phone_5->setText(number->_phone);
            _ui->chbx_aux_phone5_sim1->setChecked(number->_allowed_simcard & 1);
            _ui->chbx_aux_phone5_sim2->setChecked(number->_allowed_simcard & 2);
            break;
        }
    }

    qx_query get_codecs("SELECT * FROM s_codecs ORDER BY id ASC");
    QSqlError error = qx::dao::call_query(get_codecs);
    if (!error.isValid()) {
        _ui->cmbox_codec->clear();

        int rows = get_codecs.getSqlResultRowCount();
        for (int i = 0; i < rows; ++i) {
            int idx = get_codecs.getSqlResultAt(i, 0).toInt();
            QString name = get_codecs.getSqlResultAt(i, 1).toString();
            _ui->cmbox_codec->insertItem(idx, name);
        }
    }

    _ui->cmbox_codec->setCurrentIndex(_common_settings->_codecType);
    _ui->spinBox_entry_delay->setValue(_common_settings->_entry_delay_sec);
    _ui->spinBox_arming_delay->setValue(_common_settings->_arming_delay_sec);

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    _ui->cmbox_debugLevel->setCurrentText(QString::number(_common_settings->_debug_level));
#else
    int idx = ui->cmbox_debugLevel->findText(QString::number(_common_settings->_debug_level));
    ui->cmbox_debugLevel->setCurrentIndex(idx);
#endif

    _ui->cmbox_dtmfTxRate->setCurrentIndex(-1);
    _ui->cmbox_dtmfTxRate->setCurrentText(_common_settings->_dtmf_tx_rate);

    // --------------------------------------------------------------------------------
    setModified(false);
}

void CommonSettingsManager::saveForm()
{
    if (_common_settings->_simcards_list.size() >= 2)  // UI form have 2 SIM cards
    {
        // SIM 1
        t_SimCard_ptr sc1 = _common_settings->_simcards_list.getByKey(1);

        sc1->_usable = _ui->groupBox_sim1->isChecked();
        sc1->_allow_sms = _ui->chbx_sim1_allow_SMS->isChecked();
        sc1->_prefer_gprs = _ui->rdbtn_sim1_prefer_GPRS->isChecked();

        sc1->_voicecall_attempts_qty = _ui->spinBox_dtmf_attempts->value();
        sc1->_gprs_attempts_qty = _ui->spinBox_gprs_attempts->value();
        sc1->_udp_iplist_default_size = 2;
        sc1->_tcp_iplist_default_size = 2;
        sc1->_phonelist_default_size = 2;

        sc1->_udp_dest_port = _ui->spinBox_port_sim1->value();
        sc1->_udp_local_port = _ui->spinBox_localport_sim1->value();
        sc1->_tcp_dest_port = 0;
        sc1->_tcp_local_port = 0;

        sc1->_apn = _ui->le_apn_1->text();
        sc1->_login = _ui->le_login_sim1->text();
        sc1->_password = _ui->le_password_sim1->text();

        for (int i = 0; i < sc1->_phoneNumbers_list.size(); ++i) {
            t_Phone_ptr number = sc1->_phoneNumbers_list.getByIndex(i);
            switch (number->_idx) {
            case 1:
                number->_phone = _ui->le_dtmf_phone1->text();
                break;
            case 2:
                number->_phone = _ui->le_dtmf_phone2->text();
                break;
            }
        }

        for (int i = 0; i < sc1->_ipAddresses_list.size(); ++i) {
            t_IpAddress_ptr address = sc1->_ipAddresses_list.getByIndex(i);
            switch (address->_idx) {
            case 1:
                address->_ip = _ui->le_ip1->text();
                break;
            case 2:
                address->_ip = _ui->le_ip2->text();
                break;
            }
        }

        // --------------------------------------------------------------------------------
        // SIM 2
        t_SimCard_ptr sc2 = _common_settings->_simcards_list.getByKey(2);

        sc2->_usable = _ui->groupBox_sim2->isChecked();
        sc2->_allow_sms = _ui->chbx_sim2_allow_SMS->isChecked();
        sc2->_prefer_gprs = _ui->rdbtn_sim2_prefer_GPRS->isChecked();

        sc2->_voicecall_attempts_qty = _ui->spinBox_dtmf_attempts_2->value();
        sc2->_gprs_attempts_qty = _ui->spinBox_gprs_attempts_2->value();
        sc2->_udp_iplist_default_size = 2;
        sc2->_tcp_iplist_default_size = 2;
        sc2->_phonelist_default_size = 2;

        sc2->_udp_dest_port = _ui->spinBox_port_sim2->value();
        sc2->_udp_local_port = _ui->spinBox_localport_sim2->value();
        sc2->_tcp_dest_port = 0;
        sc2->_tcp_local_port = 0;

        sc2->_apn = _ui->le_apn_2->text();
        sc2->_login = _ui->le_login_sim2->text();
        sc2->_password = _ui->le_password_sim2->text();

        for (int i = 0; i < sc2->_phoneNumbers_list.size(); ++i) {
            t_Phone_ptr number = sc2->_phoneNumbers_list.getByIndex(i);
            switch (number->_idx) {
            case 1:
                number->_phone = _ui->le_dtmf_phone1_2->text();
                break;
            case 2:
                number->_phone = _ui->le_dtmf_phone2_2->text();
                break;
            }
        }

        for (int i = 0; i < sc2->_ipAddresses_list.size(); ++i) {
            t_IpAddress_ptr address = sc2->_ipAddresses_list.getByIndex(i);
            switch (address->_idx) {
            case 1:
                address->_ip = _ui->le_ip1_2->text();
                break;
            case 2:
                address->_ip = _ui->le_ip2_2->text();
                break;
            }
        }
    }

    // ---------------------------------------------------------------------------------
    // common
    _common_settings->_id = 1;

    t_AuxPhoneX *aux_phones = &_common_settings->_aux_phones_list;

    for (int i = 0; i < aux_phones->size(); ++i) {
        t_AuxPhone_ptr number = aux_phones->getByIndex(i);
        long access = 0;
        switch (number->_id) {
        case 1:
            number->_phone = _ui->le_aux_phone_1->text();
            access = 0;
            if (_ui->chbx_aux_phone1_sim1->isChecked()) access |= 1;
            if (_ui->chbx_aux_phone1_sim2->isChecked()) access |= 2;
            number->_allowed_simcard = access;
            break;
        case 2:
            number->_phone = _ui->le_aux_phone_2->text();
            access = 0;
            if (_ui->chbx_aux_phone2_sim1->isChecked()) access |= 1;
            if (_ui->chbx_aux_phone2_sim2->isChecked()) access |= 2;
            number->_allowed_simcard = access;
            break;
        case 3:
            number->_phone = _ui->le_aux_phone_3->text();
            access = 0;
            if (_ui->chbx_aux_phone3_sim1->isChecked()) access |= 1;
            if (_ui->chbx_aux_phone3_sim2->isChecked()) access |= 2;
            number->_allowed_simcard = access;
            break;
        case 4:
            number->_phone = _ui->le_aux_phone_4->text();
            access = 0;
            if (_ui->chbx_aux_phone4_sim1->isChecked()) access |= 1;
            if (_ui->chbx_aux_phone4_sim2->isChecked()) access |= 2;
            number->_allowed_simcard = access;
            break;
        case 5:
            number->_phone = _ui->le_aux_phone_5->text();
            access = 0;
            if (_ui->chbx_aux_phone5_sim1->isChecked()) access |= 1;
            if (_ui->chbx_aux_phone5_sim2->isChecked()) access |= 2;
            number->_allowed_simcard = access;
            break;
        }
    }

    _common_settings->_codecType = _ui->cmbox_codec->currentIndex();
    _common_settings->_entry_delay_sec = _ui->spinBox_entry_delay->value();
    _common_settings->_arming_delay_sec = _ui->spinBox_arming_delay->value();
    _common_settings->_debug_level = _ui->cmbox_debugLevel->currentText().toLong();

    _common_settings->_aux_phonelist_default_size = 5;
    _common_settings->_dtmf_tx_rate = _ui->cmbox_dtmfTxRate->currentText();
}

inline void CommonSettingsManager::setModified(bool modified)
{
    _bModified = modified;
    _ui->buttonBox->setEnabled(modified);
}

void CommonSettingsManager::db_update()
{
    QStringList relations;
    relations << "aux_phones_list";
    relations << "simcards_list";
//    relations << "simcards_list->phoneNumbers_list";
//    relations << "simcards_list->ipAddresses_list";

    QSqlError e = qx::dao::update_with_relation(relations, _common_settings);
    if (e.isValid()) {
        qDebug() << e.text();
        return;
    }

    relations.clear();
    relations << "phoneNumbers_list" << "ipAddresses_list";
    e = qx::dao::update_with_relation(relations, _common_settings->_simcards_list);
    if (e.isValid()) {
        qDebug() << e.text();
        return;
    }
}

void CommonSettingsManager::sltUpdateData()
{
    _common_settings = t_CommonSettings_ptr(new t_CommonSettings);
    _common_settings->_id = 1;

    QStringList relations;
    relations << "aux_phones_list";
    relations << "simcards_list->phoneNumbers_list";
    relations << "simcards_list->ipAddresses_list";

    QSqlError e = qx::dao::fetch_by_id_with_relation(relations, _common_settings);
    if (e.isValid()) {
        qDebug() << e.text();
        return;
    }

    Q_ASSERT(_common_settings->_simcards_list.size() == 2);
    Q_ASSERT(_common_settings->_simcards_list.getByIndex(0)->_phoneNumbers_list.size() == 2);
    Q_ASSERT(_common_settings->_simcards_list.getByIndex(0)->_ipAddresses_list.size() == 2);
    Q_ASSERT(_common_settings->_simcards_list.getByIndex(1)->_phoneNumbers_list.size() == 2);
    Q_ASSERT(_common_settings->_simcards_list.getByIndex(1)->_ipAddresses_list.size() == 2);
    Q_ASSERT(_common_settings->_aux_phones_list.size() == 5);

    fillForm();
}

void CommonSettingsManager::sltUpdateUI()
{
    setModified(true);
}

void CommonSettingsManager::sltApply()
{
    saveForm();
    db_update();
    fillForm();
    Q_EMIT snlDataChanged();
}

void CommonSettingsManager::sltCancel()
{
    fillForm();
}
