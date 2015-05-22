#include "precompiled.h"

#include "relaysmanager.h"
#include "expandersmanager.h"

#include "ui_relaysmanager.h"

#include "bo/t_systemboard.h"
#include "bo/t_etr.h"

#include "bo/s_parentunit.h"

#include "configurator_protocol.h"

#include <QTimer>

#include <QxMemLeak.h>


#define LOAD_COMMON_TEXT        QT_TRANSLATE_NOOP("RelaysManager", "Common load")
#define LOAD_ARMING_LED_TEXT    QT_TRANSLATE_NOOP("RelaysManager", "Arming Led")
#define LOAD_BELL_TEXT          QT_TRANSLATE_NOOP("RelaysManager", "Bell")

#define NO_GROUP_TEXT   ""


RelaysManager::RelaysManager(QWidget *parent) :
    ManagerWidget(parent),
    _ui(new Ui::RelaysManager())
{
    _ui->setupUi(this);

    _ui->splitter->setSizes(QList<int>() << 180 << 100);
    _ui->splitter->setStretchFactor(1, 1);

    _ui->le_alias->setMaxLength(ALIAS_LEN);

    QStringList load_types;
    load_types.insert(t_Relay::RelayLoad_Common,    tr(LOAD_COMMON_TEXT));
    load_types.insert(t_Relay::RelayLoad_ArmingLed, tr(LOAD_ARMING_LED_TEXT));
    load_types.insert(t_Relay::RelayLoad_Bell,      tr(LOAD_BELL_TEXT));
    _ui->cmbx_load_type->addItems(load_types);

    connect(_ui->listWidget_main_list, &QListWidget::currentRowChanged,
            this, &RelaysManager::sltUpdateUI);

    connect(_ui->chbx_enabled,        &QCheckBox::clicked, this, &RelaysManager::sltUpdateUI);
    connect(_ui->chbx_remote_control, &QCheckBox::clicked, this, &RelaysManager::sltUpdateUI);
    connect(_ui->chbx_notify_on_state_changed, &QCheckBox::clicked,
            this, &RelaysManager::sltUpdateUI);

    connect(_ui->le_alias, &QLineEdit::textEdited, this, &RelaysManager::sltUpdateUI);
    connect(_ui->le_ID,    &QLineEdit::textEdited, this, &RelaysManager::sltUpdateUI);

    connect(_ui->btn_apply,  &QPushButton::clicked, this,  &RelaysManager::sltApply);
    connect(_ui->btn_cancel, &QPushButton::clicked, this,  &RelaysManager::sltCancel);

    connect(_ui->cmbx_load_type, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &RelaysManager::sltUpdateUI);
    connect(_ui->cmbx_related_group, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &RelaysManager::sltUpdateUI);

    setModifiedHook(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

RelaysManager::~RelaysManager()
{ delete _ui; }

void RelaysManager::setModifiedHook(bool modified)
{
    _ui->btn_apply->setEnabled(modified);
    _ui->btn_cancel->setEnabled(modified);
}

void RelaysManager::fillFormHook(type_ptr_const_ref pElement)
{
    _ui->lbl_parent_unit->setText(s_ParentUnit::getParentDescription(pElement->_pParent));
    _ui->lbl_suin->setText(QString::number(pElement->_suin));

    _ui->chbx_enabled->setChecked(pElement->_bEnabled);
    _ui->chbx_remote_control->setChecked(pElement->_bRemoteControl);
    _ui->chbx_notify_on_state_changed->setChecked(pElement->_bNotifyOnStateChanged);

    _ui->le_alias->setText(pElement->_alias);
    _ui->le_ID->setText(QString::number(pElement->_humanizedId));

    setLoadTypeAtCombobox(pElement);
    actualizeGroupsCombobox(_ui->cmbx_load_type->currentText());
    setGroupAtCombobox(pElement);
}

void RelaysManager::clearForm()
{
    _ui->lbl_parent_unit->clear();
    _ui->lbl_suin->clear();

    _ui->chbx_enabled->setChecked(false);
    _ui->chbx_remote_control->setChecked(false);
    _ui->chbx_notify_on_state_changed->setChecked(false);

    _ui->le_alias->clear();
    _ui->le_ID->clear();

    _ui->cmbx_load_type->setCurrentIndex(-1);
    _ui->cmbx_related_group->setCurrentIndex(-1);
}

void RelaysManager::saveForm(AbstractManager::type_ptr_ref pElement)
{
    pElement->_bEnabled = _ui->chbx_enabled->isChecked();
    pElement->_bRemoteControl = _ui->chbx_remote_control->isChecked();
    pElement->_bNotifyOnStateChanged = _ui->chbx_notify_on_state_changed->isChecked();
    pElement->_alias = _ui->le_alias->text();

    pElement->_loadType = getLoadTypeFromCombobox();
    pElement->_pGroup = getGroupFromCombobox();
}

QListWidget *RelaysManager::mainListWidget()
{ return _ui->listWidget_main_list; }

QObject *RelaysManager::sender() const
{ return QWidget::sender(); }

inline void RelaysManager::setLoadTypeAtCombobox(type_ptr_const_ref pElement)
{ _ui->cmbx_load_type->setCurrentIndex(pElement->_loadType); }

inline int RelaysManager::getLoadTypeFromCombobox()
{ return _ui->cmbx_load_type->currentIndex(); }

void RelaysManager::updateGroupsCombobox()
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

void RelaysManager::setGroupAtCombobox(type_ptr_const_ref pElement)
{
    int index = -1;
    if (!pElement->_pGroup.isNull())
        index = _ui->cmbx_related_group->findData(QVariant::fromValue(pElement->_pGroup->_id),
                                                    IdRole);
    _ui->cmbx_related_group->setCurrentIndex(index);
}

t_ArmingGroup_ptr RelaysManager::getGroupFromCombobox()
{
    QVariant ret = _ui->cmbx_related_group->itemData(_ui->cmbx_related_group->currentIndex(),
                                                       DataRole);
    return ret.isValid() ? ret.value<t_ArmingGroup_ptr>() : t_ArmingGroup_ptr();
}

void RelaysManager::actualizeGroupsCombobox(const QString &loadTypeText)
{
    /* add/remove 'no group' item */
    const int idx = _ui->cmbx_related_group->findText(NO_GROUP_TEXT);
    if (0 == loadTypeText.compare(tr(LOAD_ARMING_LED_TEXT))) {
        if (-1 != idx)
            _ui->cmbx_related_group->removeItem(idx);
    }
    else if (0 == loadTypeText.compare(tr(LOAD_COMMON_TEXT))) {
        if (-1 == idx) {
            _ui->cmbx_related_group->insertItem(0, tr(NO_GROUP_TEXT));
            _ui->cmbx_related_group->setItemData(0, QVariant::fromValue(0), IdRole);
            _ui->cmbx_related_group->setItemData(0, QVariant::fromValue(t_ArmingGroup_ptr()), DataRole);
        }
    }

    /* actualize combobox state */
    if (0 == loadTypeText.compare(tr(LOAD_BELL_TEXT))) {
        _ui->cmbx_related_group->setEnabled(false);
        _ui->lbl_group->setEnabled(false);
        _ui->cmbx_related_group->setCurrentIndex(-1);
    }
    else if (0 == loadTypeText.compare(tr(LOAD_ARMING_LED_TEXT))) {
        _ui->cmbx_related_group->setEnabled(true);
        _ui->lbl_group->setEnabled(true);
        _ui->cmbx_related_group->setCurrentIndex(0);
    }
    else if (0 == loadTypeText.compare(tr(LOAD_COMMON_TEXT))) {

        _ui->cmbx_related_group->setEnabled(true);
        _ui->lbl_group->setEnabled(true);
    }
}

void RelaysManager::sltUpdateList()
{
    updateGroupsCombobox();
    updateList(this);
}

void RelaysManager::sltUpdateUI()
{
    updateUI();
    if (sender() == _ui->cmbx_load_type)
        actualizeGroupsCombobox(_ui->cmbx_load_type->currentText());
}
