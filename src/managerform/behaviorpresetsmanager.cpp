#include "precompiled.h"

#include "behaviorpresetsmanager.h"
#include "ui_behaviorpresetssmanager.h"

#include "bo/t_reaction.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


#define ONE_PULSE_TEXT          QT_TRANSLATE_NOOP("BehaviorPresetsManager", "Single pulse")
#define ONE_BATCH_TEXT          QT_TRANSLATE_NOOP("BehaviorPresetsManager", "Single batch")
#define INFINITE_PULSES_TEXT    QT_TRANSLATE_NOOP("BehaviorPresetsManager", "Infinite pulses")
#define INFINITE_BATCHES_TEXT   QT_TRANSLATE_NOOP("BehaviorPresetsManager", "Infinite batches")


bool BehaviorPresetsManager::_bModified = false;


BehaviorPresetsManager::BehaviorPresetsManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::BehaviorPresetsManager())
{
    _ui->setupUi(this);

    _ui->splitter_behavior_presets->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_behavior_presets->setStretchFactor(1, 1);

    _ui->le_behavior_preset_alias->setMaxLength(ALIAS_LEN);

    QStringList list_types;
    list_types << tr(ONE_PULSE_TEXT) << tr(ONE_BATCH_TEXT) << tr(INFINITE_PULSES_TEXT)
               << tr(INFINITE_BATCHES_TEXT);
    _ui->cmbx_type->addItems(list_types);

    actualizeUI(_ui->cmbx_type->currentIndex());

    connect(_ui->listWidget_behavior_presets, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_behavior_preset_alias, SIGNAL(textEdited(QString)),  SLOT(sltUpdateUI()));
    connect(_ui->cmbx_type,                SIGNAL(activated(int)),       SLOT(sltUpdateUI()));
    connect(_ui->spbx_pulses_in_batch,     SIGNAL(valueChanged(int)),    SLOT(sltUpdateUI()));
    connect(_ui->dspbx_pulse_len,          SIGNAL(valueChanged(double)), SLOT(sltUpdateUI()));
    connect(_ui->dspbx_pulse_pause_len,    SIGNAL(valueChanged(double)), SLOT(sltUpdateUI()));
    connect(_ui->dspbx_batch_pause_len,    SIGNAL(valueChanged(double)), SLOT(sltUpdateUI()));

    connect(_ui->btn_add_apply,  SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));


    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

BehaviorPresetsManager::~BehaviorPresetsManager()
{
    delete _ui;
}

void BehaviorPresetsManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add preset"));
        _ui->btn_del_cancel->setText(tr("Delete preset"));
    }

    _ui->listWidget_behavior_presets->setEnabled(!modified);
    _bModified = modified;
}

