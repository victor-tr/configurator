#include "precompiled.h"

#include "etrmanager.h"
#include "ui_etrmanager.h"

#include "bo/s_parentunit.h"

#include <QTimer>

#include "configurator_protocol.h"

#include <QxMemLeak.h>


#define ETR_TYPE_SIMPLE_TEXT        QT_TRANSLATE_NOOP("EtrManager", "Simple ETR")
#define ETR_TYPE_WITH_4_ZONES_TEXT  QT_TRANSLATE_NOOP("EtrManager", "ETR with 4 protection loops")


bool EtrManager::_bModified = false;


EtrManager::EtrManager(QWidget *parent) :
    QWidget(parent),
    _bAdding(false),
    _ui(new Ui::EtrManager())
{
    _ui->setupUi(this);

    _ui->splitter_etr->setSizes(QList<int>() << 180 << 100);
    _ui->splitter_etr->setStretchFactor(1, 1);

    _ui->le_alias_etr->setMaxLength(ALIAS_LEN);
    _ui->le_uin_etr->setValidator(new QIntValidator(1, 65535, this));

    _relation << "uin_id";

    connect(_ui->listWidget_etr, SIGNAL(currentRowChanged(int)), SLOT(sltUpdateUI()));

    connect(_ui->le_alias_etr, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->le_uin_etr, SIGNAL(textEdited(QString)), SLOT(sltUpdateUI()));
    connect(_ui->cmbx_etr_type, SIGNAL(activated(QString)), SLOT(sltUpdateUI()));

    connect(_ui->btn_add_apply, SIGNAL(clicked()), SLOT(sltAddApply()));
    connect(_ui->btn_del_cancel, SIGNAL(clicked()), SLOT(sltDeleteCancel()));

    _ui->cmbx_etr_type->addItems(QStringList() << tr(ETR_TYPE_SIMPLE_TEXT) << tr(ETR_TYPE_WITH_4_ZONES_TEXT));
    clearForm();

    setModified(false);
    QTimer::singleShot(0, this, SLOT(sltUpdateList()));
}

EtrManager::~EtrManager()
{
    delete _ui;
}

bool EtrManager::isModified()
{
    return _bModified;
}

void EtrManager::setModified(bool modified)
{
    if (modified) {
        _ui->btn_add_apply->setText(tr("Apply"));
        _ui->btn_del_cancel->setText(tr("Cancel"));
    } else {
        _ui->btn_add_apply->setText(tr("Add ETR"));
        _ui->btn_del_cancel->setText(tr("Delete ETR"));
    }

    _ui->listWidget_etr->setEnabled(!modified);
    _bModified = modified;
}

