#include "precompiled.h"

#include "arminggroupsmanager.h"
#include "ui_arminggroupsmanager.h"
#include "keysmanager.h"

#include <QTimer>
#include <QMessageBox>

#include "bo/t_zone.h"
#include "bo/s_parentunit.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


bool ArmingGroupsManager::_bModified = false;

enum ArmingDeviceType {
    ADT_ETR,
    ADT_Keyboard
};

ArmingGroupsManager::ArmingGroupsManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::ArmingGroupsManager())
{
    _ui->setupUi(this);

    _ui->splitter_groups->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_groups->setStretchFactor(1, 1);

    _ui->le_group_alias->setMaxLength(ALIAS_LEN);

    _ui->cmbx_arming_device_type->insertItem(ADT_ETR, tr("ETRs"));
    _ui->cmbx_arming_device_type->insertItem(ADT_Keyboard, tr("Keyboards"));

    _group_relations << "keys_list" << "etr_list" << "etr_list->uin_id";


    connect(_ui->listWidget_groups, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_group_alias, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));

    connect(_ui->btn_add_apply,  SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));

    connect(_ui->btn_add_key_to_group,      SIGNAL(clicked()), SLOT(sltMoveKeyToRelated()));
    connect(_ui->btn_remove_key_from_group, SIGNAL(clicked()), SLOT(sltMoveKeyToFree()));

    connect(_ui->btn_accept_device_for_group, SIGNAL(clicked()), SLOT(sltMoveArmingDeviceToRelated()));
    connect(_ui->btn_deny_device_for_group,   SIGNAL(clicked()), SLOT(sltMoveArmingDeviceToAvailable()));

    connect(_ui->cmbx_arming_device_type, SIGNAL(currentIndexChanged(int)),
            SLOT(sltUpdateArmingDevicesLists(int)));


    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

ArmingGroupsManager::~ArmingGroupsManager()
{
    delete _ui;
}

inline void ArmingGroupsManager::fillForm()
{
    t_ArmingGroup_ptr pGroup = currentGroup();

    if (pGroup.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    _ui->lbl_group_id->setText(QString::number(pGroup->_id));
    _ui->le_group_alias->setText(pGroup->_alias);

    sltUpdateArmingDevicesLists(_ui->cmbx_arming_device_type->currentIndex());
    updateRelatedKeysList(pGroup);

    setModified(false);
}

inline void ArmingGroupsManager::clearForm()
{
    _ui->lbl_group_id->clear();
    _ui->le_group_alias->clear();
}

inline void ArmingGroupsManager::saveForm(t_ArmingGroup_ptr &pGroup)
{
    if (pGroup.isNull())
        return;

    pGroup->_alias = _ui->le_group_alias->text();
}

inline void ArmingGroupsManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
        _ui->tabWidget->setCurrentIndex(0);
    } else {
        _ui->btn_add_apply->setText(tr("Add group"));
        _ui->btn_del_cancel->setText(tr("Delete group"));
    }

    _ui->tabWidget->setTabEnabled(1, !modified);
    _ui->tabWidget->setTabEnabled(2, !modified);

    _ui->listWidget_groups->setEnabled(!modified);
    _bModified = modified;
}

inline bool ArmingGroupsManager::canBeDeleted(const t_ArmingGroup_ptr &pGroup)
{
    if (pGroup.isNull())
        return false;

    t_ZoneX zone_list;
    qx_query query("WHERE arming_group_id = :id");
    query.bind(":id", (int)pGroup->_id);
    qx::dao::fetch_by_query(query, zone_list);
    return zone_list.size() == 0;
}

inline QListWidgetItem *ArmingGroupsManager::currentItem()
{
    return _ui->listWidget_groups->currentItem();
}

t_ArmingGroup_ptr ArmingGroupsManager::currentGroup()
{
    if (!currentItem())
        return t_ArmingGroup_ptr();

    int id = currentItem()->data(IdRole).toInt();
    return _arming_group_list.getByKey(id);
}

