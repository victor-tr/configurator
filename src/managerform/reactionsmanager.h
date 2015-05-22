#ifndef REACTIONSMANAGER_H
#define REACTIONSMANAGER_H

#include <QWidget>
#include "bo/t_reaction.h"
#include "bo/t_event.h"
#include "bo/t_behaviorpreset.h"


namespace Ui {

class ReactionsManager;

} // namespace Ui

class QListWidgetItem;


class ReactionsManager : public QWidget
{
    Q_OBJECT

public:

    enum {
        IdRole      = Qt::UserRole,
        DataRole
    };

    explicit ReactionsManager(QWidget *parent = 0);
    ~ReactionsManager();

    static bool isModified()    { return _bModified; }

private:

    void setModified(bool modified);

    void fillForm();
    void clearForm();
    void saveForm(t_Reaction_ptr &pReaction);

    bool canBeDeleted(const t_Reaction_ptr &pReaction);

    QListWidgetItem *currentItem();
    t_Reaction_ptr currentReaction();

    void updatePerformerTypeCombobox();
    void setPerformerTypeAtCombobox(int performerType);
    int  getPerformerTypeFromCombobox();

    void updatePerformerAliasCombobox(int performerType);
    void setPerformerAliasAtCombobox(const t_Reaction_ptr &pReaction);
    int  getPerformerIdFromAliasCombobox();

    void updateBehaviorPresetCombobox();
    void setBehaviorPresetAtCombobox(const t_Reaction_ptr &pReaction);
    t_BehaviorPreset_ptr getBehaviorPresetFromCombobox();
    void setReverseBehaviorPresetAtCombobox(const t_Reaction_ptr &pReaction);
    t_BehaviorPreset_ptr getReverseBehaviorPresetFromCombobox();

    void updateEventCombobox();
    void setEventAtCombobox(const t_Reaction_ptr &pReaction);
    t_Event_ptr  getEventFromCombobox();

    void clearReversibleRelated();
    void fillReversibleRelated();
    void updateUiReversibleRelated();

    void actualizeAvailableStates();
    void fillStatesCheckboxes();
    void clearStatesCheckboxes();


    static bool _bModified;

    bool _bAdding;

    Ui::ReactionsManager *_ui;

    t_ReactionX    _reaction_list;

    QStringList _reaction_relations;

signals:

    void snlListChanged();

public slots:

    void sltUpdateList();

private slots:

    void sltUpdateUI();
    void sltUpdateDependentComboboxes(int performerTypeIndex);
    void sltUpdateDependentPerformers();

    void sltAddApply();
    void sltDeleteCancel();

};

#endif // REACTIONSMANAGER_H