void EtrManager::fillForm()
{
    t_Etr_ptr pEtr = currentEtr();

    if (pEtr.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    _ui->le_alias_etr->setText(pEtr->_uin->_palias);
    _ui->le_uin_etr->setText(QString::number(pEtr->_uin->_puin));

    QString temp_type;
    switch (pEtr->_etr_type) {
    case t_Etr::Etr_type_simple:
        temp_type = tr(ETR_TYPE_SIMPLE_TEXT);
        break;
    case t_Etr::Etr_type_with_4_zones:
        temp_type = tr(ETR_TYPE_WITH_4_ZONES_TEXT);
        break;
    }
    bool bItemSelected = _ui->listWidget_etr->currentRow() != -1;
    _ui->cmbx_etr_type->setEnabled(!bItemSelected);
    _ui->cmbx_etr_type->setCurrentIndex(-1);
    _ui->cmbx_etr_type->setCurrentText(temp_type);

    setModified(false);
}

void EtrManager::clearForm()
{
    _ui->le_alias_etr->clear();
    _ui->le_uin_etr->clear();
    _ui->cmbx_etr_type->setCurrentIndex(-1);
}

void EtrManager::saveForm(t_Etr_ptr &pEtr)
{
    if (pEtr.isNull())
        return;

    if (_ui->cmbx_etr_type->currentText() == tr(ETR_TYPE_SIMPLE_TEXT))
        pEtr->_etr_type = t_Etr::Etr_type_simple;
    else if (_ui->cmbx_etr_type->currentText() == tr(ETR_TYPE_WITH_4_ZONES_TEXT))
        pEtr->_etr_type = t_Etr::Etr_type_with_4_zones;
    else
        pEtr->_etr_type = -1;

    pEtr->_uin->_palias = _ui->le_alias_etr->text();
    pEtr->_uin->_puin = _ui->le_uin_etr->text().toInt();
    pEtr->_uin->_ptype = OBJ_ETR;
}

bool EtrManager::canBeDeleted(const t_Etr_ptr &pEtr)
{
    Q_UNUSED(pEtr);
    return true;
}

bool EtrManager::isValidForm()
{
    return _ui->cmbx_etr_type->currentIndex() != -1;
}

inline QListWidgetItem *EtrManager::currentItem()
{
    return _ui->listWidget_etr->currentItem();
}

inline t_Etr_ptr EtrManager::currentEtr()
{
    //Q_ASSERT(currentItem());

    if (!currentItem())
        return t_Etr_ptr();

    int id = currentItem()->data(Qt::UserRole).toInt();
    return _etr_list.getByKey(id);
}

void EtrManager::sltUpdateList()
{
    // -- update keys list
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(metaObject()->className()))
                        ? _ui->listWidget_etr->currentRow()
                        : -1;

    qx::dao::fetch_all_with_relation(_relation, _etr_list);

    _ui->listWidget_etr->blockSignals(true);
    _ui->listWidget_etr->clear();
    _ui->listWidget_etr->blockSignals(false);
    clearForm();
    setModified(false);

    t_Etr_ptr pEtr;
    _foreach(pEtr, _etr_list) {
        QListWidgetItem *item = new QListWidgetItem(pEtr->_uin->_palias);
        item->setData(Qt::UserRole, QVariant((int)pEtr->_id));
        _ui->listWidget_etr->addItem(item);
    }

    if (_bAdding || currentRow >= _ui->listWidget_etr->count())
        _ui->listWidget_etr->setCurrentRow(_ui->listWidget_etr->count() - 1);
    else if (-1 == currentRow && _ui->listWidget_etr->count())
        _ui->listWidget_etr->setCurrentRow(0);
    else /*if (currentRow < _ui->listWidget_etr->count())*/
        _ui->listWidget_etr->setCurrentRow(currentRow);
}

void EtrManager::sltUpdateUI()
{
    QObject *obj = sender();
    if (obj == _ui->listWidget_etr) {
        fillForm();
    } else {
        setModified(true);
    }
}

void EtrManager::sltAddApply()
{
    if (isModified()) {     // apply
        if (!isValidForm())
            return;
        if (_bAdding || !currentItem()) {     // apply adding new
            t_Etr_ptr pEtr(new t_Etr());
            pEtr->_uin = s_ParentUnit_ptr(new s_ParentUnit());
            saveForm(pEtr);

            qx::dao::insert_with_relation(_relation, pEtr);

            // -- actualize _pid field value
            qx::dao::fetch_by_id_with_relation("etr_list", pEtr->_uin);
            if (!pEtr->_uin->_etr_list.empty())
                pEtr->_uin->_pid = pEtr->_uin->_etr_list.getFirst()->_id;
            qx::dao::update_optimized(pEtr->_uin);
            // --

            sltUpdateList();

            _ui->listWidget_etr->setCurrentRow(_ui->listWidget_etr->count() - 1);
            _bAdding = false;
        } else {            // apply editing
            t_Etr_ptr pEtr = currentEtr();
            saveForm(pEtr);

            qx::dao::update_with_relation(_relation, pEtr);

            int currentKeyRow = _ui->listWidget_etr->currentRow();
            sltUpdateList();
            _ui->listWidget_etr->setCurrentRow(currentKeyRow);
        }

        Q_EMIT snlListChanged();
        setModified(false);
    } else {                // add
        clearForm();
        _bAdding = true;
        _ui->listWidget_etr->clearSelection();
        setModified(true);
        _ui->le_alias_etr->setFocus();
        _ui->cmbx_etr_type->setEnabled(true);
    }
}

void EtrManager::sltDeleteCancel()
{
    if (currentItem()) {
        if (isModified()) { // cancel
            _ui->listWidget_etr->setCurrentItem(currentItem());
            fillForm();
        } else {            // delete
            t_Etr_ptr pEtr = currentEtr();
            qx::dao::delete_by_id(pEtr->_uin);
            qx::dao::delete_by_id(pEtr);
            sltUpdateList();
            Q_EMIT snlListChanged();
        }
    } else {
        clearForm();
    }

    _bAdding = false;
    setModified(false);
    bool bItemSelected = _ui->listWidget_etr->currentRow() != -1;
    _ui->cmbx_etr_type->setEnabled(!bItemSelected);
}
