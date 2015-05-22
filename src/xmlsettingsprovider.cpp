#include "precompiled.h"

#include "xmlsettingsprovider.h"
#include "configurator_protocol.h"
#include "mainwindow.h"

#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDate>
#include <QTime>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QProgressDialog>
#include <QCoreApplication>


XmlSettingsProvider::XmlSettingsProvider(const QString &dbname, QWidget *parent) :
    _dbname(dbname),
    _parent_widget(parent)
{}

XmlSettingsProvider::~XmlSettingsProvider()
{
    QString connName = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(connName);

    QString tempfile = _dbname + "~";
    if (QFile::exists(tempfile) && !QFile::remove(tempfile))
        qDebug() << "Can't remove temporary database file";
}

bool XmlSettingsProvider::importSettings(const QString &filename, QByteArray *pOut)
{
    // -- create temporary database
    QString tempDbName = _dbname + "~";

    if (QFile::exists(tempDbName))
        QFile::remove(tempDbName);


    QFile skeleton(ETALON_DB_SKELETON_FILE_PATH);
    QFile tempDbFile(tempDbName);

    bool success = skeleton.open(QIODevice::ReadOnly) &&
                    tempDbFile.open(QIODevice::ReadWrite|QIODevice::Truncate);
    if (!success)
        throw XMLImportError(QObject::tr("Open DB file error"));


    tempDbFile.write(skeleton.readAll());

    if (!tempDbFile.seek(0))
        throw XMLImportError(QObject::tr("Read DB content error"));

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(tempDbName);
        if (!db.open())
            throw XMLImportError(QObject::tr("Open DB connection error"));

        qx::QxSession session(db);
        Q_UNUSED(session);

        // -- process XML settings and fill the temporary database
        QFile xmlSettings(filename);
        if (!xmlSettings.open(QIODevice::ReadOnly))
            throw XMLImportError(QObject::tr("Open XML file error"));

        QXmlStreamReader r(&xmlSettings);

        QString tablename;
        QSqlRecord rec;
        int recId = 0;

        QSqlTableModel table;
        table.setEditStrategy(QSqlTableModel::OnManualSubmit);

        QString fieldname;
        int fieldtype = 0;
        QVariant fieldvalue;

        // -- setup progress dialog
        int tableNo = 0;
        QProgressDialog progress(_parent_widget);
        progress.setRange(0, db.tables().size());
        progress.setModal(true);
        progress.setFixedWidth(250);
        progress.setWindowTitle(QObject::tr("Import XML"));

        while (!r.atEnd()) {
            switch (r.readNext()) {
            case QXmlStreamReader::StartElement:
                if (0 == r.name().compare("table")) {
                    tablename = r.attributes().value("name").toString();
                    table.clear();
                    table.setTable(tablename);
                }
                else if (0 == r.name().compare("tr")) {
                    bool converted = false;
                    recId = r.attributes().value("id").toInt(&converted);
                    rec = db.record(tablename);
                }
                else if (0 == r.name().compare("td")) {
                    fieldname = r.attributes().value("name").toString();
                    fieldtype = r.attributes().value("type").toInt();
                }
                else if (0 == r.name().compare("PPK_Settings")) {
                    QString db_version_str = r.attributes().value("version").toString();
                    bool version_ok = 0 == db_version_str.compare(DB_STRUCTURE_VERSION, Qt::CaseInsensitive);
                    if (!version_ok)
                    {
                        throw XMLImportError(QObject::tr("Bad settings version\n"
                                             "Aborted"));
                    }
                }
                else {
                    r.skipCurrentElement();
                }
                break;

            case QXmlStreamReader::Characters:
                if (!fieldname.isEmpty() && fieldtype) {
                    fieldvalue = fromString(fieldtype, r.text().toString());
                    rec.setValue(fieldname, fieldvalue);
                }
                break;

            case QXmlStreamReader::EndElement:
                if (0 == r.name().compare("table")) {
                    QCoreApplication::processEvents();
                    if (progress.wasCanceled())
                        return false;
                    progress.setLabelText(QObject::tr("Importing %1 table...").arg(tablename));
                    table.submitAll();
                    progress.setValue(++tableNo);
                }
                else if (0 == r.name().compare("tr")) {
                    rec.setValue("id", recId);
                    table.insertRecord(-1, rec);
                    rec.clear();
                }
                else if (0 == r.name().compare("td")) {
                    fieldname.clear();
                    fieldvalue.clear();
                }
                break;

            default:
                break;
            }
        }
        if (r.hasError()) {
            throw XMLImportError(QObject::tr("Parsing XML error"));
        }
    }

    QString connName = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(connName);

    // -- return serialized result
    pOut->append(tempDbFile.readAll());
    tempDbFile.close();

    return true;
}

