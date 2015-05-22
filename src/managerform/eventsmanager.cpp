#include "precompiled.h"

#include "eventsmanager.h"
#include "ui_eventsmanager.h"

#include "bo/t_reaction.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


bool EventsManager::_bModified = false;


EventsManager::EventsManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::EventsManager())
{
    _ui->setupUi(this);

    _ui->splitter_events->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_events->setStretchFactor(1, 1);

    _ui->le_event_alias->setMaxLength(ALIAS_LEN);

    _event_relations << "reactions_list->behavior_preset_list";
    _event_relations << "reactions_list->event_id";


    connect(_ui->listWidget_events, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_event_alias,           SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_event_emitter_type,  SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_event_emitter_alias, SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_event,               SIGNAL(activated(int)),      SLOT(sltUpdateUI()));

    connect(_ui->cmbx_event_emitter_type,  SIGNAL(activated(int)),
            SLOT(sltUpdateDependentComboboxes(int)));

    connect(_ui->btn_link_reaction,   SIGNAL(clicked()), SLOT(sltLinkReaction()));
    connect(_ui->btn_unlink_reaction, SIGNAL(clicked()), SLOT(sltUnlinkReaction()));

    connect(_ui->btn_add_apply,  SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));

    updateEmitterTypeCombobox();

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

EventsManager::~EventsManager()
{
    delete _ui;
}

void EventsManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add trigger"));
        _ui->btn_del_cancel->setText(tr("Delete trigger"));
    }

    _ui->listWidget_available_reactions->setEnabled(!modified);
    _ui->listWidget_linked_reactions->setEnabled(!modified);
    _ui->btn_link_reaction->setEnabled(!modified);
    _ui->btn_unlink_reaction->setEnabled(!modified);

    _ui->listWidget_events->setEnabled(!modified);
    _bModified = modified;
}

