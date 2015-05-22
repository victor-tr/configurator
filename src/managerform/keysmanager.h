#ifndef KEYSMANAGER_H
#define KEYSMANAGER_H

#include <QWidget>
#include "bo/t_key.h"
#include "bo/t_arminggroup.h"


namespace Ui {

class KeysManager;

} // namespace Ui

class QListWidgetItem;


class KeysManager : public QWidget
{
    Q_OBJECT

public:

    enum {
        IdRole      = Qt::UserRole,
        DataRole
    };

    explicit KeysManager(QWidget *parent = 0);
    ~KeysManager();

    static bool isModified() { return _bModified; }
    static int  getCurrentETRUIN() { return _current_ETR_UIN; }
    static bool keyExists(const t_Key_ptr &key);

    static QString fromHexDataToText(const QByteArray &a);
    static QByteArray toHexDataFromText(const QString &text);

private:

    void setModified(bool modified);
    void fillForm();
    void clearForm();
    void saveForm(t_Key_ptr &pKey);

    t_ArmingGroup_ptr getGroupFromCombobox();
    void setGroupAtCombobox(t_Key_ptr pKey);
    void updateAvailableGroupsCombobox();

    QListWidgetItem *currentItem();
    t_Key_ptr currentKey();

    void actualizeGroupsCombobox();

    static bool _bModified;
    static int  _current_ETR_UIN;

    bool             _bAdding;
    Ui::KeysManager *_ui;

    t_KeyX         _keys_list;

Q_SIGNALS:

    void snlListChanged();
    void snlRequestKeyFromDevice();

public Q_SLOTS:

    void sltUpdateList();
    void sltUpdateAvailableETRsCombobox();
    void sltActivateSpecialControls(bool activate); // they require direct access to the configured device

private Q_SLOTS:

    void sltUpdateUI();
    void sltAddApply();
    void sltDeleteCancel();
    void sltRequestKeyFromDevice();

};

#endif // KEYSMANAGER_H
