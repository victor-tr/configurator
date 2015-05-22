#include "precompiled.h"
#include "progressdialog.h"
#include <QWindow>
#include <QKeyEvent>


ProgressDialog::ProgressDialog(QWidget *parent)
    : QProgressDialog(parent)
{
    setMinimumDuration(0);
    setWindowModality(Qt::ApplicationModal);

    Qt::WindowFlags wflags = windowFlags();
    wflags |= Qt::FramelessWindowHint;
    setWindowFlags(wflags);

    qApp->installEventFilter(this);  // catch Esc key press
}

bool ProgressDialog::eventFilter(QObject *obj, QEvent *e)
{
    // prevent closing the progress dialog by Esc key press
    if ((obj == windowHandle())
            && (e->type() == QEvent::KeyPress))
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Escape)
            return true;
    }

    return QProgressDialog::eventFilter(obj, e);
}

void ProgressDialog::stInitProgressDialog(int max, const QString &label)
{
    setRange(0, max);
    setLabelText(label);
    setValue(0);
}

void ProgressDialog::stUpdateProgressDialog(int progress)
{
    if (!wasCanceled()) {
        setValue(progress);
    }
}
