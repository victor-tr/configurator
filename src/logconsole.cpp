#include "logconsole.h"
#include "ui_logconsole.h"

#include <QTime>


LogConsole::LogConsole(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogConsole())
    , _logAsHex(false)
    , _useTimeMark(true)
    , _visible(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("Log"));

    connect(ui->btn_clear, &QPushButton::clicked, ui->console, &QPlainTextEdit::clear);
    connect(ui->chbx_ashex, &QCheckBox::toggled, this, &LogConsole::setLogAsHex_helper);

    ui->chbx_ashex->setChecked(false);

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    installEventFilter(this);
}

LogConsole::~LogConsole()
{
    delete ui;
}

void LogConsole::appendTextLine(const QString &text,
                                LogConsole::ConsoleContent type,
                                const QString &textColor)
{
    if (text.isEmpty())
        return;

    QTime time = QTime::currentTime();
    QString formatted_time = _useTimeMark
                                ? QString("[%1] ").arg(time.isValid() ? time.toString("hh:mm:ss") : tr("Invalid Time"))
                                : "";

    switch (type) {
    case CC_Html:
        ui->console->appendHtml(QString("%1%2")
                                .arg(formatted_time)
                                .arg(QString("<span style=\"color:%1\">%2</span> ").arg(textColor, text))
                                );
        break;
    case CC_PlainText:
        ui->console->appendPlainText(QString("%1%2").arg(formatted_time).arg(text));
        break;
    }
}

void LogConsole::appendTextLine(const QString &prependText,
                                const QString &msgText,
                                const QColor &prependTextColor,
                                const QColor &msgTextColor)
{
    if (prependText.isEmpty() && msgText.isEmpty())
        return;

    QTime time = QTime::currentTime();
    QString formatted_time = _useTimeMark
                                ? QString("[%1] ").arg(time.isValid() ? time.toString("hh:mm:ss") : tr("Invalid Time"))
                                : "";

    ui->console->appendHtml(QString("%1%2 %3")
                            .arg(formatted_time)
                            .arg(!prependText.isEmpty() ? QString("<span style=\"color:%1\">%2</span> ").arg(prependTextColor.name(), prependText) : "")
                            .arg(!msgText.isEmpty() ? QString("<span style=\"color:%1\">%2</span> ").arg(msgTextColor.name(), msgText) : "")
                            );

}

void LogConsole::appendDataLine(const QByteArray &data, const QString &prependText, const QColor &textColor)
{
    QTime time = QTime::currentTime();

    ui->console->appendHtml( QString("%1%2 %3")
                             .arg(_useTimeMark
                                  ? QString("[%1] ").arg(time.isValid() ? time.toString("hh:mm:ss") : tr("Invalid Time"))
                                  : "")
                             .arg(!prependText.isEmpty()
                                  ? QString("<span style=\"color:%1\">%2</span> ").arg(textColor.name(), prependText)
                                  : "")
                             .arg(_logAsHex
                                  ? splitByWhitespaces(data.toHex().toUpper())
                                  : data.data())
                            );
}

void LogConsole::appendSeparatorLine()
{
    ui->console->appendPlainText(" ");
}

void LogConsole::setLogAsHex(bool asHex)
{
    ui->chbx_ashex->setChecked(asHex);
}

void LogConsole::setUseTimeMark(bool use)
{
    setUseTimeMark_helper(use);
}

void LogConsole::show()
{
    _visible = true;
    restoreGeometry(_geometry);
    QDialog::showNormal();
}

void LogConsole::showIfNeeded()
{
    if (_visible)
        show();
}

void LogConsole::closeEvent(QCloseEvent *e)
{
    _visible = false;
    _geometry = saveGeometry();
    QDialog::closeEvent(e);
}

bool LogConsole::eventFilter(QObject *obj, QEvent *e)
{
//    qDebug() << QString("LogConsole::eventFilter(%1)").arg(e->type());
    if (/*obj == this && */e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Escape)
            return true;
    } else if (e->type() == QEvent::Hide) {
        _geometry = saveGeometry();
    }

    return QDialog::eventFilter(obj, e);
}

inline QString LogConsole::splitByWhitespaces(const QByteArray &hexData)
{
    QString ret;
    int size = hexData.size();
    for (int i = 0; i < size; i += 2) {
        ret.append(hexData.at(i));
        ret.append(hexData.at(i + 1));
        ret.append(' ');
    }

    return ret;
}

//bool LogConsole::event(QEvent *e)
//{
//    qDebug() << QString("LogConsole::event(%1)").arg(e->type());
//    return QDialog::event(e);
//}