void BehaviorPresetsManager::fillForm()
{
    t_BehaviorPreset_ptr pPreset = currentBehaviorPreset();

    if (pPreset.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    actualizeUI(pPreset->_type);

    blockSpinboxesSignals(true);
    _ui->le_behavior_preset_alias->setText(pPreset->_alias);
    _ui->cmbx_type->setCurrentIndex(pPreset->_type);
    _ui->spbx_pulses_in_batch->setValue(pPreset->_pulses_in_batch);
    _ui->dspbx_pulse_len->setValue(pPreset->_pulse_len);
    _ui->dspbx_pulse_pause_len->setValue(pPreset->_pause_len);
    _ui->dspbx_batch_pause_len->setValue(pPreset->_batch_pause_len);
    blockSpinboxesSignals(false);

    setModified(false);
}

void BehaviorPresetsManager::clearForm()
{
    blockSpinboxesSignals(true);
    _ui->le_behavior_preset_alias->clear();
    _ui->cmbx_type->setCurrentIndex(0);
    _ui->spbx_pulses_in_batch->setValue(0);
    _ui->dspbx_pulse_len->setValue(0);
    _ui->dspbx_pulse_pause_len->setValue(0);
    _ui->dspbx_batch_pause_len->setValue(0);
    blockSpinboxesSignals(false);
}

void BehaviorPresetsManager::saveForm(t_BehaviorPreset_ptr &pPreset)
{
    if (pPreset.isNull())
        return;

    pPreset->_alias = _ui->le_behavior_preset_alias->text();
    pPreset->_type = _ui->cmbx_type->currentIndex();
    pPreset->_pulses_in_batch = _ui->spbx_pulses_in_batch->value();
    pPreset->_pulse_len = _ui->dspbx_pulse_len->value();
    pPreset->_pause_len = _ui->dspbx_pulse_pause_len->value();
    pPreset->_batch_pause_len = _ui->dspbx_batch_pause_len->value();
}

bool BehaviorPresetsManager::canBeDeleted(const t_BehaviorPreset_ptr &pPreset)
{
    if (pPreset.isNull())
        return false;

    qx_query q("SELECT * FROM s_Reaction_BehaviorPreset WHERE behavior_preset_id = :id");
    q.bind(":id", pPreset->_id);
    qx::dao::call_query(q);
    const long usedBy = q.getSqlResultRowCount();
    return 0 == usedBy;
}

inline QListWidgetItem *BehaviorPresetsManager::currentItem()
{
    return _ui->listWidget_behavior_presets->currentItem();
}

t_BehaviorPreset_ptr BehaviorPresetsManager::currentBehaviorPreset()
{
    if (!currentItem())
        return t_BehaviorPreset_ptr();

    return currentItem()->data(DataRole).value<t_BehaviorPreset_ptr>();
}

void BehaviorPresetsManager::actualizeUI(int behaviorType)
{
    blockSpinboxesSignals(true);
    QComboBox *pCmbx = _ui->cmbx_type;
    if (tr(ONE_PULSE_TEXT) == pCmbx->itemText(behaviorType)) {
        _ui->dspbx_batch_pause_len->setMinimum(0.);
        _ui->dspbx_batch_pause_len->setEnabled(false);
        _ui->dspbx_batch_pause_len->setValue(0);

        _ui->spbx_pulses_in_batch->setMinimum(1);
        _ui->spbx_pulses_in_batch->setEnabled(false);
        _ui->spbx_pulses_in_batch->setValue(1);

        _ui->dspbx_pulse_pause_len->setEnabled(false);
        _ui->dspbx_pulse_pause_len->setValue(0);
    }
    else if (tr(ONE_BATCH_TEXT) == pCmbx->itemText(behaviorType)) {
        _ui->dspbx_batch_pause_len->setMinimum(0.);
        _ui->dspbx_batch_pause_len->setEnabled(false);
        _ui->dspbx_batch_pause_len->setValue(0);

        _ui->spbx_pulses_in_batch->setMinimum(1);
        _ui->spbx_pulses_in_batch->setEnabled(true);

        _ui->dspbx_pulse_pause_len->setEnabled(true);
    }
    else if (tr(INFINITE_PULSES_TEXT) == pCmbx->itemText(behaviorType)) {
        _ui->dspbx_batch_pause_len->setMinimum(0.);
        _ui->dspbx_batch_pause_len->setEnabled(false);
        _ui->dspbx_batch_pause_len->setValue(0);

        _ui->spbx_pulses_in_batch->setMinimum(0);
        _ui->spbx_pulses_in_batch->setEnabled(false);
        _ui->spbx_pulses_in_batch->setValue(0);

        _ui->dspbx_pulse_pause_len->setEnabled(true);
    }
    else if (tr(INFINITE_BATCHES_TEXT) == pCmbx->itemText(behaviorType)) {
        _ui->dspbx_batch_pause_len->setMinimum(0.1);
        _ui->dspbx_batch_pause_len->setEnabled(true);

        _ui->spbx_pulses_in_batch->setMinimum(1);
        _ui->spbx_pulses_in_batch->setEnabled(true);

        _ui->dspbx_pulse_pause_len->setEnabled(true);
    }
    blockSpinboxesSignals(false);
}

inline void BehaviorPresetsManager::blockSpinboxesSignals(bool block)
{
    _ui->spbx_pulses_in_batch->blockSignals(block);
    _ui->dspbx_batch_pause_len->blockSignals(block);
    _ui->dspbx_pulse_len->blockSignals(block);
    _ui->dspbx_pulse_pause_len->blockSignals(block);
}

void BehaviorPresetsManager::sltUpdateList()
{
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_behavior_presets->currentRow()
                        : -1;

    qx::dao::fetch_all(_behavior_preset_list);

    _ui->listWidget_behavior_presets->blockSignals(true);
    _ui->listWidget_behavior_presets->clear();
    _ui->listWidget_behavior_presets->blockSignals(false);
    clearForm();
    setModified(false);

    t_BehaviorPreset_ptr pPreset;
    _foreach (pPreset, _behavior_preset_list) {
        QListWidgetItem *item = new QListWidgetItem(pPreset->_alias);
        item->setData(IdRole,   QVariant::fromValue(pPreset->_id));
        item->setData(DataRole, QVariant::fromValue(pPreset));
        _ui->listWidget_behavior_presets->addItem(item);
    }

    // -- select appropriate row
    if (_bAdding || currentRow >= _ui->listWidget_behavior_presets->count())
        _ui->listWidget_behavior_presets->setCurrentRow(_ui->listWidget_behavior_presets->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_behavior_presets->count())
        _ui->listWidget_behavior_presets->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_behavior_presets->count())*/
        _ui->listWidget_behavior_presets->setCurrentRow(currentRow);
}

void BehaviorPresetsManager::sltUpdateUI()
{
    QObject * const obj = sender();
    if (obj == _ui->listWidget_behavior_presets) {
        fillForm();
    } else {
        setModified(true);

        if (obj == _ui->cmbx_type)
            actualizeUI(_ui->cmbx_type->currentIndex());
    }
}

void BehaviorPresetsManager::sltAddApply()
{
    if (isModified()) {        // apply
        if (!_bAdding && currentItem()) {    // apply editing
            t_BehaviorPreset_ptr pPreset = currentBehaviorPreset();
            saveForm(pPreset);

            qx::dao::update_optimized(pPreset);

            int row = _ui->listWidget_behavior_presets->currentRow();
            sltUpdateList();
            _ui->listWidget_behavior_presets->setCurrentRow(row);
        } else {            // apply adding
            t_BehaviorPreset_ptr pPreset(new t_BehaviorPreset());
            saveForm(pPreset);

            qx::dao::insert(pPreset);

            sltUpdateList();
            _bAdding = false;
            _ui->listWidget_behavior_presets->setCurrentRow(_ui->listWidget_behavior_presets->count() - 1);
        }
        Q_EMIT snlListChanged();
        setModified(false);
    } else {               // add new
        clearForm();
        _bAdding = true;
        _ui->listWidget_behavior_presets->clearSelection();
        setModified(true);
        _ui->le_behavior_preset_alias->setFocus();
    }
}

void BehaviorPresetsManager::sltDeleteCancel()
{
    _bAdding = false;

    if (currentItem()) {
        if (isModified()) // cancel
        {
            _ui->listWidget_behavior_presets->setCurrentItem(currentItem());  // => current item will be selected
            fillForm();
        }
        else          // delete
        {
            t_BehaviorPreset_ptr pPreset = currentBehaviorPreset();
            if (canBeDeleted(pPreset))
            {
                qx::dao::delete_by_id(pPreset);
                sltUpdateList();
                Q_EMIT snlListChanged();
            }
        }
    } else {
        clearForm();
    }

    setModified(false);
}
