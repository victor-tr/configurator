#ifndef T_AUXPHONE_H
#define T_AUXPHONE_H

class t_CommonSettings;

class t_AuxPhone
{
public:

    typedef boost::shared_ptr<t_CommonSettings> t_CommonSettings_ptr;

    t_AuxPhone() : _id(-1) {}
    virtual ~t_AuxPhone() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long _id;

    QString _phone;
    long    _allowed_simcard;

    t_CommonSettings_ptr _p_commonSettings;

};

ARMOR_QX_REGISTER_HPP(t_AuxPhone, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_AuxPhone> t_AuxPhone_ptr;
typedef qx::QxCollection<long, t_AuxPhone_ptr> t_AuxPhoneX;
typedef qx::dao::ptr<t_AuxPhoneX> t_AuxPhoneX_ptr;


#endif // T_AUXPHONE_H
