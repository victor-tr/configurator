#include "precompiled.h"

#include "reactionsmanager.h"
#include "ui_reactionsmanager.h"

#include <QTimer>

#include "configurator_protocol.h"

#include "bo/t_led.h"
#include "bo/t_bell.h"
#include "bo/t_relay.h"
#include "bo/t_zone.h"
#include "bo/t_key.h"

#include <QxMemLeak.h>


#define SWITCH_ON_TEXT  QT_TRANSLATE_NOOP("ReactionsManager", "Switch On")
#define SWITCH_OFF_TEXT QT_TRANSLATE_NOOP("ReactionsManager", "Switch Off")
#define USE_PRESET_TEXT QT_TRANSLATE_NOOP("ReactionsManager", "Use preset:")


enum PerformerType {
    PerfType_Bell,
    PerfType_Led,
    PerfType_Relay,
    PerfType_TimerStart,
    PerfType_TimerStop
};

bool ReactionsManager::_bModified = false;


ReactionsManager::ReactionsManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::ReactionsManager())
{
    _ui->setupUi(this);

    _ui->chbx_state_ad->setVisible(false);
    _ui->chbx_state_da->setVisible(false);

    _ui->splitter_reactions->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_reactions->setStretchFactor(1, 1);

    _ui->le_reaction_alias->setMaxLength(ALIAS_LEN);

    // -- setup reactions tab
    QStringList behavior_list;
    behavior_list.insert(PB_SwitchOn,  tr(SWITCH_ON_TEXT));
    behavior_list.insert(PB_SwitchOff, tr(SWITCH_OFF_TEXT));
    behavior_list.insert(PB_UsePreset, tr(USE_PRESET_TEXT));

    _ui->cmbx_reaction_performer_behavior_type->addItems(behavior_list);
    _ui->cmbx_reverse_performer_behavior_type->addItems(behavior_list);

    _reaction_relations << "event_id" << "behavior_preset_list";


    connect(_ui->listWidget_reactions, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_reaction_alias,                       SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reaction_event,                     SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reaction_performer_type,            SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reaction_performer_alias,           SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reaction_performer_behavior_type,   SIGNAL(activated(int)),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reaction_performer_behavior_preset, SIGNAL(activated(int)),      SLOT(sltUpdateUI()));

    connect(_ui->chbx_state_ad,              SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_armed,           SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_configuring,     SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_da,              SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_disarmed,        SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_forced_disarmed, SIGNAL(clicked()), SLOT(sltUpdateUI()));
    connect(_ui->chbx_state_partial_armed,   SIGNAL(clicked()), SLOT(sltUpdateUI()));

    connect(_ui->cmbx_reaction_performer_type, SIGNAL(activated(int)),
            SLOT(sltUpdateDependentComboboxes(int)));
    connect(_ui->cmbx_reaction_event, SIGNAL(activated(int)), SLOT(sltUpdateDependentPerformers()));

    connect(_ui->btn_add_apply,  SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));

    connect(_ui->chbx_reversible_flag,                   SIGNAL(clicked()),      SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reverse_performer_behavior_type,   SIGNAL(activated(int)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_reverse_performer_behavior_preset, SIGNAL(activated(int)), SLOT(sltUpdateUI()));

    updatePerformerTypeCombobox();

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

ReactionsManager::~ReactionsManager()
{
    delete _ui;
}

void ReactionsManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add reaction"));
        _ui->btn_del_cancel->setText(tr("Delete reaction"));
    }

    _ui->listWidget_reactions->setEnabled(!modified);
    _bModified = modified;
}

