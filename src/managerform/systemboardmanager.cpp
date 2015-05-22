#include "precompiled.h"

#include "systemboardmanager.h"
#include "ui_systemboardmanager.h"

#include "bo/s_parentunit.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>



bool SystemBoardManager::_bModified = false;


SystemBoardManager::SystemBoardManager(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::SystemBoardManager()),
    _system_board(new t_SystemBoard())
{
    _ui->setupUi(this);

    _ui->le_alias->setMaxLength(ALIAS_LEN);
    _ui->le_uin->setValidator(new QIntValidator(1, 65535, this));

    /* NOTE: disable "UIN" field in the configurator UI but entire related functionality is retained */
    _ui->label_2->setVisible(false);
    _ui->le_uin->setVisible(false);

    _system_board->_id = 1;

    _relation << "uin_id";

    connect(_ui->btn_apply,  &QPushButton::clicked, this, &SystemBoardManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this, &SystemBoardManager::sltCancel);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &SystemBoardManager::sltUpdateUI);
    connect(_ui->le_uin,   &QLineEdit::textEdited, this, &SystemBoardManager::sltUpdateUI);

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateData()));
}

SystemBoardManager::~SystemBoardManager()
{
    delete _ui;
}

bool SystemBoardManager::isModified()
{
    return _bModified;
}

void SystemBoardManager::setModified(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);

    _bModified = modified;
}

void SystemBoardManager::fillForm()
{
    _ui->le_alias->setText(_system_board->_uin->_palias);
    _ui->le_uin->setText(QString::number(_system_board->_uin->_puin));

    setModified(false);
    _ui->le_alias->setSelection(0,0);
}

void SystemBoardManager::saveForm()
{
    _system_board->_uin->_palias = _ui->le_alias->text();
    _system_board->_uin->_puin = _ui->le_uin->text().toInt();
}

void SystemBoardManager::sltUpdateData()
{
    qx::dao::fetch_by_id_with_relation(_relation, _system_board);
    fillForm();
}

void SystemBoardManager::sltUpdateUI()
{
    setModified(true);
}

void SystemBoardManager::sltApply()
{
    saveForm();
    qx::dao::update_with_relation(_relation, _system_board);
    fillForm();
    Q_EMIT snlDataChanged();
}

void SystemBoardManager::sltCancel()
{
    fillForm();
}
