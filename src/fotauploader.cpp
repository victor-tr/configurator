#include "fotauploader.h"
#include "ui_fotauploaderform.h"
#include "abstractcommunicator.h"
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QState>
#include <QSignalTransition>


FotaUploader::FotaUploader(AbstractCommunicator *pCommunicator, QState *pSrcState,
                           QState *pTargetState, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FotaUploader),
    _pCommunicator(pCommunicator),
    _pSrcState(pSrcState)
{
    _ui->setupUi(this);

    _pTransition = pSrcState->addTransition(this, SIGNAL(snFotaStart(QString)), pTargetState);

    connect(_ui->btn_selectFilepath, &QToolButton::clicked, this, &FotaUploader::stSetFilePath);
    connect(_ui->le_filepath, &QLineEdit::textChanged, this, &FotaUploader::updateUI);
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &FotaUploader::stStartFotaUploading);

    updateUI();
}

FotaUploader::~FotaUploader()
{
    _pSrcState->removeTransition(_pTransition);
    delete _ui;
}

void FotaUploader::stSetFilePath()
{
    QString filepath = QFileDialog::getOpenFileName(this,
                                                    tr("Choose firmware file"),
                                                    QApplication::applicationDirPath(),
                                                    tr("Firmware (*.bin)"));
    _ui->le_filepath->setText(filepath);
}

void FotaUploader::stStartFotaUploading()
{
    QFile fotaFile(_ui->le_filepath->text());
    if (!fotaFile.open(QFile::ReadOnly)) {
        qDebug("Can't open FOTA file");
        return;
    }

    QSharedPointer<QByteArray> fota(new QByteArray);
    fota->append(static_cast<char>(OBJ_FOTA_FILE));
    fota->append(static_cast<char>(0));    // WARNING: magic numbers !!!
    fota->append(static_cast<char>(1));
    fota->append(fotaFile.readAll());
    if (fota->size() <= 3)
        return;

    _pCommunicator->clearTxBuffer();
    _pCommunicator->appendToTxBuffer(fota);

    Q_EMIT snFotaStart(_ui->le_filepath->text());
    _pCommunicator->beginUploading();
}

void FotaUploader::updateUI()
{
    QString filepath = _ui->le_filepath->text();
    bool bValid = !filepath.isEmpty() &&
                      QFile::exists(filepath) &&
                      filepath.endsWith(".bin", Qt::CaseInsensitive);

    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);

    QPalette palette = _ui->le_filepath->palette();
    palette.setColor(QPalette::Text, bValid ? Qt::black : Qt::red);
    _ui->le_filepath->setPalette(palette);
}