void EventsManager::fillForm()
{
    t_Event_ptr pEvent = currentEvent();

    if (pEvent.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    _ui->le_event_alias->setText(pEvent->_alias);
    setEmitterTypeAtCombobox(pEvent->_emitter_type);
    setEmitterAliasAtCombobox(pEvent);
    setEventAtCombobox(pEvent->_event);

    updateLinkedReactionsList(pEvent);

    setModified(false);
}

void EventsManager::clearForm()
{
    _ui->le_event_alias->clear();
    _ui->cmbx_event_emitter_type->setCurrentIndex(-1);
    _ui->cmbx_event_emitter_alias->clear();
    _ui->cmbx_event->clear();

    _ui->listWidget_linked_reactions->clear();
}

void EventsManager::saveForm(t_Event_ptr &pEvent)
{
    if (pEvent.isNull())
        return;

    pEvent->_alias = _ui->le_event_alias->text();
    pEvent->_emitter_id = getEmitterIdFromAliasCombobox();
    pEvent->_emitter_type = getEmitterTypeFromCombobox();
    pEvent->_event = getEventFromCombox();
}

bool EventsManager::canBeDeleted(const t_Event_ptr &pEvent)
{
    Q_UNUSED(pEvent)
    return true;
}

inline QListWidgetItem *EventsManager::currentItem()
{
    return _ui->listWidget_events->currentItem();
}

inline t_Event_ptr EventsManager::currentEvent()
{
    if (!currentItem())
        return t_Event_ptr();

    return currentItem()->data(DataRole).value<t_Event_ptr>();
}

inline void EventsManager::updateEmitterTypeCombobox()
{
    QComboBox *c = _ui->cmbx_event_emitter_type;
    c->clear();

    c->addItem(tr("Group"),           OBJ_ARMING_GROUP);
    c->addItem(tr("Button"),          OBJ_BUTTON);
    c->addItem(tr("Protection Loop"), OBJ_ZONE);
//    c->addItem(tr("Timer"),           OBJ_INTERNAL_TIMER);
    c->addItem(tr("Touchmemory Key"), OBJ_TOUCHMEMORY_CODE);

    c->setCurrentIndex(-1);
}

inline void EventsManager::setEmitterTypeAtCombobox(int emitterType)
{
    int index = _ui->cmbx_event_emitter_type->findData(emitterType);
    _ui->cmbx_event_emitter_type->setCurrentIndex(index);
    updateEmitterAliasCombobox(emitterType);
    updateEventCombobox(emitterType);
}

inline int EventsManager::getEmitterTypeFromCombobox()
{
    QVariant v = _ui->cmbx_event_emitter_type->itemData(_ui->cmbx_event_emitter_type->currentIndex());

    if (v.isValid())
        return v.toInt();
    else
        return SEPARATOR_1;
}

void EventsManager::updateEmitterAliasCombobox(int emitterType)
{
    QComboBox *c = _ui->cmbx_event_emitter_alias;
    c->clear();

    QString pattern("SELECT id,alias FROM %1");
    qx_query q;

    switch (emitterType) {
    case OBJ_ARMING_GROUP:
        q = qx_query(pattern.arg("t_ArmingGroup"));
        break;

    case OBJ_ZONE:
        q = qx_query(pattern.arg("t_Zone"));
        break;

    case OBJ_TOUCHMEMORY_CODE:
        q = qx_query("SELECT id,alias FROM t_Key WHERE type = :keytype AND action = :action");
        q.bind(":keytype", OBJ_TOUCHMEMORY_CODE);
        q.bind(":action",  COMMON_KEY_EVENT);
        break;

    case OBJ_BUTTON:
        q = qx_query(pattern.arg("t_Button"));
        break;

    case OBJ_INTERNAL_TIMER:
        q = qx_query(pattern.arg("t_Timer"));
        break;

    default:
        return;
    }

    qx::dao::call_query(q);
    for (int i = 0; i < q.getSqlResultRowCount(); ++i)  {
        int id = q.getSqlResultAt(i,0).toInt();
        QString alias = q.getSqlResultAt(i,1).toString();
        _ui->cmbx_event_emitter_alias->addItem(alias, id);
    }

    if (c->count())
        c->setCurrentIndex(0);
}

void EventsManager::setEmitterAliasAtCombobox(const t_Event_ptr &pEvent)
{
    int index = _ui->cmbx_event_emitter_alias->findData(pEvent->_emitter_id);
    _ui->cmbx_event_emitter_alias->setCurrentIndex(index);
}

int EventsManager::getEmitterIdFromAliasCombobox()
{
    int index = _ui->cmbx_event_emitter_alias->currentIndex();
    return _ui->cmbx_event_emitter_alias->itemData(index).toInt();
}

inline void EventsManager::updateEventCombobox(int emitterType)
{
    QComboBox *c = _ui->cmbx_event;
    c->clear();

    switch (emitterType) {
    case OBJ_ARMING_GROUP:
        c->addItem(tr("ARMING"), GROUP_EVENT_STATE_CHANGED_DISARMED_TO_ARMED);
        c->addItem(tr("DISARMING"), GROUP_EVENT_STATE_CHANGED_ARMED_TO_DISARMED);
        break;

    case OBJ_BUTTON:
        c->addItem(tr("Button released"),     BUTTON_RELEASED);
        c->addItem(tr("Button pressed"),      BUTTON_PRESSED);
        c->addItem(tr("Button long-pressed"), BUTTON_PRESSED_LONG);
        break;

    case OBJ_ZONE:
        c->addItem(tr("Zone normal"),   ZONE_EVENT_NORMAL);
        c->addItem(tr("Zone break"),    ZONE_EVENT_BREAK);
        c->addItem(tr("Zone disabled"), ZONE_EVENT_DISABLED);
        c->addItem(tr("Zone fitting"),  ZONE_EVENT_FITTING);
        c->addItem(tr("Zone short"),    ZONE_EVENT_SHORT);
        break;

    case OBJ_TOUCHMEMORY_CODE:
//        c->addItem(tr("Arming/Disarming key"), ARMING_EVENT);
        c->addItem(tr("Common key"),           COMMON_KEY_EVENT);
        break;

    case OBJ_INTERNAL_TIMER:
        c->addItem(tr("Delayed Zone timeout"), DELAYEDZONE_TIMER_EVENT_TIMEOUT);
        break;

    default:
        return;
    }

    c->setCurrentIndex(0);
}

inline void EventsManager::setEventAtCombobox(int eventCode)
{
    int index = _ui->cmbx_event->findData(eventCode);
    _ui->cmbx_event->setCurrentIndex(index);
}

inline int EventsManager::getEventFromCombox()
{
    QVariant v = _ui->cmbx_event->itemData(_ui->cmbx_event->currentIndex());

    if (v.isValid())
        return v.toInt();
    else
        return -1;
}

inline void EventsManager::updateAvailableReactionsList()
{
    _ui->listWidget_available_reactions->clear();

    t_ReactionX list;

    qx::dao::fetch_by_query(qx_query().where("event_id").isNull(), list);

    t_Reaction_ptr pReaction;
    _foreach (pReaction, list) {
        QListWidgetItem *item = new QListWidgetItem(pReaction->_alias);
        item->setData(IdRole,   QVariant::fromValue(pReaction->_id));
        item->setData(DataRole, QVariant::fromValue(pReaction));
        _ui->listWidget_available_reactions->addItem(item);
    }
}

inline void EventsManager::updateLinkedReactionsList(const t_Event_ptr &pEvent)
{
    _ui->listWidget_linked_reactions->clear();

    if (pEvent.isNull())
        return;

    t_Reaction_ptr pReaction;
    _foreach (pReaction, pEvent->_reactions_list) {
        QListWidgetItem *item = new QListWidgetItem(pReaction->_alias);
        item->setData(IdRole,   QVariant::fromValue(pReaction->_id));
        item->setData(DataRole, QVariant::fromValue(pReaction));
        _ui->listWidget_linked_reactions->addItem(item);
    }
}


void EventsManager::sltUpdateList()
{
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_events->currentRow()
                        : -1;

    qx::dao::fetch_all_with_relation(_event_relations, _event_list);

    _ui->listWidget_events->blockSignals(true);
    _ui->listWidget_events->clear();
    _ui->listWidget_events->blockSignals(false);
    clearForm();
    setModified(false);

    t_Event_ptr pEvent;
    _foreach (pEvent, _event_list) {
        QListWidgetItem *item = new QListWidgetItem(pEvent->_alias);
        item->setData(IdRole,   QVariant::fromValue(pEvent->_id));
        item->setData(DataRole, QVariant::fromValue(pEvent));
        _ui->listWidget_events->addItem(item);
    }

    updateAvailableReactionsList();

    // -- select appropriate row
    if (_bAdding || currentRow >= _ui->listWidget_events->count())
        _ui->listWidget_events->setCurrentRow(_ui->listWidget_events->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_events->count())
        _ui->listWidget_events->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_events->count())*/
        _ui->listWidget_events->setCurrentRow(currentRow);
}

