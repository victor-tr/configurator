#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include <QWidget>
#include "bo/t_event.h"


namespace Ui {

class EventsManager;

} // namespace Ui

class QListWidgetItem;


class EventsManager : public QWidget
{
    Q_OBJECT

public:

    enum {
        IdRole      = Qt::UserRole,
        DataRole
    };

    explicit EventsManager(QWidget *parent = 0);
    ~EventsManager();

    static bool isModified()    { return _bModified; }

private:

    void setModified(bool modified);

    void fillForm();
    void clearForm();
    void saveForm(t_Event_ptr &pEvent);

    bool canBeDeleted(const t_Event_ptr &pEvent);

    QListWidgetItem *currentItem();
    t_Event_ptr currentEvent();

    void updateEmitterTypeCombobox();
    void setEmitterTypeAtCombobox(int emitterType);
    int  getEmitterTypeFromCombobox();

    void updateEmitterAliasCombobox(int emitterType);
    void setEmitterAliasAtCombobox(const t_Event_ptr &pEvent);
    int  getEmitterIdFromAliasCombobox();

    void updateEventCombobox(int emitterType);
    void setEventAtCombobox(int eventCode);
    int  getEventFromCombox();

    void updateAvailableReactionsList();
    void updateLinkedReactionsList(const t_Event_ptr &pEvent);

    static bool _bModified;

    bool _bAdding;

    Ui::EventsManager *_ui;

    t_EventX    _event_list;

    QStringList _event_relations;

signals:

    void snlListChanged();

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI();
    void sltUpdateDependentComboboxes(int emitterTypeIndex);

    void sltAddApply();
    void sltDeleteCancel();

    void sltLinkReaction();
    void sltUnlinkReaction();

};

#endif // EVENTSMANAGER_H
