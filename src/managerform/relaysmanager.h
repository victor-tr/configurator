#ifndef RELAYSMANAGER_H
#define RELAYSMANAGER_H

#include "abstractmanager.h"
#include "bo/t_relay.h"
#include "bo/t_arminggroup.h"


namespace Ui {

class RelaysManager;

} // namespace Ui


class RelaysManager : public ManagerWidget, public AbstractManager<t_Relay>
{
    Q_OBJECT

public:

    explicit RelaysManager(QWidget *parent = 0);
    ~RelaysManager();

private:

    void setModifiedHook(bool modified);

    void fillFormHook(type_ptr_const_ref pElement);
    void clearForm();
    void saveForm(type_ptr_ref pElement);

    QListWidget *mainListWidget();
    QObject *sender() const;

    void setLoadTypeAtCombobox(type_ptr_const_ref pElement);
    int  getLoadTypeFromCombobox();

    void updateGroupsCombobox();
    void setGroupAtCombobox(type_ptr_const_ref pElement);
    t_ArmingGroup_ptr getGroupFromCombobox();
    void actualizeGroupsCombobox(const QString &loadTypeText);

    Ui::RelaysManager *_ui;

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI();
    void sltApply()    { if (apply()) Q_EMIT snlListChanged(); }
    void sltCancel()   { cancel(); }

};


#endif // RELAYSMANAGER_H
