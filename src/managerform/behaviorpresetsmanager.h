#ifndef BEHAVIORPRESETSMANAGER_H
#define BEHAVIORPRESETSMANAGER_H

#include <QWidget>
#include "bo/t_behaviorpreset.h"


namespace Ui {

class BehaviorPresetsManager;

} // namespace Ui

class QListWidgetItem;


class BehaviorPresetsManager : public QWidget
{
    Q_OBJECT

public:

    enum {
        IdRole      = Qt::UserRole,
        DataRole
    };

    explicit BehaviorPresetsManager(QWidget *parent = 0);
    ~BehaviorPresetsManager();

    static bool isModified()    { return _bModified; }

private:

    void setModified(bool modified);

    void fillForm();
    void clearForm();
    void saveForm(t_BehaviorPreset_ptr &pPreset);

    bool canBeDeleted(const t_BehaviorPreset_ptr &pPreset);

    QListWidgetItem *currentItem();
    t_BehaviorPreset_ptr currentBehaviorPreset();

    void actualizeUI(int behaviorType);
    void blockSpinboxesSignals(bool block);

    static bool _bModified;

    bool _bAdding;

    Ui::BehaviorPresetsManager *_ui;

    t_BehaviorPresetX _behavior_preset_list;

signals:

    void snlListChanged();

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI();
    void sltAddApply();
    void sltDeleteCancel();

};

#endif // BEHAVIORPRESETSMANAGER_H
