#ifndef XMLSETTINGSPROVIDER_H
#define XMLSETTINGSPROVIDER_H

#include <exception>
#include <QString>


class XMLImportError : public std::exception
{
public:
    explicit XMLImportError(const char *message) : m_msg(message)    {}
    explicit XMLImportError(const QString &message) : m_msg(message) {}

    const char * what() const throw()
    {
        return m_msg.toStdString().c_str();
    }

private:
    QString m_msg;
};


class XmlSettingsProvider
{

public:

    explicit XmlSettingsProvider(const QString &dbname, QWidget *parent = 0);
    ~XmlSettingsProvider();

    bool importSettings(const QString &filename, QByteArray *pOut);
    bool exportSettings(const QString &filename);

private:

    QString toString(const QVariant &value);
    QVariant fromString(int dataType, const QString &value);

    QString  _dbname;
    QWidget *_parent_widget;

};

#endif // XMLSETTINGSPROVIDER_H
