#include "precompiled.h"

#include "expandersmanager.h"
#include "ui_expandersmanager.h"

#include "bo/t_zone.h"

#include "bo/s_parentunit.h"

#include "configurator_protocol.h"

#include <QTimer>

#include <QxMemLeak.h>


#define TYPE_ZONES_TEXT  QT_TRANSLATE_NOOP("ExpandersManager", "Z - expander ( 8 loops )")
#define TYPE_RELAYS_TEXT QT_TRANSLATE_NOOP("ExpandersManager", "R - expander ( 6 relays )")


bool ExpandersManager::_bModified = false;


ExpandersManager::ExpandersManager(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ExpandersManager()),
    _bAdding(false)
{
    _ui->setupUi(this);

    _ui->splitter_expanders->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_expanders->setStretchFactor(1, 1);

    _ui->le_expander_alias->setMaxLength(ALIAS_LEN);
    _ui->le_expander_uin->setValidator(new QIntValidator(1, 65535, this));

    _relation << "uin_id";

    connect(_ui->listWidget_expanders, SIGNAL(currentRowChanged(int)), this, SLOT(sltUpdateUI()));

    connect(_ui->btn_add_apply, SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));
    connect(_ui->le_expander_uin, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_expander_alias, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->comboBox_expander_type, SIGNAL(activated(int)), SLOT(sltUpdateUI()));

    _ui->comboBox_expander_type->addItems(QStringList() << tr(TYPE_ZONES_TEXT) << tr(TYPE_RELAYS_TEXT));
    clearForm();

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

ExpandersManager::~ExpandersManager()
{
    delete _ui;
}

bool ExpandersManager::isModified()
{
    return _bModified;
}

inline void ExpandersManager::fillForm()
{
    t_Expander_ptr pExpander = currentExpander();

    if (pExpander.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    _ui->le_expander_alias->setText(pExpander->_uin->_palias);
    _ui->le_expander_uin->setText(QString::number(pExpander->_uin->_puin));

    QString temp_type;
    switch (pExpander->_type) {
    case OBJ_RZE:
        temp_type = tr(TYPE_ZONES_TEXT);
        break;
    case OBJ_RAE:
        temp_type = tr(TYPE_RELAYS_TEXT);
        break;
    }
    bool bItemSelected = _ui->listWidget_expanders->currentRow() != -1;
    _ui->comboBox_expander_type->setEnabled(!bItemSelected);
    _ui->comboBox_expander_type->setCurrentIndex(-1);
    _ui->comboBox_expander_type->setCurrentText(temp_type);

    setModified(false);
}

inline void ExpandersManager::clearForm()
{
    _ui->le_expander_alias->clear();
    _ui->le_expander_uin->clear();
    _ui->comboBox_expander_type->setCurrentIndex(-1);
}

inline void ExpandersManager::saveForm(t_Expander_ptr &pExpander)
{
    if (_ui->comboBox_expander_type->currentText() == tr(TYPE_ZONES_TEXT))
        pExpander->_type = OBJ_RZE;
    else if (_ui->comboBox_expander_type->currentText() == tr(TYPE_RELAYS_TEXT))
        pExpander->_type = OBJ_RAE;
    else
        pExpander->_type = -1;

    pExpander->_uin->_ptype = pExpander->_type;
    pExpander->_uin->_puin = _ui->le_expander_uin->text().toInt();
    pExpander->_uin->_palias = _ui->le_expander_alias->text();
}

inline void ExpandersManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add expander"));
        _ui->btn_del_cancel->setText(tr("Delete expander"));
    }
    _ui->listWidget_expanders->setEnabled(!modified);
    _bModified = modified;
}

inline bool ExpandersManager::canBeDeleted(int expanderId)
{
    Q_UNUSED(expanderId)
    return true;
}

bool ExpandersManager::isValidForm()
{
    return _ui->comboBox_expander_type->currentIndex() != -1;
}

QListWidgetItem *ExpandersManager::currentItem()
{
    return _ui->listWidget_expanders->currentItem();
}

