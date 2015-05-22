#ifndef LEDSMANAGER_H
#define LEDSMANAGER_H

#include "abstractmanager.h"
#include "bo/t_led.h"
#include "bo/t_arminggroup.h"


namespace Ui {

class LedsManager;

} // namespace Ui


class LedsManager : public ManagerWidget, public AbstractManager<t_Led>
{
    Q_OBJECT

public:

    explicit LedsManager(QWidget *parent = 0);
    ~LedsManager();

private:

    void setModifiedHook(bool modified);

    void fillFormHook(type_ptr_const_ref pElement);
    void clearForm();
    void saveForm(type_ptr_ref pElement);

    QListWidget *mainListWidget();
    QObject *sender() const;

    void updateGroupsCombobox();
    void setGroupAtCombobox(type_ptr_const_ref pElement);
    t_ArmingGroup_ptr getGroupFromCombobox();
    void actualizeGroupsCombobox();

    Ui::LedsManager *_ui;

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI();
    void sltApply()    { if (apply()) Q_EMIT snlListChanged(); }
    void sltCancel()   { cancel(); }

};

#endif // LEDSMANAGER_H
