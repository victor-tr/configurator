#ifndef ARMINGGROUPSMANAGER_H
#define ARMINGGROUPSMANAGER_H

#include <QWidget>

#include "bo/t_arminggroup.h"
#include "bo/t_etr.h"
#include "bo/t_key.h"


namespace Ui {
    class ArmingGroupsManager;
}

class QListWidgetItem;


class ArmingGroupsManager : public QWidget
{
    Q_OBJECT

public:

    enum {
        IdRole      = Qt::UserRole,
        DataRole
    };

    explicit ArmingGroupsManager(QWidget *parent = 0);
    ~ArmingGroupsManager();

    static bool isModified()    { return _bModified; }

private:

    void setModified(bool modified);

    void fillForm();
    void clearForm();
    void saveForm(t_ArmingGroup_ptr &pGroup);

    bool canBeDeleted(const t_ArmingGroup_ptr &pGroup);

    QListWidgetItem *currentItem();
    t_ArmingGroup_ptr currentGroup();

    void updateRelatedKeyboardsList(t_ArmingGroup_ptr pGroup);
    void updateRelatedETRsList(t_ArmingGroup_ptr pGroup);
    void updateRelatedKeysList(t_ArmingGroup_ptr pGroup);

    void updateAvailableKeyboardsList();
    void updateAvailableETRsList(t_ArmingGroup_ptr pGroup);
    void updateFreeKeysList();
    // TODO: add related zones read-only list

    static bool _bModified;

    bool _bAdding;

    Ui::ArmingGroupsManager *_ui;

    t_ArmingGroupX     _arming_group_list;

    QStringList _group_relations;

Q_SIGNALS:

    void snlListChanged();

public Q_SLOTS:

    void sltUpdateList();

private Q_SLOTS:

    void sltUpdateUI();
    void sltAddApply();
    void sltDeleteCancel();

    void sltUpdateArmingDevicesLists(int index);

    void sltMoveArmingDeviceToRelated();
    void sltMoveArmingDeviceToAvailable();

    void sltMoveKeyToRelated();
    void sltMoveKeyToFree();

};

#endif // ARMINGGROUPSMANAGER_H
