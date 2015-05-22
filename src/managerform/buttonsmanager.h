#ifndef BUTTONSMANAGER_H
#define BUTTONSMANAGER_H

#include "abstractmanager.h"
#include "bo/t_button.h"


namespace Ui {

class ButtonsManager;

} // namespace Ui


class ButtonsManager : public ManagerWidget, public AbstractManager<t_Button>
{
    Q_OBJECT

public:

    explicit ButtonsManager(QWidget *parent = 0);
    ~ButtonsManager();

private:

    void setModifiedHook(bool modified);

    void fillFormHook(type_ptr_const_ref pElement);
    void clearForm();
    void saveForm(type_ptr_ref pElement);

    QListWidget *mainListWidget();
    QObject *sender() const;

    Ui::ButtonsManager *_ui;

public slots:

    void sltUpdateList() { updateList(this); }

private slots:

    void sltUpdateUI() { updateUI(); }
    void sltApply()    { if (apply()) Q_EMIT snlListChanged(); }
    void sltCancel()   { cancel(); }

};


#endif // BUTTONSMANAGER_H
