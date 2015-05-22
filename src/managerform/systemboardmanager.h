#ifndef SYSTEMBOARDMANAGER_H
#define SYSTEMBOARDMANAGER_H

#include <QWidget>
#include "bo/t_systemboard.h"


namespace Ui {

class SystemBoardManager;

} // namespace Ui


class SystemBoardManager : public QWidget
{
    Q_OBJECT

public:

    explicit SystemBoardManager(QWidget *parent = 0);
    ~SystemBoardManager();

    static bool isModified();

private:

    void setModified(bool modified);
    void fillForm();
    void saveForm();

    static bool _bModified;

    Ui::SystemBoardManager *_ui;

    t_SystemBoard_ptr _system_board;
    QStringList _relation;

Q_SIGNALS:

    void snlDataChanged();

public Q_SLOTS:

    void sltUpdateData();

private Q_SLOTS:

    void sltUpdateUI();
    void sltApply();
    void sltCancel();

};

#endif // SYSTEMBOARDMANAGER_H