t_Expander_ptr ExpandersManager::currentExpander()
{
    if (!currentItem())
        return t_Expander_ptr();

    int id = currentItem()->data(Qt::UserRole).toInt();
    return _expanders_list.getByKey(id);
}

void ExpandersManager::sltUpdateList()
{
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_expanders->currentRow()
                        : -1;

//    qx_query query("SELECT name FROM s_expander_types ORDER BY id ASC");
//    qx::dao::call_query(query);
//    ui->comboBox_expander_type->clear();
//    for (int i = 0; i < query.getSqlResultRowCount(); ++i) {
//        ui->comboBox_expander_type->addItem(query.getSqlResultAt(i,0).toString());
//    }

    qx::dao::fetch_all_with_relation(_relation, _expanders_list);

    _ui->listWidget_expanders->blockSignals(true);
    _ui->listWidget_expanders->clear();
    _ui->listWidget_expanders->blockSignals(false);
    clearForm();
    setModified(false);

    t_Expander_ptr pExpander;
    _foreach (pExpander, _expanders_list) {
        QListWidgetItem *item = new QListWidgetItem(pExpander->_uin->_palias,
                                                    _ui->listWidget_expanders);
        item->setData(Qt::UserRole, QVariant((int)pExpander->_id));
        _ui->listWidget_expanders->addItem(item);
    }

    if (_bAdding || currentRow >= _ui->listWidget_expanders->count())
        _ui->listWidget_expanders->setCurrentRow(_ui->listWidget_expanders->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_expanders->count())
        _ui->listWidget_expanders->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_expanders->count())*/
        _ui->listWidget_expanders->setCurrentRow(currentRow);
}

void ExpandersManager::sltUpdateUI()
{
    QObject *obj = sender();
    if (obj == _ui->listWidget_expanders) {
        fillForm();
    } else {
        setModified(true);
    }
}

void ExpandersManager::sltAddApply()
{
    if (isModified()) {        // apply
        if (!isValidForm())
            return;
        if (!_bAdding && currentItem()) {    // apply editing
            t_Expander_ptr pExpander = currentExpander();
            saveForm(pExpander);

            qx::dao::update_with_relation(_relation, pExpander);

            int row = _ui->listWidget_expanders->currentRow();
            sltUpdateList();
            _ui->listWidget_expanders->setCurrentRow(row);
        } else {            // apply adding
            t_Expander_ptr pExpander (new t_Expander());
            pExpander->_uin = s_ParentUnit_ptr(new s_ParentUnit());
            saveForm(pExpander);

            qx::dao::insert_with_relation(_relation, pExpander);

            // -- actualize _pid field value
            qx::dao::fetch_by_id_with_relation("expander_list", pExpander->_uin);
            if (!pExpander->_uin->_expander_list.empty())
                pExpander->_uin->_pid = pExpander->_uin->_expander_list.getFirst()->_id;
            qx::dao::update_optimized(pExpander->_uin);
            // --

            sltUpdateList();
            _ui->listWidget_expanders->setCurrentRow(_ui->listWidget_expanders->count() - 1);
            _bAdding = false;
        }

        Q_EMIT snlListChanged();
        setModified(false);
    } else {               // add new
        clearForm();
        _bAdding = true;
        _ui->listWidget_expanders->clearSelection();
        setModified(true);
        _ui->le_expander_alias->setFocus();
        _ui->comboBox_expander_type->setEnabled(true);
    }
}

void ExpandersManager::sltDeleteCancel()
{
    if (currentItem()) {
        if (_bModified) {    // cancel
            _ui->listWidget_expanders->setCurrentItem(currentItem());
            fillForm();
        } else {            // delete
            t_Expander_ptr pExpander = currentExpander();
            if (canBeDeleted(pExpander->_id)) {
                qx::dao::delete_by_id(pExpander->_uin);
                qx::dao::delete_by_id(pExpander);
                sltUpdateList();
                Q_EMIT snlListChanged();
            }
        }
    } else {
        clearForm();
    }

    _bAdding = false;
    setModified(false);
    bool bItemSelected = _ui->listWidget_expanders->currentRow() != -1;
    _ui->comboBox_expander_type->setEnabled(!bItemSelected);
}
