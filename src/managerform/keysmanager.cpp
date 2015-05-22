#include "precompiled.h"

#include "keysmanager.h"
#include "ui_keysmanager.h"

#include "bo/s_parentunit.h"
#include "bo/t_etr.h"

#include <QTimer>
#include <QMessageBox>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


//: key actions
#define KEY_ACTION_ARM_DISARM_TEXT      QT_TRANSLATE_NOOP("KeysManager", "Arming / Disarming")
#define KEY_ACTION_ACCESS_CONTROL_TEXT  QT_TRANSLATE_NOOP("KeysManager", "Common key")

//: key type
#define KEY_TYPE_KBD_TEXT           QT_TRANSLATE_NOOP("KeysManager", "Keyboard key")
#define KEY_TYPE_TOUCHMEMORY_TEXT   QT_TRANSLATE_NOOP("KeysManager", "Touchmemory key")

//: meaning <any arming group> item of combobox
#define GROUP_ANY_TEXT  QT_TRANSLATE_NOOP("KeysManager", "ANY")


bool KeysManager::_bModified = false;
int  KeysManager::_current_ETR_UIN = 0;


KeysManager::KeysManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::KeysManager)
{
    _ui->setupUi(this);

    _ui->splitter_keys->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_keys->setStretchFactor(1, 1);

    _ui->le_key_alias->setMaxLength(ALIAS_LEN);
    _ui->le_key_value->setValidator(new QRegExpValidator(QRegExp("(\\d|[A-Fa-f]){12}"), this));

    _ui->cmbx_key_action->insertItem(COMMON_KEY_EVENT, tr(KEY_ACTION_ACCESS_CONTROL_TEXT));
    _ui->cmbx_key_action->insertItem(ARMING_EVENT, tr(KEY_ACTION_ARM_DISARM_TEXT));

    _ui->cmbx_key_type->addItems(QStringList() << tr(KEY_TYPE_TOUCHMEMORY_TEXT)
                                              << tr(KEY_TYPE_KBD_TEXT));

    connect(_ui->btn_add_apply, SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));
    connect(_ui->btn_read_from_device, SIGNAL(clicked()), SLOT(sltRequestKeyFromDevice()));

    connect(_ui->listWidget_keys, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->cmbx_key_action, SIGNAL(activated(int)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_key_type,   SIGNAL(activated(int)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_key_group,  SIGNAL(activated(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_key_alias, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_key_value, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));

    setModified(false);

    // -- update all needed data
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, &KeysManager::sltUpdateList);

    timer->start(0);
}

KeysManager::~KeysManager()
{
    delete _ui;
}

void KeysManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add key"));
        _ui->btn_del_cancel->setText(tr("Delete key"));
    }

    _ui->listWidget_keys->setEnabled(!modified);
    _bModified = modified;
}

void KeysManager::fillForm()
{
    t_Key_ptr pKey = currentKey();

    if (pKey.isNull()) {  // protection
        clearForm();
        setModified(false);
        return;
    }

    _ui->lbl_key_id->setText(QString::number(pKey->_id));
    _ui->le_key_alias->setText(pKey->_alias);
    _ui->le_key_value->setText(fromHexDataToText(pKey->_value));

    QString keyAction;
    switch (pKey->_action) {
    case COMMON_KEY_EVENT:
        keyAction = tr(KEY_ACTION_ACCESS_CONTROL_TEXT);
        break;
    case ARMING_EVENT:
        keyAction = tr(KEY_ACTION_ARM_DISARM_TEXT);
        break;
    default:
        break;
    }
    _ui->cmbx_key_action->setCurrentIndex(-1);
    _ui->cmbx_key_action->setCurrentText(keyAction);

    QString keyType;
    switch (pKey->_type) {
    case OBJ_TOUCHMEMORY_CODE:
        keyType = tr(KEY_TYPE_TOUCHMEMORY_TEXT);
        break;
    case OBJ_KEYBOARD_CODE:
        keyType = tr(KEY_TYPE_KBD_TEXT);
        break;
    }
    _ui->cmbx_key_type->setCurrentIndex(-1);
    _ui->cmbx_key_type->setCurrentText(keyType);

    actualizeGroupsCombobox();
    setGroupAtCombobox(pKey);

    setModified(false);
}

void KeysManager::clearForm()
{
    _ui->lbl_key_id->clear();
    _ui->le_key_alias->clear();
    _ui->le_key_value->clear();
    _ui->cmbx_key_action->setCurrentIndex(-1);
    _ui->cmbx_key_type->setCurrentIndex(-1);
    _ui->cmbx_key_group->setCurrentIndex(-1);
}

