#ifndef COMMONSETTINGSMANAGER_H
#define COMMONSETTINGSMANAGER_H

#include <QWidget>
#include "bo/t_commonsettings.h"


namespace Ui {
    class CommonSettingsManager;
}

class CommonSettingsManager : public QWidget
{
    Q_OBJECT

public:

    explicit CommonSettingsManager(QWidget *parent = 0);
    ~CommonSettingsManager();

    static bool isModified();

private:

    void fillForm();
    void saveForm();
    void setModified(bool modified);
    void db_update();

    Ui::CommonSettingsManager *_ui;
    t_CommonSettings_ptr       _common_settings;

    static bool                _bModified;

Q_SIGNALS:

    void snlDataChanged();

public Q_SLOTS:

    void sltUpdateData();

private Q_SLOTS:

    void sltUpdateUI();
    void sltApply();
    void sltCancel();
};

#endif // COMMONSETTINGSMANAGER_H