void EventsManager::sltUpdateUI()
{
    QObject *obj = sender();
    if (obj == _ui->listWidget_events) {
        fillForm();
    } else {
        setModified(true);

        // TODO: when event emitter type is OBJ_TOUCHMEMORY_CODE
        //       disable event_combobox widget and forcedly select right 'event' according to
        //       selected 'Key-item'
    }
}

void EventsManager::sltUpdateDependentComboboxes(int emitterTypeIndex)
{
    int emitterType = _ui->cmbx_event_emitter_type->itemData(emitterTypeIndex).toInt();
    updateEmitterAliasCombobox(emitterType);
    updateEventCombobox(emitterType);
}

void EventsManager::sltAddApply()
{
    if (isModified()) {        // apply
        if (!_bAdding && currentItem()) {    // apply editing
            t_Event_ptr pEvent = currentEvent();
            saveForm(pEvent);

            qx::dao::update_optimized(pEvent);

            int row = _ui->listWidget_events->currentRow();
            sltUpdateList();
            _ui->listWidget_events->setCurrentRow(row);
        } else {            // apply adding
            t_Event_ptr pEvent(new t_Event());
            saveForm(pEvent);

            qx::dao::insert(pEvent);

            sltUpdateList();
            _bAdding = false;
            _ui->listWidget_events->setCurrentRow(_ui->listWidget_events->count() - 1);
        }
        Q_EMIT snlListChanged();
        setModified(false);
    } else {               // add new
        clearForm();
        _bAdding = true;
        _ui->listWidget_events->clearSelection();
        setModified(true);
        _ui->le_event_alias->setFocus();
    }
}

void EventsManager::sltDeleteCancel()
{
    _bAdding = false;

    if (currentItem()) {
        if (isModified()) // cancel
        {
            _ui->listWidget_events->setCurrentItem(currentItem());  // => current item will be selected
            fillForm();
        }
        else          // delete
        {
            t_Event_ptr pEvent = currentEvent();
            if (canBeDeleted(pEvent))
            {
                qx_query q("UPDATE t_Reaction SET event_id = NULL WHERE event_id = :event_id");
                q.bind(":event_id", pEvent->_id);
                qx::dao::call_query(q);

                qx::dao::delete_by_id(pEvent);

                sltUpdateList();
                Q_EMIT snlListChanged();
            }
        }
    } else {
        clearForm();
    }

    setModified(false);
}

void EventsManager::sltLinkReaction()
{
    t_Event_ptr pEvent = currentEvent();
    QListWidgetItem *reaction_item = _ui->listWidget_available_reactions->currentItem();

    if (pEvent.isNull() || !reaction_item)
        return;

    t_Reaction_ptr pReaction = reaction_item->data(DataRole).value<t_Reaction_ptr>();
    pReaction->_event = pEvent;

    qx::dao::update_optimized(pReaction);

    Q_EMIT snlListChanged();
}

void EventsManager::sltUnlinkReaction()
{
    QListWidgetItem *reaction_item = _ui->listWidget_linked_reactions->currentItem();

    if (!reaction_item)
        return;

    t_Reaction_ptr pReaction = reaction_item->data(DataRole).value<t_Reaction_ptr>();
    pReaction->_event = t_Event_ptr();

    qx::dao::update_optimized(pReaction);

    Q_EMIT snlListChanged();
}
