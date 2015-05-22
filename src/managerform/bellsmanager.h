#ifndef BELLSMANAGER_H
#define BELLSMANAGER_H

#include "abstractmanager.h"
#include "bo/t_bell.h"


namespace Ui {

class BellsManager;

} // namespace Ui


class BellsManager : public ManagerWidget, public AbstractManager<t_Bell>
{
    Q_OBJECT

public:

    explicit BellsManager(QWidget *parent = 0);
    ~BellsManager();

private:

    void setModifiedHook(bool modified);

    void fillFormHook(type_ptr_const_ref pElement);
    void clearForm();
    void saveForm(type_ptr_ref pElement);

    QListWidget *mainListWidget();
    QObject *sender() const;

    Ui::BellsManager *_ui;

public slots:

    void sltUpdateList() { updateList(this); }

private slots:

    void sltUpdateUI() { updateUI(); }
    void sltApply()    { if (apply()) Q_EMIT snlListChanged(); }
    void sltCancel()   { cancel(); }

};


#endif // BELLSMANAGER_H
