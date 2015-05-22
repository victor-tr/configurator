#ifndef KEYREQUESTDIALOG_H
#define KEYREQUESTDIALOG_H

#include <QDialog>


namespace Ui {

class KeyRequestDialogForm;

} // namespace Ui


class KeyRequestDialog : public QDialog
{
    Q_OBJECT

public:

    explicit KeyRequestDialog(QWidget *parent = 0);
    ~KeyRequestDialog();

private:

    void actualizeSaveButtonState();
    void saveKeyToDB(const QByteArray &trimmedKey);

    Ui::KeyRequestDialogForm *_ui;
    QByteArray _trimmedValue;

signals:

    void snlListChanged();
    void snlRequestCancel();
    
public slots:

    void sltSetKey(const QByteArray &key);

private slots:

    void sltSaveKey();
    void sltCancelRequest();
    
};

#endif // KEYREQUESTDIALOG_H