void ArmingGroupsManager::updateRelatedKeyboardsList(t_ArmingGroup_ptr pGroup)
{
    Q_UNUSED(pGroup)

    _ui->listWidget_allowed_arming_devices->clear();
}

void ArmingGroupsManager::updateRelatedETRsList(t_ArmingGroup_ptr pGroup)
{
    _ui->listWidget_allowed_arming_devices->clear();

    if (pGroup.isNull())
        return;

    t_Etr_ptr pEtr;
    _foreach (pEtr, pGroup->_etr_list) {
        QListWidgetItem *item = new QListWidgetItem(pEtr->_uin->_palias);
        item->setData(IdRole,   QVariant::fromValue(pEtr->_id));
        item->setData(DataRole, QVariant::fromValue(pEtr));
        _ui->listWidget_allowed_arming_devices->addItem(item);
    }
}

void ArmingGroupsManager::updateRelatedKeysList(t_ArmingGroup_ptr pGroup)
{
    _ui->listWidget_ingroup_keys->clear();

    if (pGroup.isNull())
        return;

    t_Key_ptr pKey;
    _foreach (pKey, pGroup->_keys_list) {
        QString name = pKey->_alias.isEmpty()
                            ? KeysManager::fromHexDataToText(pKey->_value)
                            : pKey->_alias;
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(IdRole,   QVariant::fromValue(pKey->_id));
        item->setData(DataRole, QVariant::fromValue(pKey));
        _ui->listWidget_ingroup_keys->addItem(item);
    }
}

void ArmingGroupsManager::updateAvailableKeyboardsList()
{
    _ui->listWidget_available_arming_devices->clear();
}

void ArmingGroupsManager::updateAvailableETRsList(t_ArmingGroup_ptr pGroup)
{
    _ui->listWidget_available_arming_devices->clear();

    if (pGroup.isNull())
        return;

    t_EtrX list;

    qx::dao::fetch_all_with_relation("uin_id", list);

    t_Etr_ptr pEtr;
    _foreach (pEtr, list) {
        if (!pGroup->_etr_list.exist(pEtr->_id)) {
            QListWidgetItem *item = new QListWidgetItem(pEtr->_uin->_palias);
            item->setData(IdRole,   QVariant::fromValue(pEtr->_id));
            item->setData(DataRole, QVariant::fromValue(pEtr));
            _ui->listWidget_available_arming_devices->addItem(item);
        }
    }
}

void ArmingGroupsManager::updateFreeKeysList()
{
    _ui->listWidget_free_keys->clear();

    t_KeyX list;

    // -- fetch Keys that not belong to any GROUP and can be used for Arming/Disarming
    qx::dao::fetch_by_query(qx_query().where("arming_group_id").isNull()
                            .and_("action").isEqualTo(ARMING_EVENT), list);

    t_Key_ptr pKey;
    _foreach (pKey, list) {
        QString name = pKey->_alias.isEmpty()
                            ? KeysManager::fromHexDataToText(pKey->_value)
                            : pKey->_alias;
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(IdRole,   QVariant::fromValue(pKey->_id));
        item->setData(DataRole, QVariant::fromValue(pKey));
        _ui->listWidget_free_keys->addItem(item);
    }
}

void ArmingGroupsManager::sltUpdateList()
{
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_groups->currentRow()
                        : -1;

    // -- get all GROUPS
    qx::dao::fetch_all_with_relation(_group_relations, _arming_group_list);

    _ui->listWidget_groups->blockSignals(true);
    _ui->listWidget_groups->clear();
    _ui->listWidget_groups->blockSignals(false);
    clearForm();
    setModified(false);

    t_ArmingGroup_ptr pGroup;
    _foreach (pGroup, _arming_group_list) {
        QListWidgetItem *item = new QListWidgetItem(pGroup->_alias);
        item->setData(IdRole, QVariant((int)pGroup->_id));
        _ui->listWidget_groups->addItem(item);
    }

    // -- get all free KEYs
    updateFreeKeysList();

    // -- select appropriate row
    if (_bAdding || currentRow >= _ui->listWidget_groups->count())
        _ui->listWidget_groups->setCurrentRow(_ui->listWidget_groups->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_groups->count())
        _ui->listWidget_groups->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_groups->count())*/
        _ui->listWidget_groups->setCurrentRow(currentRow);
}

