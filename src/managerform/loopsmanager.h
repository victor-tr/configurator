#ifndef LOOPSMANAGER_H
#define LOOPSMANAGER_H

#include "abstractmanager.h"
#include "bo/t_zone.h"


namespace Ui {
    class ProtectionLoopsManager;
}


class LoopsManager : public ManagerWidget, public AbstractManager<t_Zone>
{
    Q_OBJECT

public:

    explicit LoopsManager(QWidget *parent = 0);
    ~LoopsManager();

private:

    void setModifiedHook(bool modified);

    void fillFormHook(type_ptr_const_ref pElement);
    void clearForm();
    void saveForm(type_ptr_ref pElement);

    QListWidget *mainListWidget();
    QObject *sender() const;

    t_ArmingGroup_ptr getGroupFromCombobox();
    void setGroupAtCombobox(type_ptr_const_ref pElement);
    void updateAvailableGroups();

    Ui::ProtectionLoopsManager *_ui;

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI() { updateUI(); }
    void sltApply()    { if (apply()) Q_EMIT snlListChanged(); }
    void sltCancel()   { cancel(); }

};


#endif // LOOPSMANAGER_H
