#ifndef ETRMANAGER_H
#define ETRMANAGER_H

#include <QWidget>
#include "bo/t_etr.h"


namespace Ui {

class EtrManager;

} // namespace Ui

class QListWidgetItem;


class EtrManager : public QWidget
{
    Q_OBJECT

public:

    explicit EtrManager(QWidget *parent = 0);
    ~EtrManager();

    static bool isModified();

private:

    void setModified(bool modified);

    void fillForm();
    void clearForm();
    void saveForm(t_Etr_ptr &pEtr);
    bool canBeDeleted(const t_Etr_ptr &pEtr);
    bool isValidForm();

    QListWidgetItem *currentItem();
    t_Etr_ptr currentEtr();

    static bool _bModified;

    bool            _bAdding;
    Ui::EtrManager *_ui;

    t_EtrX  _etr_list;
    // TODO: add related arming groups read-only list

    QStringList _relation;

Q_SIGNALS:

    void snlListChanged();

public Q_SLOTS:

    void sltUpdateList();

private Q_SLOTS:

    void sltUpdateUI();
    void sltAddApply();
    void sltDeleteCancel();

};

#endif // ETRMANAGER_H
