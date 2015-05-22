#ifndef EXPANDERSMANAGER_H
#define EXPANDERSMANAGER_H

#include <QWidget>
#include "bo/t_expander.h"


namespace Ui {
    class ExpandersManager;
}

class QListWidgetItem;

class ExpandersManager : public QWidget
{

    Q_OBJECT

public:

    explicit ExpandersManager(QWidget *parent = 0);
    ~ExpandersManager();

    static bool isModified();

private:

    void fillForm();
    void clearForm();
    void saveForm(t_Expander_ptr &expander);
    void setModified(bool modified);
    bool canBeDeleted(int expanderId);
    bool isValidForm();

    QListWidgetItem *currentItem();
    t_Expander_ptr currentExpander();

    static bool           _bModified;

    Ui::ExpandersManager *_ui;

    t_ExpanderX  _expanders_list;
    QStringList  _relation;

    bool             _bAdding;

Q_SIGNALS:

    void snlListChanged();

public Q_SLOTS:

    void sltUpdateList();
    void sltUpdateUI();

private Q_SLOTS:

    void sltAddApply();
    void sltDeleteCancel();
};

#endif // EXPANDERSMANAGER_H