bool XmlSettingsProvider::exportSettings(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter w(&file);
    w.setAutoFormatting(true);
    w.writeStartDocument();

    w.writeComment(QString("Exported on %1 at %2")
                   .arg(QDate::currentDate().toString("dd/MM/yyyy"))
                   .arg(QTime::currentTime().toString()));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(_dbname);
    if (!db.open()) {
        w.writeEndDocument();
        file.close();
        file.resize(0);
        return false;
    }

    QString col_name("settings_ident_string");
    QSqlQuery query_db_version(QString("SELECT %1 FROM t_SystemInfo").arg(col_name));
    if (!query_db_version.exec() || !query_db_version.first()) {
        w.writeEndDocument();
        file.close();
        file.resize(0);
        return false;
    }

    w.writeStartElement("PPK_Settings");
    w.writeAttribute("version", query_db_version.value(col_name).toString());

    foreach(QString tablename, db.tables()) {
        w.writeStartElement("table");
        w.writeAttribute("name", tablename);

        QSqlQuery q(QString("SELECT * FROM %1").arg(tablename));
        if (!q.exec()) {
            w.writeEndDocument();
            file.close();
            file.resize(0);
            return false;
        }

        while (q.next()) {
            QSqlRecord rec = q.record();
            w.writeStartElement("tr");

            if (-1 != rec.indexOf("id"))
                w.writeAttribute("id", rec.value("id").toString());

            for (int i = 0; i < rec.count(); ++i ) {
                if (0 == rec.fieldName(i).compare("id"))
                    continue;

                w.writeStartElement("td");
                w.writeAttribute("name", rec.fieldName(i));
                w.writeAttribute("type", QString::number(rec.value(i).type()));
                w.writeCharacters( toString(rec.value(i)) );
                w.writeEndElement();
            }

            w.writeEndElement();
        }

        w.writeEndElement();
    }

    w.writeEndElement();
    w.writeEndDocument();
    file.close();

    return true;
}

QString XmlSettingsProvider::toString(const QVariant &value)
{
    switch (value.type()) {
    case QVariant::LongLong:
    case QVariant::Double:
    case QVariant::String:
        return value.toString();

    case QVariant::ByteArray:
    {
        QString str;
        QByteArray b(value.toByteArray());
        for (int i = 0; i < b.size(); ++i) {
            str.append(QString::number( static_cast<quint8>(b.at(i)) ));
            str.append(" ");
        }
        str.chop(1);
        return str;
    }

    case QVariant::Bool:
        return value.toBool() ? "true" : "false";

    default:
        return "";
    }
}

QVariant XmlSettingsProvider::fromString(int dataType, const QString &value)
{
    switch (dataType) {
    case QVariant::LongLong:
        return QVariant(value.toInt());

    case QVariant::Double:
        return QVariant(value.toDouble());

    case QVariant::String:
        return QVariant(value);

    case QVariant::ByteArray:
    {
        QByteArray b;
        foreach (QString s, value.split(" "))
            b.append( static_cast<char>(s.toShort()) );
        return b;
    }

    case QVariant::Bool:
        return QVariant(value.compare("true", Qt::CaseInsensitive) == 0 ? true : false);

    default:
        return QVariant();
    }
}
