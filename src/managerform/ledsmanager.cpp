#include "precompiled.h"

#include "ledsmanager.h"
#include "ui_ledsmanager.h"

#include "bo/t_expander.h"
#include "bo/t_systemboard.h"
#include "bo/t_etr.h"

#include "bo/s_parentunit.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


LedsManager::LedsManager(QWidget *parent) :
    ManagerWidget(parent),
    _ui(new Ui::LedsManager())
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << 180 << 100);
    _ui->splitter->setStretchFactor(1, 1);

    _ui->le_alias->setMaxLength(ALIAS_LEN);

    connect(_ui->listWidget_main_list, &QListWidget::currentRowChanged,
            this, &LedsManager::sltUpdateUI);

    connect(_ui->chbx_enabled, &QCheckBox::clicked, this, &LedsManager::sltUpdateUI);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &LedsManager::sltUpdateUI);
    connect(_ui->le_ID,    &QLineEdit::textEdited, this, &LedsManager::sltUpdateUI);

    connect(_ui->btn_apply,  &QPushButton::clicked, this,  &LedsManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this,  &LedsManager::sltCancel);

    connect(_ui->chbx_arming_led, &QCheckBox::clicked, this, &LedsManager::sltUpdateUI);
    connect(_ui->cmbx_related_group, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &LedsManager::sltUpdateUI);

    setModifiedHook(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

LedsManager::~LedsManager()
{ delete _ui; }

void LedsManager::setModifiedHook(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);
}

void LedsManager::fillFormHook(type_ptr_const_ref pElement)
{
    _ui->lbl_parent_unit->setText(s_ParentUnit::getParentDescription(pElement->_pParent));
    _ui->lbl_suin->setText(QString::number(pElement->_suin));

    _ui->chbx_enabled->setChecked(pElement->_bEnabled);

    _ui->le_alias->setText(pElement->_alias);
    _ui->le_ID->setText(QString::number(pElement->_humanizedId));

    _ui->chbx_arming_led->setChecked(pElement->_bArmingLed);
    actualizeGroupsCombobox();
    setGroupAtCombobox(pElement);
}

void LedsManager::clearForm()
{
    _ui->lbl_parent_unit->clear();
    _ui->lbl_suin->clear();

    _ui->chbx_enabled->setChecked(false);

    _ui->le_alias->clear();
    _ui->le_ID->clear();

    _ui->chbx_arming_led->setChecked(false);
    _ui->cmbx_related_group->setCurrentIndex(-1);
}

void LedsManager::saveForm(AbstractManager::type_ptr_ref pElement)
{
    pElement->_bEnabled = _ui->chbx_enabled->isChecked();
    pElement->_alias = _ui->le_alias->text();
    pElement->_bArmingLed = _ui->chbx_arming_led->isChecked();
    pElement->_pGroup = getGroupFromCombobox();
}

QListWidget *LedsManager::mainListWidget()
{ return _ui->listWidget_main_list; }

QObject *LedsManager::sender() const
{ return QWidget::sender(); }

void LedsManager::updateGroupsCombobox()
{
    t_ArmingGroupX  groups_list;
    qx::dao::fetch_all(groups_list);
    _ui->cmbx_related_group->clear();

    t_ArmingGroup_ptr pGroup;
    _foreach(pGroup, groups_list) {
        _ui->cmbx_related_group->addItem(pGroup->_alias);

        // -- set data to the last added item
        int index = _ui->cmbx_related_group->count() - 1;
        _ui->cmbx_related_group->setItemData(index, QVariant::fromValue(pGroup->_id), IdRole);
        _ui->cmbx_related_group->setItemData(index, QVariant::fromValue(pGroup),      DataRole);
    }
}

void LedsManager::setGroupAtCombobox(type_ptr_const_ref pElement)
{
    int index = -1;
    if (!pElement->_pGroup.isNull())
        index = _ui->cmbx_related_group->findData(QVariant::fromValue(pElement->_pGroup->_id),
                                                    IdRole);
    _ui->cmbx_related_group->setCurrentIndex(index);
}

t_ArmingGroup_ptr LedsManager::getGroupFromCombobox()
{
    QVariant ret = _ui->cmbx_related_group->itemData(_ui->cmbx_related_group->currentIndex(),
                                                       DataRole);
    return ret.isValid() ? ret.value<t_ArmingGroup_ptr>() : t_ArmingGroup_ptr();
}

void LedsManager::actualizeGroupsCombobox()
{
    if (_ui->chbx_arming_led->isChecked()) {
        _ui->cmbx_related_group->setEnabled(true);
        _ui->cmbx_related_group->setCurrentIndex(0);
    } else {
        _ui->cmbx_related_group->setEnabled(false);
        _ui->cmbx_related_group->setCurrentIndex(-1);
    }
}

void LedsManager::sltUpdateList()
{
    updateGroupsCombobox();
    updateList(this);
}

void LedsManager::sltUpdateUI()
{
     updateUI();
     if (sender() == _ui->chbx_arming_led)
         actualizeGroupsCombobox();
}
