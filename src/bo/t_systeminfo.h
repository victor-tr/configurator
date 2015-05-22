#ifndef T_SYSTEMINFO_H
#define T_SYSTEMINFO_H

class ARMOR_DLL_EXPORT t_SystemInfo
{

public:

    t_SystemInfo() {}
    virtual ~t_SystemInfo() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long    _id;

    QString _settings_ident_string;
    long    _settings_signature;

};

ARMOR_QX_REGISTER_HPP(t_SystemInfo, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_SystemInfo> t_SystemInfo_ptr;
typedef qx::QxCollection<long, t_SystemInfo_ptr> t_SystemInfoX;


#endif // T_SYSTEMINFO_H
