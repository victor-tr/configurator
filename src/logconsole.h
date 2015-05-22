#ifndef LOGCONSOLE_H
#define LOGCONSOLE_H

#include <QDialog>


namespace Ui {

class LogConsole;

} // namespase Ui


class LogConsole : public QDialog
{
    Q_OBJECT

public:

    enum ConsoleContent {
        CC_Html,
        CC_PlainText
    };

    explicit LogConsole(QWidget *parent = 0);
    ~LogConsole();

    void appendTextLine(const QString &text,
                        ConsoleContent type = CC_Html,
                        const QString &textColor = QString("black"));

    void appendTextLine(const QString &prependText,
                        const QString &msgText,
                        const QColor &prependTextColor = QColor("black"),
                        const QColor &msgTextColor = QColor("black"));

    void appendDataLine(const QByteArray &data,
                        const QString &prependText = QString(),
                        const QColor &textColor = QColor("black"));

    void appendSeparatorLine();

public Q_SLOTS:

    bool isLogAsHex() const { return _logAsHex; }
    void setLogAsHex(bool asHex);

    bool isUseTimeMark() const { return _useTimeMark; }
    void setUseTimeMark(bool use);

    QByteArray windowGeometry()                        { return _geometry; }
    void setWindowGeometry(const QByteArray &geometry) { _geometry = geometry; }

    void show();
    void showIfNeeded();

protected:

    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    //bool event(QEvent *e);

private:

    QString splitByWhitespaces(const QByteArray &hexData);
    void setLogAsHex_helper(bool asHex) { _logAsHex = asHex; }
    void setUseTimeMark_helper(bool use) { _useTimeMark = use; }

    Ui::LogConsole *ui;

    bool _logAsHex;
    bool _useTimeMark;
    bool _visible;

    QByteArray _geometry;

};

#endif // LOGCONSOLE_H
