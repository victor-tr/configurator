#ifndef T_SIMCARD_H
#define T_SIMCARD_H


class t_CommonSettings;
class t_Phone;
class t_IpAddress;

class ARMOR_DLL_EXPORT t_SimCard
{
public:

    typedef boost::shared_ptr<t_CommonSettings> t_CommonSettings_ptr;
    typedef qx::dao::ptr<t_Phone> t_Phone_ptr;
    typedef qx::QxCollection<long, t_Phone_ptr> t_PhoneX;
    typedef qx::dao::ptr<t_IpAddress> t_IpAddress_ptr;
    typedef qx::QxCollection<long, t_IpAddress_ptr> t_IpAddressX;

    t_SimCard() : _id(-1) {}
    virtual ~t_SimCard() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long _id;

    bool _usable;
    bool _prefer_gprs;
    bool _allow_sms;

    long _gprs_attempts_qty;
    long _voicecall_attempts_qty;
    long _udp_iplist_default_size;
    long _tcp_iplist_default_size;
    long _phonelist_default_size;
    long _udp_dest_port;
    long _udp_local_port;
    long _tcp_dest_port;
    long _tcp_local_port;

    QString _apn;
    QString _login;
    QString _password;

    t_CommonSettings_ptr _p_commonSettings;

    t_PhoneX             _phoneNumbers_list;
    t_IpAddressX         _ipAddresses_list;

};

ARMOR_QX_REGISTER_HPP(t_SimCard, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_SimCard> t_SimCard_ptr;
typedef qx::QxCollection<long, t_SimCard_ptr> t_SimCardX;
typedef qx::dao::ptr<t_SimCardX> t_SimCardX_ptr;


#endif // T_SIMCARD_H