void KeysManager::saveForm(t_Key_ptr &pKey)
{
    if (pKey.isNull())
        return;

    pKey->_alias = _ui->le_key_alias->text();
    pKey->_value = toHexDataFromText(_ui->le_key_value->text());

    if (tr(KEY_ACTION_ACCESS_CONTROL_TEXT) == _ui->cmbx_key_action->currentText())
        pKey->_action = COMMON_KEY_EVENT;
    else if (tr(KEY_ACTION_ARM_DISARM_TEXT) == _ui->cmbx_key_action->currentText())
        pKey->_action = ARMING_EVENT;
    else
        pKey->_action = -1;

    if (tr(KEY_TYPE_KBD_TEXT) == _ui->cmbx_key_type->currentText())
        pKey->_type = OBJ_KEYBOARD_CODE;
    else if (tr(KEY_TYPE_TOUCHMEMORY_TEXT) == _ui->cmbx_key_type->currentText())
        pKey->_type = OBJ_TOUCHMEMORY_CODE;
    else
        pKey->_type = SEPARATOR_1;

    pKey->_arming_group = getGroupFromCombobox();
}

inline t_ArmingGroup_ptr KeysManager::getGroupFromCombobox()
{
    return _ui->cmbx_key_group->itemData(_ui->cmbx_key_group->currentIndex(), DataRole)
                                    .value<t_ArmingGroup_ptr>();
}

inline void KeysManager::setGroupAtCombobox(t_Key_ptr pKey)
{
    int index = -1;
    if (!pKey->_arming_group.isNull())
        index = _ui->cmbx_key_group->findData(QVariant::fromValue(pKey->_arming_group->_id), IdRole);
    else
        index = 0;  // for 'Any group'
    _ui->cmbx_key_group->setCurrentIndex(index);
}

inline void KeysManager::updateAvailableGroupsCombobox()
{
    t_ArmingGroupX list;

    qx::dao::fetch_all(list);
    _ui->cmbx_key_group->clear();
\
    /* fill combobox with all available groups */
    t_ArmingGroup_ptr pGroup;
    _foreach(pGroup, list) {
        _ui->cmbx_key_group->addItem(pGroup->_alias);

        // -- set data to the last added item
        int index =  _ui->cmbx_key_group->count() - 1;
        _ui->cmbx_key_group->setItemData(index, QVariant::fromValue(pGroup->_id), IdRole);
        _ui->cmbx_key_group->setItemData(index, QVariant::fromValue(pGroup),      DataRole);
    }
}

inline QListWidgetItem *KeysManager::currentItem()
{
    return _ui->listWidget_keys->currentItem();
}

inline t_Key_ptr KeysManager::currentKey()
{
    //Q_ASSERT(currentItem());

    if (!currentItem())
        return t_Key_ptr();

    int id = currentItem()->data(Qt::UserRole).toInt();
    return _keys_list.getByKey(id);
}

void KeysManager::actualizeGroupsCombobox()
{
    const int idx =_ui->cmbx_key_group->findText(tr(GROUP_ANY_TEXT));
    if (0 ==_ui->cmbx_key_action->currentText().compare(tr(KEY_ACTION_ARM_DISARM_TEXT))) {
        if (-1 != idx)
            _ui->cmbx_key_group->removeItem(idx);
    }
    else {
        if (-1 == idx) {
            _ui->cmbx_key_group->insertItem(0, tr(GROUP_ANY_TEXT));
            _ui->cmbx_key_group->setItemData(0, QVariant::fromValue(0), IdRole);
            _ui->cmbx_key_group->setItemData(0, QVariant::fromValue(t_ArmingGroup_ptr()), DataRole);
        }
    }
}

bool KeysManager::keyExists(const t_Key_ptr &key)
{
    t_Key_ptr pKey;
    t_KeyX list;

    qx::dao::fetch_all(list);

    _foreach (pKey, list) {
        if (!key->_value.isEmpty()
                && pKey->_id != key->_id
                && pKey->_value == key->_value)
            return true;
    }

    return false;
}

QString KeysManager::fromHexDataToText(const QByteArray &a)
{ return a.toHex().toUpper(); }

QByteArray KeysManager::toHexDataFromText(const QString &text)
{
    QByteArray arr;

    for (int i = 0; i < text.size(); i += 2) {
        QString s = text.mid(i, 2);
        bool ok = true;
        char byte = s.toInt(&ok, 16);
        if (!ok)
            return QByteArray();
        arr.append(byte);
    }

    return arr;
}