void ReactionsManager::fillForm()
{
    t_Reaction_ptr pReaction = currentReaction();

    if (pReaction.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    _ui->le_reaction_alias->setText(pReaction->_alias);
    _ui->cmbx_reaction_performer_behavior_type->setCurrentIndex(pReaction->_performer_behavior);

    setEventAtCombobox(pReaction);

    setPerformerTypeAtCombobox(pReaction->_performer_type);
    setPerformerAliasAtCombobox(pReaction);

    setBehaviorPresetAtCombobox(pReaction);

    _ui->chbx_reversible_flag->setChecked(pReaction->_bReversible);
    updateUiReversibleRelated();

    actualizeAvailableStates();

    setModified(false);
}

void ReactionsManager::clearForm()
{
    _ui->le_reaction_alias->clear();
    _ui->cmbx_reaction_performer_behavior_type->setCurrentIndex(-1);
    _ui->cmbx_reaction_performer_type->setCurrentIndex(-1);
    _ui->cmbx_reaction_performer_alias->clear();
    _ui->cmbx_reaction_event->setCurrentIndex(-1);
    _ui->cmbx_reaction_performer_behavior_preset->setCurrentIndex(-1);

    _ui->chbx_reversible_flag->setChecked(false);
    clearReversibleRelated();

    clearStatesCheckboxes();
}

void ReactionsManager::saveForm(t_Reaction_ptr &pReaction)
{
    if (pReaction.isNull())
        return;

    pReaction->_alias              = _ui->le_reaction_alias->text();
    pReaction->_performer_behavior = _ui->cmbx_reaction_performer_behavior_type->currentIndex();

    pReaction->_bReversible = _ui->chbx_reversible_flag->isChecked();
    pReaction->_reverse_performer_behavior = _ui->cmbx_reverse_performer_behavior_type->currentIndex();

    pReaction->_event              = getEventFromCombobox();
    pReaction->_performer_type     = getPerformerTypeFromCombobox();
    pReaction->_performer_id       = getPerformerIdFromAliasCombobox();

    // --------------------------------------------------------------------------------------
    /* save major and reverse behavior preset */
    // --------------------------------------------------------------------------------------
    pReaction->_behavior_preset_list.clear();

    t_BehaviorPreset_ptr pBehaviorPreset = getBehaviorPresetFromCombobox();
    if (!pBehaviorPreset.isNull())
        pReaction->_behavior_preset_list.insert(pBehaviorPreset->_id, pBehaviorPreset);

    pBehaviorPreset = getReverseBehaviorPresetFromCombobox();
    if (!pBehaviorPreset.isNull())
        pReaction->_behavior_preset_list.insert(pBehaviorPreset->_id, pBehaviorPreset);
    // --------------------------------------------------------------------------------------

    QByteArray valid_states(STATE_MAX, 0);
    valid_states[STATE_DISARMED_ING]        = _ui->chbx_state_ad->isChecked();
    valid_states[STATE_ARMED]               = _ui->chbx_state_armed->isChecked();
    valid_states[STATE_CONFIGURING]         = _ui->chbx_state_configuring->isChecked();
    valid_states[STATE_ARMED_ING]           = _ui->chbx_state_da->isChecked();
    valid_states[STATE_DISARMED]            = _ui->chbx_state_disarmed->isChecked();
    valid_states[STATE_DISARMED_FORCED]     = _ui->chbx_state_forced_disarmed->isChecked();
    valid_states[STATE_ARMED_PARTIAL]       = _ui->chbx_state_partial_armed->isChecked();

    pReaction->_valid_states = valid_states;
}

bool ReactionsManager::canBeDeleted(const t_Reaction_ptr &pReaction)
{
    Q_UNUSED(pReaction)
    return true;
}

inline QListWidgetItem *ReactionsManager::currentItem()
{
    return _ui->listWidget_reactions->currentItem();
}

inline t_Reaction_ptr ReactionsManager::currentReaction()
{
    if (!currentItem())
        return t_Reaction_ptr();

    return currentItem()->data(DataRole).value<t_Reaction_ptr>();
}

inline void ReactionsManager::updatePerformerTypeCombobox()
{
    QComboBox *c = _ui->cmbx_reaction_performer_type;
    c->clear();

    c->insertItem(PerfType_Bell,  tr("Bell"),  OBJ_BELL);
    c->insertItem(PerfType_Led,   tr("Led"),   OBJ_LED);
    c->insertItem(PerfType_Relay, tr("Relay"), OBJ_RELAY);

    c->setCurrentIndex(-1);
}

inline void ReactionsManager::setPerformerTypeAtCombobox(int performerType)
{
    int index = _ui->cmbx_reaction_performer_type->findData(performerType);
    _ui->cmbx_reaction_performer_type->setCurrentIndex(index);
    updatePerformerAliasCombobox(performerType);
}

inline int ReactionsManager::getPerformerTypeFromCombobox()
{
    QVariant v = _ui->cmbx_reaction_performer_type->itemData(
                                            _ui->cmbx_reaction_performer_type->currentIndex());
    if (v.isValid())
        return v.toInt();
    else
        return SEPARATOR_1;
}

inline void ReactionsManager::updatePerformerAliasCombobox(int performerType)
{
    t_ArmingGroup_ptr pTriggerGroup;
    bool bArmingPerformer = false;
    t_Event_ptr pTrigger = getEventFromCombobox();
    if (pTrigger) {
        switch (pTrigger->_emitter_type) {
        case OBJ_ARMING_GROUP:
        {
            bArmingPerformer = true;
            t_ArmingGroup_ptr pGroup(new t_ArmingGroup);
            pGroup->_id = pTrigger->_emitter_id;
            qx::dao::fetch_by_id(pGroup);
            pTriggerGroup = pGroup;
            break;
        }
        case OBJ_ZONE:
        {
            t_Zone_ptr pZone(new t_Zone);
            pZone->_id = pTrigger->_emitter_id;
            qx::dao::fetch_by_id_with_relation("arming_group_id", pZone);
            pTriggerGroup = pZone->_arming_group;
            break;
        }
        case OBJ_TOUCHMEMORY_CODE:
        {
            t_Key_ptr pKey(new t_Key);
            pKey->_id = pTrigger->_emitter_id;
            qx::dao::fetch_by_id_with_relation("arming_group_id", pKey);
            pTriggerGroup = pKey->_arming_group;
            break;
        }
        }
    }

    QComboBox *c = _ui->cmbx_reaction_performer_alias;
    c->clear();

    switch (performerType) {
    case OBJ_RELAY:
    {
        t_RelayX list;
        qx::dao::fetch_all(list);
        t_Relay_ptr pRelay;
        _foreach (pRelay, list) {
            if (pTrigger) {
                if (bArmingPerformer && t_Relay::RelayLoad_ArmingLed == pRelay->_loadType) {
                    c->addItem(pRelay->_alias, (int)pRelay->_id);
                }
                else if (!bArmingPerformer && t_Relay::RelayLoad_ArmingLed != pRelay->_loadType) {
                    if (pRelay->_pGroup && pTriggerGroup) {
                        if (pRelay->_pGroup->_id == pTriggerGroup->_id)
                            c->addItem(pRelay->_alias, (int)pRelay->_id);
                    }
                    else
                        c->addItem(pRelay->_alias, (int)pRelay->_id);
                }
            }
            else {
                c->addItem(pRelay->_alias, (int)pRelay->_id);
            }
        }
        break;
    }
    case OBJ_LED:
    {
        t_LedX list;
        qx::dao::fetch_all(list);
        t_Led_ptr pLed;
        _foreach (pLed, list) {
            if (pTrigger) {
                if (bArmingPerformer && pLed->_bArmingLed) {
                    c->addItem(pLed->_alias, pLed->_id);
                }
                else if (!bArmingPerformer && !pLed->_bArmingLed) {
                    if (pLed->_pGroup && pTriggerGroup) {
                        if (pLed->_pGroup->_id == pTriggerGroup->_id)
                            c->addItem(pLed->_alias, (int)pLed->_id);
                    }
                    else
                        c->addItem(pLed->_alias, pLed->_id);
                }
            }
            else {
                c->addItem(pLed->_alias, pLed->_id);
            }
        }
        break;
    }
    case OBJ_BELL:
    {
        if (!bArmingPerformer) {
            t_BellX list;
            qx::dao::fetch_all(list);
            t_Bell_ptr pBell;
            _foreach (pBell, list) {
                c->addItem(pBell->_alias, pBell->_id);
            }
        }
        break;
    }
    default:
        break;
    }

    if (c->count())
        c->setCurrentIndex(0);
}

inline void ReactionsManager::setPerformerAliasAtCombobox(const t_Reaction_ptr &pReaction)
{
    int index = _ui->cmbx_reaction_performer_alias->findData(pReaction->_performer_id);
    _ui->cmbx_reaction_performer_alias->setCurrentIndex(index);
}

inline int ReactionsManager::getPerformerIdFromAliasCombobox()
{
    int index = _ui->cmbx_reaction_performer_alias->currentIndex();
    return _ui->cmbx_reaction_performer_alias->itemData(index).toInt();
}

inline void ReactionsManager::updateBehaviorPresetCombobox()
{
    t_BehaviorPresetX list;
    qx::dao::fetch_all(list);

    QComboBox *c = _ui->cmbx_reaction_performer_behavior_preset;
    c->clear();

    t_BehaviorPreset_ptr pPreset;
    _foreach(pPreset, list) {
        c->addItem(pPreset->_alias);

        // -- set data to the last added item
        int index =  c->count() - 1;
        c->setItemData(index, QVariant::fromValue(pPreset->_id), IdRole);
        c->setItemData(index, QVariant::fromValue(pPreset),      DataRole);
    }

    c->setCurrentIndex(-1);


    // --------------------------------------------------------------------------------
    /* reverse-related behavior */
    c = _ui->cmbx_reverse_performer_behavior_preset;
    c->clear();

    t_BehaviorPreset_ptr pPreset_reverse;
    _foreach(pPreset_reverse, list) {
        c->addItem(pPreset_reverse->_alias);

        // -- set data to the last added item
        int index =  c->count() - 1;
        c->setItemData(index, QVariant::fromValue(pPreset_reverse->_id), IdRole);
        c->setItemData(index, QVariant::fromValue(pPreset_reverse),      DataRole);
    }

    c->setCurrentIndex(-1);
    // --------------------------------------------------------------------------------
}

inline void ReactionsManager::setBehaviorPresetAtCombobox(const t_Reaction_ptr &pReaction)
{
    int index = -1;
    if (pReaction->_behavior_preset_list.size() > 0 &&  // if valid data is on 0 position...
            0 == _ui->cmbx_reaction_performer_behavior_type->currentText().compare(tr(USE_PRESET_TEXT)))  // ...and the behavior is needed as major
    {
        t_BehaviorPreset_ptr pBehaviorPreset = pReaction->_behavior_preset_list.getByIndex(0);
        if (!pBehaviorPreset.isNull())
            index = _ui->cmbx_reaction_performer_behavior_preset->findData(
                                                                pBehaviorPreset->_id,
                                                                IdRole);
    }
    _ui->cmbx_reaction_performer_behavior_preset->setCurrentIndex(index);
}

inline t_BehaviorPreset_ptr ReactionsManager::getBehaviorPresetFromCombobox()
{
    QVariant v = _ui->cmbx_reaction_performer_behavior_preset->itemData(
                                    _ui->cmbx_reaction_performer_behavior_preset->currentIndex(),
                                    DataRole);
    if (v.isValid())
        return v.value<t_BehaviorPreset_ptr>();
    else
        return t_BehaviorPreset_ptr();
}

inline void ReactionsManager::setReverseBehaviorPresetAtCombobox(const t_Reaction_ptr &pReaction)
{
    int index = -1;
    int pos = -1;

    if (pReaction->_behavior_preset_list.size() == 2) {      // if valid data is on 1 position => use it always
        pos = 1;
    }
    else if (pReaction->_behavior_preset_list.size() == 1 &&     // if valid data is on 0 position...
             0 == _ui->cmbx_reverse_performer_behavior_type->currentText().compare(tr(USE_PRESET_TEXT)))  // ...and the behavior is needed as reverse
    {
        pos = 0;
    }

    if (-1 != pos) {
        t_BehaviorPreset_ptr pBehaviorPreset = pReaction->_behavior_preset_list.getByIndex(pos);
        if (!pBehaviorPreset.isNull())
            index = _ui->cmbx_reverse_performer_behavior_preset->findData(
                                                                pBehaviorPreset->_id,
                                                                IdRole);
    }

    _ui->cmbx_reverse_performer_behavior_preset->setCurrentIndex(index);
}

inline t_BehaviorPreset_ptr ReactionsManager::getReverseBehaviorPresetFromCombobox()
{
    QVariant v = _ui->cmbx_reverse_performer_behavior_preset->itemData(
                                    _ui->cmbx_reverse_performer_behavior_preset->currentIndex(),
                                    DataRole);
    if (v.isValid())
        return v.value<t_BehaviorPreset_ptr>();
    else
        return t_BehaviorPreset_ptr();
}

inline void ReactionsManager::updateEventCombobox()
{
    t_EventX list;
    qx::dao::fetch_all(list);

    QComboBox *c = _ui->cmbx_reaction_event;
    c->clear();

    t_Event_ptr pEvent;
    _foreach(pEvent, list) {
        c->addItem(pEvent->_alias);

        // -- set data to the last added item
        int index =  c->count() - 1;
        c->setItemData(index, QVariant::fromValue(pEvent->_id), IdRole);
        c->setItemData(index, QVariant::fromValue(pEvent),      DataRole);
    }

    c->setCurrentIndex(-1);
}

inline void ReactionsManager::setEventAtCombobox(const t_Reaction_ptr &pReaction)
{
    int index = -1;
    if (!pReaction->_event.isNull())
        index = _ui->cmbx_reaction_event->findData(pReaction->_event->_id,
                                                   IdRole);
    _ui->cmbx_reaction_event->setCurrentIndex(index);
}

inline t_Event_ptr ReactionsManager::getEventFromCombobox()
{
    QVariant v = _ui->cmbx_reaction_event->itemData(
                                    _ui->cmbx_reaction_event->currentIndex(),
                                    DataRole);
    if (v.isValid())
        return v.value<t_Event_ptr>();
    else
        return t_Event_ptr();
}


void ReactionsManager::clearReversibleRelated()
{
    _ui->label_reverse_behavior->setEnabled(false);
    _ui->cmbx_reverse_performer_behavior_type->setEnabled(false);
    _ui->cmbx_reverse_performer_behavior_preset->setEnabled(false);

    _ui->cmbx_reverse_performer_behavior_type->setCurrentIndex(-1);
    _ui->cmbx_reverse_performer_behavior_preset->setCurrentIndex(-1);
}

void ReactionsManager::fillReversibleRelated()
{
    _ui->label_reverse_behavior->setEnabled(true);
    _ui->cmbx_reverse_performer_behavior_type->setEnabled(true);
    _ui->cmbx_reverse_performer_behavior_preset->setEnabled(true);

    t_Reaction_ptr pReaction = currentReaction();
    if (pReaction.isNull())
        return;

    _ui->cmbx_reverse_performer_behavior_type->setCurrentIndex(pReaction->_reverse_performer_behavior);
    setReverseBehaviorPresetAtCombobox(pReaction);
}

void ReactionsManager::updateUiReversibleRelated()
{
    if (_ui->chbx_reversible_flag->isChecked()) {
        fillReversibleRelated();
    } else {
        clearReversibleRelated();
    }
}

void ReactionsManager::actualizeAvailableStates()
{
    int performer_type_idx = -1;
    t_Event_ptr pEvent  = getEventFromCombobox();

    if (!pEvent.isNull()) {
        const int event_emitter_type = getEventFromCombobox()->_emitter_type;
        switch (event_emitter_type) {
        case OBJ_BUTTON:
        case OBJ_ARMING_GROUP:
            goto set_all_states;
        case OBJ_TOUCHMEMORY_CODE:
        case OBJ_ZONE:
        case OBJ_INTERNAL_TIMER:
        default:
            performer_type_idx = _ui->cmbx_reaction_performer_type->currentIndex();
        }
    }

    switch (performer_type_idx) {
    case -1:    // no item selected at the combobox
        clearStatesCheckboxes();
        break;
    case PerfType_Bell:
        goto set_all_states;
    case PerfType_Led:
    {
        t_Led_ptr pLed(new t_Led);
        pLed->_id = getPerformerIdFromAliasCombobox();
        qx::dao::fetch_by_id_with_relation("group_id", pLed);
        if (pLed->_pGroup.isNull())
            goto set_all_states;
        fillStatesCheckboxes();
        break;
    }
    case PerfType_Relay:
    {
        t_Relay_ptr pRelay(new t_Relay());
        pRelay->_id = getPerformerIdFromAliasCombobox();
        qx::dao::fetch_by_id_with_relation("group_id", pRelay);
        if (pRelay->_pGroup.isNull())
            goto set_all_states;
        fillStatesCheckboxes();
        break;
    }
    case PerfType_TimerStart:
    case PerfType_TimerStop:
    default:
        goto set_all_states;
    }

    return;

set_all_states:
    _ui->groupBox_valid_states->setEnabled(false);

    _ui->chbx_state_ad->setChecked(true);
    _ui->chbx_state_armed->setChecked(true);
    _ui->chbx_state_configuring->setChecked(true);
    _ui->chbx_state_da->setChecked(true);
    _ui->chbx_state_disarmed->setChecked(true);
    _ui->chbx_state_forced_disarmed->setChecked(true);
    _ui->chbx_state_partial_armed->setChecked(true);
}

void ReactionsManager::fillStatesCheckboxes()
{
    t_Reaction_ptr pReaction = currentReaction();
    if (pReaction.isNull()) {
        clearStatesCheckboxes();
        return;
    }

    if (STATE_MAX == pReaction->_valid_states.size())
    {
        _ui->groupBox_valid_states->setEnabled(true);

        _ui->chbx_state_ad->setChecked(             pReaction->_valid_states.at(STATE_DISARMED_ING)   );
        _ui->chbx_state_armed->setChecked(          pReaction->_valid_states.at(STATE_ARMED)          );
        _ui->chbx_state_configuring->setChecked(    pReaction->_valid_states.at(STATE_CONFIGURING)    );
        _ui->chbx_state_da->setChecked(             pReaction->_valid_states.at(STATE_ARMED_ING)      );
        _ui->chbx_state_disarmed->setChecked(       pReaction->_valid_states.at(STATE_DISARMED)       );
        _ui->chbx_state_forced_disarmed->setChecked(pReaction->_valid_states.at(STATE_DISARMED_FORCED));
        _ui->chbx_state_partial_armed->setChecked(  pReaction->_valid_states.at(STATE_ARMED_PARTIAL)  );
    }
}

void ReactionsManager::clearStatesCheckboxes()
{
    _ui->groupBox_valid_states->setEnabled(false);

    _ui->chbx_state_ad->setChecked(             false);
    _ui->chbx_state_armed->setChecked(          false);
    _ui->chbx_state_configuring->setChecked(    false);
    _ui->chbx_state_da->setChecked(             false);
    _ui->chbx_state_disarmed->setChecked(       false);
    _ui->chbx_state_forced_disarmed->setChecked(false);
    _ui->chbx_state_partial_armed->setChecked(  false);
}


void ReactionsManager::sltUpdateList()
{
    QListWidget *lw = _ui->listWidget_reactions;
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? lw->currentRow()
                        : -1;

    qx::dao::fetch_all_with_relation(_reaction_relations, _reaction_list);

    lw->blockSignals(true);
    lw->clear();
    lw->blockSignals(false);
    clearForm();
    setModified(false);

    t_Reaction_ptr pReaction;
    _foreach (pReaction, _reaction_list) {
        QListWidgetItem *item = new QListWidgetItem(pReaction->_alias);
        item->setData(IdRole,   QVariant::fromValue(pReaction->_id));
        item->setData(DataRole, QVariant::fromValue(pReaction));
        lw->addItem(item);
    }

    updateBehaviorPresetCombobox();
    updateEventCombobox();

    // -- select appropriate row
    if (_bAdding || currentRow >= lw->count())
        lw->setCurrentRow(lw->count() - 1);
    else if (-1 == currentRow && lw->count())
        lw->setCurrentRow(0);
    else /*if (currentRow < lw->count())*/
        lw->setCurrentRow(currentRow);
}

void ReactionsManager::sltUpdateUI()
{
    QObject * const sndr = sender();
    if (sndr == _ui->listWidget_reactions) {
        fillForm();
    } else {
        setModified(true);

        /* actualize major user-defined behavior */
        if (sndr == _ui->cmbx_reaction_performer_behavior_preset)
            _ui->cmbx_reaction_performer_behavior_type->setCurrentIndex(PB_UsePreset);

        if (sndr == _ui->cmbx_reaction_performer_behavior_type) {
            switch (_ui->cmbx_reaction_performer_behavior_type->currentIndex()) {
            case PB_UsePreset:
                _ui->cmbx_reaction_performer_behavior_preset->setCurrentIndex(0);
                break;
            default:
                _ui->cmbx_reaction_performer_behavior_preset->setCurrentIndex(-1);
            }
        }

        // --------------------------------------------------------------------------------
        /* actualize reverse-related behavior */
        if (sndr == _ui->cmbx_reverse_performer_behavior_preset)
            _ui->cmbx_reverse_performer_behavior_type->setCurrentIndex(PB_UsePreset);

        if (sndr == _ui->cmbx_reverse_performer_behavior_type) {
            switch (_ui->cmbx_reverse_performer_behavior_type->currentIndex()) {
            case PB_UsePreset:
                _ui->cmbx_reverse_performer_behavior_preset->setCurrentIndex(0);
                break;
            default:
                _ui->cmbx_reverse_performer_behavior_preset->setCurrentIndex(-1);
            }
        }
        // --------------------------------------------------------------------------------


        /* enable/disable reverse-related behavior widgets */
        if (sndr == _ui->chbx_reversible_flag)
            updateUiReversibleRelated();


        /* if the trigger event or the performer was changed */
        if (sndr == _ui->cmbx_reaction_event ||
                sndr == _ui->cmbx_reaction_performer_type ||
                sndr == _ui->cmbx_reaction_performer_alias)
        {
            actualizeAvailableStates();
        }

        // TODO: check if any user defined behavior exists when user try
        //       to select "Use preset:" category and fix his selection or show a warning message
    }
}

void ReactionsManager::sltUpdateDependentComboboxes(int performerTypeIndex)
{
    int performerType = _ui->cmbx_reaction_performer_type->itemData(performerTypeIndex).toInt();
    updatePerformerAliasCombobox(performerType);
    actualizeAvailableStates();
}

void ReactionsManager::sltUpdateDependentPerformers()
{
    int performerTypeIdx = _ui->cmbx_reaction_performer_type->currentIndex();
    sltUpdateDependentComboboxes(performerTypeIdx);
}

void ReactionsManager::sltAddApply()
{
    if (isModified()) {        // apply
        if (!_bAdding && currentItem()) {    // apply editing
            t_Reaction_ptr pReaction = currentReaction();
            saveForm(pReaction);

            qx::dao::update_with_relation(_reaction_relations, pReaction);

            int row = _ui->listWidget_reactions->currentRow();
            sltUpdateList();
            _ui->listWidget_reactions->setCurrentRow(row);

        } else {            // apply adding
            t_Reaction_ptr pReaction(new t_Reaction());
            saveForm(pReaction);

            qx::dao::insert_with_relation(_reaction_relations, pReaction);

            sltUpdateList();
            _bAdding = false;
            _ui->listWidget_reactions->setCurrentRow(_ui->listWidget_reactions->count() - 1);
        }
        Q_EMIT snlListChanged();
        setModified(false);
    } else {               // add new
        clearForm();
        _bAdding = true;
        _ui->listWidget_reactions->clearSelection();
        setModified(true);
        _ui->le_reaction_alias->setFocus();
    }
}

void ReactionsManager::sltDeleteCancel()
{
    _bAdding = false;

    if (currentItem()) {
        if (isModified()) // cancel
        {
            _ui->listWidget_reactions->setCurrentItem(currentItem());  // => current item will be selected
            fillForm();
        }
        else          // delete
        {
            t_Reaction_ptr pReaction = currentReaction();
            if (canBeDeleted(pReaction))
            {
                qx::dao::delete_by_id(pReaction);

                /* clear all related entries in the helper table */
                qx_query q("DELETE FROM s_Reaction_BehaviorPreset WHERE behavior_preset_id = :id");
                q.bind(":id", pReaction->_id);
                qx::dao::call_query(q);

                sltUpdateList();
                Q_EMIT snlListChanged();
            }
        }
    } else {
        clearForm();
    }

    setModified(false);
}
