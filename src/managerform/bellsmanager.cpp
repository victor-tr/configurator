#include "precompiled.h"

#include "bellsmanager.h"
#include "ui_bellsmanager.h"

#include "bo/t_expander.h"
#include "bo/t_systemboard.h"

#include "bo/s_parentunit.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


BellsManager::BellsManager(QWidget *parent) :
    ManagerWidget(parent),
    _ui(new Ui::BellsManager())
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << 180 << 100);
    _ui->splitter->setStretchFactor(1, 1);

    _ui->le_alias->setMaxLength(ALIAS_LEN);

    connect(_ui->listWidget_main_list, &QListWidget::currentRowChanged,
            this, &BellsManager::sltUpdateUI);

    connect(_ui->chbx_enabled,        &QCheckBox::clicked, this, &BellsManager::sltUpdateUI);
    connect(_ui->chbx_remote_control, &QCheckBox::clicked, this, &BellsManager::sltUpdateUI);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &BellsManager::sltUpdateUI);
    connect(_ui->le_ID,    &QLineEdit::textEdited, this, &BellsManager::sltUpdateUI);

    connect(_ui->btn_apply,  &QPushButton::clicked, this,  &BellsManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this,  &BellsManager::sltCancel);

    setModifiedHook(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

BellsManager::~BellsManager()
{ delete _ui; }

void BellsManager::setModifiedHook(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);
}

void BellsManager::fillFormHook(type_ptr_const_ref pElement)
{
    _ui->lbl_parent_unit->setText(s_ParentUnit::getParentDescription(pElement->_pParent));
    _ui->lbl_suin->setText(QString::number(pElement->_suin));

    _ui->chbx_enabled->setChecked(pElement->_bEnabled);
    _ui->chbx_remote_control->setChecked(pElement->_bRemoteControl);

    _ui->le_alias->setText(pElement->_alias);
    _ui->le_ID->setText(QString::number(pElement->_humanizedId));
}

void BellsManager::clearForm()
{
    _ui->lbl_parent_unit->clear();
    _ui->lbl_suin->clear();

    _ui->chbx_enabled->setChecked(false);
    _ui->chbx_remote_control->setChecked(false);

    _ui->le_alias->clear();
    _ui->le_ID->clear();
}

void BellsManager::saveForm(AbstractManager::type_ptr_ref pElement)
{
    pElement->_bEnabled = _ui->chbx_enabled->isChecked();
    pElement->_bRemoteControl = _ui->chbx_remote_control->isChecked();
    pElement->_alias = _ui->le_alias->text();
}

QListWidget *BellsManager::mainListWidget()
{ return _ui->listWidget_main_list; }

QObject *BellsManager::sender() const
{ return QWidget::sender(); }