void KeysManager::sltUpdateList()
{
    // -- update arming groups list
    updateAvailableGroupsCombobox();

    // --
    sltUpdateAvailableETRsCombobox();

    // -- update keys list
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_keys->currentRow()
                        : -1;

    QStringList relation;
    relation << "arming_group_id";
    qx::dao::fetch_all_with_relation(relation, _keys_list);

    _ui->listWidget_keys->blockSignals(true);
    _ui->listWidget_keys->clear();
    _ui->listWidget_keys->blockSignals(false);
    clearForm();
    setModified(false);

    t_Key_ptr pKey;
    _foreach(pKey, _keys_list) {
        QString name = pKey->_alias.isEmpty()
                            ? fromHexDataToText(pKey->_value)
                            : pKey->_alias;
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, QVariant((int)pKey->_id));
        _ui->listWidget_keys->addItem(item);
    }

    if (_bAdding || currentRow >= _ui->listWidget_keys->count())
        _ui->listWidget_keys->setCurrentRow(_ui->listWidget_keys->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_keys->count())
        _ui->listWidget_keys->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_keys->count())*/
        _ui->listWidget_keys->setCurrentRow(currentRow);
}

void KeysManager::sltUpdateAvailableETRsCombobox()
{
    t_EtrX list;

    QStringList relation;
    relation << "uin_id";

    qx::dao::fetch_all_with_relation(relation, list);
    _ui->cmbx_read_device->clear();

    t_Etr_ptr pEtr;
    _foreach(pEtr, list) {
        _ui->cmbx_read_device->addItem(pEtr->_uin->_palias);

        // -- set data to the last added item
        int index =  _ui->cmbx_read_device->count() - 1;
        _ui->cmbx_read_device->setItemData(index, QVariant::fromValue(pEtr->_id), IdRole);
        _ui->cmbx_read_device->setItemData(index, QVariant::fromValue(pEtr),      DataRole);
    }
}

void KeysManager::sltActivateSpecialControls(bool activate)
{
    _ui->btn_read_from_device->setEnabled(activate);
    _ui->cmbx_read_device->setEnabled(activate);
}

void KeysManager::sltUpdateUI()
{
    QObject * const obj = sender();
    if (obj == _ui->listWidget_keys) {
        fillForm();
    } else {
        setModified(true);

        if (obj == _ui->cmbx_key_action)
            actualizeGroupsCombobox();
    }
}

void KeysManager::sltAddApply()
{
    if (isModified())         // apply key
    {
        if (_bAdding || !currentItem())   // apply adding key
        {
            t_Key_ptr pKey(new t_Key());
            saveForm(pKey);

            if (keyExists(pKey)) {
                QMessageBox::information(this,
                                         tr("Add key"),
                                         tr("Key \"<b>%1</b>\" already registered")
                                            .arg(fromHexDataToText(pKey->_value))
                                         );
                _ui->le_key_value->setFocus();
                return;
            }

            qx::dao::insert(pKey);

            sltUpdateList();
            _ui->listWidget_keys->setCurrentRow(_ui->listWidget_keys->count() - 1);
            _bAdding = false;
        }
        else                // apply editing key
        {
            t_Key_ptr pKey = currentKey();
            saveForm(pKey);

            if (keyExists(pKey)) {
                QMessageBox::information(this,
                                         tr("Edit key"),
                                         tr("Key \"<b>%1</b>\" already registered")
                                            .arg(fromHexDataToText(pKey->_value))
                                         );
                _ui->le_key_value->setFocus();
                return;
            }

            qx::dao::update_optimized(pKey);

            int currentKeyRow = _ui->listWidget_keys->currentRow();
            sltUpdateList();
            _ui->listWidget_keys->setCurrentRow(currentKeyRow);
        }

        Q_EMIT snlListChanged();
        setModified(false);
    }
    else             // add new key
    {
        clearForm();
        _bAdding = true;
        _ui->listWidget_keys->clearSelection();
        setModified(true);
        _ui->le_key_alias->setFocus();
    }

}

void KeysManager::sltDeleteCancel()
{
    _bAdding = false;

    if (currentItem()) {
        if (isModified()) // cancel
        {
            _ui->listWidget_keys->setCurrentItem(currentItem());    // => current item will be selected
            fillForm();
        }
        else   // delete
        {
            t_Key_ptr pKey = currentKey();
            qx::dao::delete_by_id(pKey);
            sltUpdateList();
            Q_EMIT snlListChanged();
        }
    } else {
        clearForm();
    }

    setModified(false);
}

void KeysManager::sltRequestKeyFromDevice()
{
    t_Etr_ptr pEtr = _ui->cmbx_read_device->itemData(_ui->cmbx_read_device->currentIndex(),
                                                     DataRole).value<t_Etr_ptr>();
    if (pEtr.isNull())
        return;

    _current_ETR_UIN = pEtr->_uin->_puin;   // remember current reader's UIN
    Q_EMIT snlRequestKeyFromDevice();
}
