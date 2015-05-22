#include "precompiled.h"
#include "abstractmanager.h"

#include <QListWidgetItem>
#include <QListWidget>

#include <QxMemLeak.h>


template <typename T, typename Traits>
bool AbstractManager<T, Traits>::_bModified = false;


template <typename T, typename Traits>
AbstractManager<T, Traits>::AbstractManager()
{
}

template <typename T, typename Traits>
void AbstractManager<T, Traits>::setModified(bool modified)
{
    setModifiedHook(modified);
    mainListWidget()->setEnabled(!modified);
    _bModified = modified;
}

template <typename T, typename Traits>
void AbstractManager<T, Traits>::fillForm()
{
    type_ptr pElement = currentElement();

    if (pElement.isNull()) {
        clearForm();
        setModified(false);
        return;
    }

    fillFormHook(pElement);
    setModified(false);
}

template <typename T, typename Traits>
inline bool AbstractManager<T, Traits>::canBeDeleted(type_ptr_const_ref pElement)
{
    Q_UNUSED(pElement)
    return true;
}

template <typename T, typename Traits>
void AbstractManager<T, Traits>::updateList(QObject *me)
{
    QListWidget *lw = mainListWidget();
    int currentRow = !sender() || (0 != QString(sender()->metaObject()->className()).compare(
                                              me->metaObject()->className()))
                        ? lw->currentRow()
                        : -1;

    type_list elements_list;

    QStringList relation;
    relation << "parent_id";
    qx::dao::fetch_all_with_relation(relation, elements_list);

    lw->blockSignals(true);
    lw->clear();
    lw->blockSignals(false);
    clearForm();
    setModified(false);

    type_ptr pElement;
    _foreach(pElement, elements_list) {
        QListWidgetItem *item = new QListWidgetItem(pElement->_alias, lw);
        item->setData(IdRole,   QVariant::fromValue(pElement->_id));
        item->setData(DataRole, QVariant::fromValue(pElement));
        lw->addItem(item);
    }

    if (currentRow >= lw->count())
        lw->setCurrentRow(lw->count() - 1);
    else if (-1 == currentRow && lw->count())
        lw->setCurrentRow(0);
    else /*if (currentRow < lw->count())*/
        lw->setCurrentRow(currentRow);
}

template <typename T, typename Traits>
void AbstractManager<T, Traits>::updateUI()
{
    if (sender() == mainListWidget()) {
        fillForm();
    } else {
        if (mainListWidget()->currentRow() != -1)
            setModified(true);
        updateUIHook();
    }
}

template <typename T, typename Traits>
bool AbstractManager<T, Traits>::apply()
{
    QListWidget *lw = mainListWidget();

    if (isModified() && currentItem()) {
        type_ptr pElement = currentElement();
        saveForm(pElement);

        qx::dao::update_optimized(pElement);

        int row = lw->currentRow();
        updateList(sender()); // NOTE: dirty hack
        lw->setCurrentRow(row);
        lw->setFocus();
        return true;
    } else {
        clearForm();
        return false;
    }
}

template <typename T, typename Traits>
void AbstractManager<T, Traits>::cancel()
{
    if (isModified()) {
        clearForm();
        if (currentItem()) {
            mainListWidget()->setCurrentItem(currentItem());
            fillForm();
        }
        setModifiedHook(false);
    }
}

template <typename T, typename Traits>
inline QListWidgetItem *AbstractManager<T, Traits>::currentItem()
{ return mainListWidget()->currentItem(); }

template <typename T, typename Traits>
inline typename AbstractManager<T, Traits>::type_ptr AbstractManager<T, Traits>::currentElement()
{
    if (!currentItem())
        return type_ptr();

    return static_cast<QVariant>(currentItem()->data(DataRole)).value<type_ptr>();
}
