#include "precompiled.h"

#include "loopsmanager.h"
#include "expandersmanager.h"

#include "ui_protectionloopsmanager.h"

#include "bo/t_systemboard.h"

#include "bo/s_parentunit.h"

#include "configurator_protocol.h"

#include <QTimer>

#include <QxMemLeak.h>


LoopsManager::LoopsManager(QWidget *parent) :
    ManagerWidget(parent),
    _ui(new Ui::ProtectionLoopsManager())
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << 180 << 100);
    _ui->splitter->setStretchFactor(1, 1);

    _ui->le_alias->setMaxLength(ALIAS_LEN);


    QStringList zonetypes;
    zonetypes << tr("GENERAL") << tr("FIRE") << tr("ENTRY DELAY") << tr("WALK THROUGH")
              << tr("24 HOURS") << tr("PARTIAL ARMING") << tr("PANIC") << tr("TRACK EVENT");
    _ui->cmbx_loop_type->addItems(zonetypes);


    connect(_ui->listWidget_main_list, &QListWidget::currentRowChanged,
            this, &LoopsManager::sltUpdateUI);

    connect(_ui->chbx_enabled,       &QCheckBox::clicked, this, &LoopsManager::sltUpdateUI);
    connect(_ui->chbx_damage_notify, &QCheckBox::clicked, this, &LoopsManager::sltUpdateUI);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &LoopsManager::sltUpdateUI);
    connect(_ui->le_ID,    &QLineEdit::textEdited, this, &LoopsManager::sltUpdateUI);

    connect(_ui->cmbx_group, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &LoopsManager::sltUpdateUI);
    connect(_ui->cmbx_loop_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &LoopsManager::sltUpdateUI);

    connect(_ui->btn_apply,  &QPushButton::clicked, this,  &LoopsManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this,  &LoopsManager::sltCancel);

    setModifiedHook(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

LoopsManager::~LoopsManager()
{ delete _ui; }

void LoopsManager::setModifiedHook(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);
}

void LoopsManager::fillFormHook(type_ptr_const_ref pElement)
{
    _ui->lbl_parent_unit->setText(s_ParentUnit::getParentDescription(pElement->_pParent));
    _ui->lbl_suin->setText(QString::number(pElement->_suin));

    _ui->chbx_enabled->setChecked(pElement->_bEnabled);
    _ui->chbx_damage_notify->setChecked(pElement->_bDamageNotificationEnabled);

    _ui->le_alias->setText(pElement->_alias);
    _ui->le_ID->setText(QString::number(pElement->_humanizedId));

    _ui->cmbx_loop_type->setCurrentIndex(pElement->_zoneType);

    setGroupAtCombobox(pElement);
}

void LoopsManager::clearForm()
{
    _ui->lbl_parent_unit->clear();
    _ui->lbl_suin->clear();

    _ui->chbx_enabled->setChecked(false);
    _ui->chbx_damage_notify->setChecked(false);

    _ui->le_alias->clear();
    _ui->le_ID->clear();

    _ui->cmbx_loop_type->setCurrentIndex(-1);
    _ui->cmbx_group->setCurrentIndex(-1);
}

void LoopsManager::saveForm(AbstractManager::type_ptr_ref pElement)
{
    pElement->_bEnabled = _ui->chbx_enabled->isChecked();
    pElement->_bDamageNotificationEnabled = _ui->chbx_damage_notify->isChecked();

    pElement->_alias = _ui->le_alias->text();
    pElement->_humanizedId = _ui->le_ID->text().toInt();

    pElement->_zoneType = _ui->cmbx_loop_type->currentIndex();

    pElement->_arming_group = getGroupFromCombobox();
}

QListWidget *LoopsManager::mainListWidget()
{ return _ui->listWidget_main_list; }

QObject *LoopsManager::sender() const
{ return QWidget::sender(); }


t_ArmingGroup_ptr LoopsManager::getGroupFromCombobox()
{
    QVariant ret = _ui->cmbx_group->itemData(_ui->cmbx_group->currentIndex(),
                                                       DataRole);
    if (ret.isValid())
        return ret.value<t_ArmingGroup_ptr>();
    else
        return t_ArmingGroup_ptr();
}

void LoopsManager::setGroupAtCombobox(type_ptr_const_ref pElement)
{
    int index = -1;
    if (!pElement->_arming_group.isNull())
        index = _ui->cmbx_group->findData(QVariant::fromValue(pElement->_arming_group->_id),
                                                    IdRole);
    _ui->cmbx_group->setCurrentIndex(index);
}

void LoopsManager::updateAvailableGroups()
{
    t_ArmingGroupX  groups_list;
    qx::dao::fetch_all(groups_list);
    _ui->cmbx_group->clear();

    t_ArmingGroup_ptr pGroup;
    _foreach(pGroup, groups_list) {
        _ui->cmbx_group->addItem(pGroup->_alias);

        // -- set data to the last added item
        int index = _ui->cmbx_group->count() - 1;
        _ui->cmbx_group->setItemData(index, QVariant::fromValue(pGroup->_id), IdRole);
        _ui->cmbx_group->setItemData(index, QVariant::fromValue(pGroup),      DataRole);
    }
}

void LoopsManager::sltUpdateList()
{
    updateAvailableGroups();
    updateList(this);
}
