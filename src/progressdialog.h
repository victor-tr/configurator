#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>


class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:

    ProgressDialog(QWidget *parent = 0);

protected:

    bool eventFilter(QObject *obj, QEvent *e);

public Q_SLOTS:

    void stInitProgressDialog(int max, const QString &label);
    void stUpdateProgressDialog(int progress);

};

#endif // PROGRESSDIALOG_H