void ArmingGroupsManager::sltUpdateUI()
{
    QObject *obj = sender();
    if (obj == _ui->listWidget_groups) {
        fillForm();
    } else if (obj == _ui->le_group_alias) {
        setModified(true);
    }
}

void ArmingGroupsManager::sltAddApply()
{
    if (isModified()) {        // apply
        if (!_bAdding && currentItem()) {    // apply editing
            t_ArmingGroup_ptr pGroup = currentGroup();
            saveForm(pGroup);

            qx::dao::update_with_relation(_group_relations, pGroup);

            int row = _ui->listWidget_groups->currentRow();
            sltUpdateList();
            _ui->listWidget_groups->setCurrentRow(row);
        } else {            // apply adding
            t_ArmingGroup_ptr pGroup(new t_ArmingGroup());
            saveForm(pGroup);

            qx::dao::insert_with_relation(_group_relations, pGroup);

            sltUpdateList();
            _bAdding = false;
            _ui->listWidget_groups->setCurrentRow(_ui->listWidget_groups->count() - 1);
        }
        Q_EMIT snlListChanged();
        setModified(false);
    } else {               // add new
        clearForm();
        _bAdding = true;
        _ui->listWidget_groups->clearSelection();
        setModified(true);
        _ui->le_group_alias->setFocus();
    }
}

void ArmingGroupsManager::sltDeleteCancel()
{
    _bAdding = false;

    if (currentItem()) {
        if (isModified()) {    // cancel
            _ui->listWidget_groups->setCurrentItem(currentItem());  // => current item will be selected
            fillForm();
        } else {            // delete
            t_ArmingGroup_ptr pGroup = currentGroup();
            if (canBeDeleted(pGroup)) {
                // -- delete GROUP
                qx::dao::delete_by_id(pGroup);

                // -- cascaded delete all related KEYs
                if (QMessageBox::Yes == QMessageBox::information(this,
                                                                 tr("Delete Group"),
                                                                 tr("Do you want to delete all related keys?"),
                                                                 QMessageBox::Yes|QMessageBox::No,
                                                                 QMessageBox::No
                                                                 ))
                {
                    qx_query q("DELETE FROM t_Key WHERE arming_group_id = :agid");
                    q.bind(":agid", (int)pGroup->_id);
                    qx::dao::call_query(q);

                    // -- compress Keys' IDs
                    t_KeyX list;
                    qx::dao::fetch_all(list);
                    qx::dao::delete_all<t_Key>();
                    qx::dao::insert(list);
                }
                else
                {
                    qx_query q("UPDATE t_Key SET arming_group_id = NULL WHERE arming_group_id = :agid");
                    q.bind(":agid", (int)pGroup->_id);
                    qx::dao::call_query(q);
                }

                // ----------------------------------------------------------------------
                // FIXME: replace this block by a trigger or smth else
                //        or maybe do smth checks and show a message for a user
                // ----------------------------------------------------------------------
                /* reset all related (dependent) entities */
                qx_query q("UPDATE t_Relay SET load_type = 0 WHERE group_id = :agid");
                q.bind(":agid", (int)pGroup->_id);
                qx::dao::call_query(q);

                q = qx_query("UPDATE t_Relay SET group_id = NULL WHERE group_id = :agid");
                q.bind(":agid", (int)pGroup->_id);
                qx::dao::call_query(q);

                q = qx_query("UPDATE t_Led SET bArmingLed = 0 WHERE group_id = :agid");
                q.bind(":agid", (int)pGroup->_id);
                qx::dao::call_query(q);

                q = qx_query("UPDATE t_Led SET group_id = NULL WHERE group_id = :agid");
                q.bind(":agid", (int)pGroup->_id);
                qx::dao::call_query(q);
                // ----------------------------------------------------------------------

                sltUpdateList();
                Q_EMIT snlListChanged();
            }
        }
    } else {
        clearForm();
    }

    setModified(false);
}

