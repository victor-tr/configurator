#include "keyrequestdialog.h"
#include "ui_keyrequestdialog.h"

#include "bo/t_key.h"

#include "managerform/keysmanager.h"
#include "configurator_protocol.h"

#include <QPushButton>
#include <QMessageBox>


KeyRequestDialog::KeyRequestDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::KeyRequestDialogForm())
{
    _ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    actualizeSaveButtonState();

    setWindowTitle(tr("Request KEY"));
    _ui->lbl_key_value->setText(tr("Waiting..."));

    connect(_ui->buttonBox->button(QDialogButtonBox::Save), &QAbstractButton::clicked,
            this, &KeyRequestDialog::sltSaveKey);
    connect(this, &QDialog::rejected,
            this, &KeyRequestDialog::sltCancelRequest);
}

KeyRequestDialog::~KeyRequestDialog()
{
    delete _ui;
}

inline void KeyRequestDialog::actualizeSaveButtonState()
{
    QPushButton *btn = _ui->buttonBox->button(QDialogButtonBox::Save);
    btn->setEnabled(!_trimmedValue.isEmpty());
    btn->setFocus();
}

void KeyRequestDialog::saveKeyToDB(const QByteArray &trimmedKey)
{
    t_Key_ptr pKey(new t_Key());

    pKey->_type = OBJ_TOUCHMEMORY_CODE;
    pKey->_value = trimmedKey;
    pKey->_action = COMMON_KEY_EVENT;

    if (KeysManager::keyExists(pKey)) {
        QMessageBox::information(this,
                                 tr("Add key"),
                                 tr("Key \"<b>%1</b>\" already registered")
                                    .arg(KeysManager::fromHexDataToText(pKey->_value))
                                 );
        return;
    }

    qx::dao::insert(pKey);
    Q_EMIT snlListChanged();
}

void KeyRequestDialog::sltSetKey(const QByteArray &key)
{
    if (key.isEmpty()) {
        _ui->lbl_key_value->setText(tr("FAILED"));
        return;
    }

    QByteArray reversed;
    reversed.resize(key.size() - 2);
    std::reverse_copy(key.constBegin() + 1,
                      key.constEnd() - 1,
                      reversed.begin());

    _trimmedValue = reversed;

    _ui->lbl_key_value->setText(KeysManager::fromHexDataToText(reversed));
    actualizeSaveButtonState();
}

void KeyRequestDialog::sltSaveKey()
{
    saveKeyToDB(_trimmedValue);
}

void KeyRequestDialog::sltCancelRequest()
{
    if (_trimmedValue.isEmpty())
        Q_EMIT snlRequestCancel();
}
