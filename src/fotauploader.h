#ifndef FOTAUPLOADER_H
#define FOTAUPLOADER_H

#include <QDialog>

namespace Ui {
class FotaUploader;
}

class AbstractCommunicator;
class QAbstractTransition;
class QState;

class FotaUploader : public QDialog
{
    Q_OBJECT
public:
    explicit FotaUploader(AbstractCommunicator *pCommunicator, QState *pSrcState,
                          QState *pTargetState, QWidget *parent = 0);
    ~FotaUploader();

Q_SIGNALS:
    void snFotaStart(const QString &filepath);

private:
    void stSetFilePath();
    void stStartFotaUploading();
    void updateUI();

private:
    Ui::FotaUploader     *_ui;
    AbstractCommunicator *_pCommunicator;
    QState               *_pSrcState;
    QAbstractTransition  *_pTransition;
};

#endif // FOTAUPLOADER_H