void ArmingGroupsManager::sltUpdateArmingDevicesLists(int index)
{
    t_ArmingGroup_ptr pGroup = currentGroup();

    switch (index) {
    case ADT_ETR:
        updateAvailableETRsList(pGroup);
        updateRelatedETRsList(pGroup);
        break;

    case ADT_Keyboard:
        updateAvailableKeyboardsList();
        updateRelatedKeyboardsList(pGroup);
        break;
    }
}

void ArmingGroupsManager::sltMoveArmingDeviceToRelated()
{
    t_ArmingGroup_ptr pGroup = currentGroup();
    QListWidgetItem *arming_device_item = _ui->listWidget_available_arming_devices->currentItem();

    if (pGroup.isNull() || !arming_device_item)
        return;

    switch (_ui->cmbx_arming_device_type->currentIndex()) {
    case ADT_ETR:
    {
        t_Etr_ptr pEtr = arming_device_item->data(DataRole).value<t_Etr_ptr>();
        pGroup->_etr_list.insert(pEtr->_id, pEtr);
        qx::dao::save_with_relation("etr_list", pGroup);
        break;
    }

    case ADT_Keyboard:
        break;
    }

    Q_EMIT snlListChanged();
}

void ArmingGroupsManager::sltMoveArmingDeviceToAvailable()
{
    t_ArmingGroup_ptr pGroup = currentGroup();
    QListWidgetItem *arming_device_item = _ui->listWidget_allowed_arming_devices->currentItem();

    if (pGroup.isNull() || !arming_device_item)
        return;

    switch (_ui->cmbx_arming_device_type->currentIndex()) {
    case ADT_ETR:
    {
        t_Etr_ptr pEtr = arming_device_item->data(DataRole).value<t_Etr_ptr>();
        pGroup->_etr_list.removeByKey(pEtr->_id);
        qx::dao::save_with_relation("etr_list", pGroup);
        break;
    }

    case ADT_Keyboard:
        break;
    }

    Q_EMIT snlListChanged();
}

void ArmingGroupsManager::sltMoveKeyToRelated()
{
    t_ArmingGroup_ptr pGroup = currentGroup();
    QListWidgetItem *key_item = _ui->listWidget_free_keys->currentItem();

    if (pGroup.isNull() || !key_item)
        return;

    t_Key_ptr pKey = key_item->data(DataRole).value<t_Key_ptr>();
    pKey->_arming_group = pGroup;

    qx::dao::update_with_relation("arming_group_id", pKey);

//    pGroup->_keys_list.insert(pKey->_id, pKey);
//    qx::dao::save_with_relation("keys_list", pGroup);

    Q_EMIT snlListChanged();
}

void ArmingGroupsManager::sltMoveKeyToFree()
{
    t_ArmingGroup_ptr pGroup = currentGroup();
    QListWidgetItem *key_item = _ui->listWidget_ingroup_keys->currentItem();

    if (pGroup.isNull() || !key_item)
        return;

    t_Key_ptr pKey = key_item->data(DataRole).value<t_Key_ptr>();
    pKey->_arming_group = t_ArmingGroup_ptr();

    qx::dao::update_with_relation("arming_group_id", pKey);

//    pGroup->_keys_list.removeByKey(pKey->_id);
//    qx::dao::save_with_relation("keys_list", pGroup);

    Q_EMIT snlListChanged();
}
