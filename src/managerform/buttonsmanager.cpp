#include "precompiled.h"

#include "buttonsmanager.h"
#include "ui_buttonsmanager.h"

#include "bo/t_etr.h"
#include "bo/t_systemboard.h"

#include "bo/s_parentunit.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ButtonsManager::ButtonsManager(QWidget *parent) :
    ManagerWidget(parent),
    _ui(new Ui::ButtonsManager())
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << 180 << 100);
    _ui->splitter->setStretchFactor(1, 1);

    _ui->le_alias->setMaxLength(ALIAS_LEN);

    connect(_ui->listWidget_main_list, &QListWidget::currentRowChanged,
            this, &ButtonsManager::sltUpdateUI);

    connect(_ui->chbx_enabled, &QCheckBox::clicked, this, &ButtonsManager::sltUpdateUI);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &ButtonsManager::sltUpdateUI);
    connect(_ui->le_ID,    &QLineEdit::textEdited, this, &ButtonsManager::sltUpdateUI);

    connect(_ui->btn_apply,  &QPushButton::clicked, this,  &ButtonsManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this,  &ButtonsManager::sltCancel);

    setModifiedHook(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

ButtonsManager::~ButtonsManager()
{ delete _ui; }

void ButtonsManager::setModifiedHook(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);
}

void ButtonsManager::fillFormHook(type_ptr_const_ref pElement)
{
    _ui->lbl_parent_unit->setText(s_ParentUnit::getParentDescription(pElement->_pParent));
    _ui->lbl_suin->setText(QString::number(pElement->_suin));

    _ui->chbx_enabled->setChecked(pElement->_bEnabled);

    _ui->le_alias->setText(pElement->_alias);
    _ui->le_ID->setText(QString::number(pElement->_humanizedId));
}

void ButtonsManager::clearForm()
{
    _ui->lbl_parent_unit->clear();
    _ui->lbl_suin->clear();

    _ui->chbx_enabled->setChecked(false);

    _ui->le_alias->clear();
    _ui->le_ID->clear();
}

void ButtonsManager::saveForm(AbstractManager::type_ptr_ref pElement)
{
    pElement->_bEnabled = _ui->chbx_enabled->isChecked();
    pElement->_alias = _ui->le_alias->text();
}

QListWidget *ButtonsManager::mainListWidget()
{ return _ui->listWidget_main_list; }

QObject *ButtonsManager::sender() const
{ return QWidget::sender(); }
