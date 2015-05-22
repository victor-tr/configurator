#ifndef T_IPADDRESS_H
#define T_IPADDRESS_H

#include "t_phone.h"


class t_SimCard;

class ARMOR_DLL_EXPORT t_IpAddress
{

public:

    typedef qx::dao::ptr<t_SimCard> t_SimCard_ptr;

    t_IpAddress() : _id(-1) {}
    virtual ~t_IpAddress() {}

    static QSharedPointer<QByteArray> fetchToByteArray(SimCard number);
    static void insertFromByteArray(SimCard number, QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long _id;

    QString _ip;
    long    _idx;

    t_SimCard_ptr _p_simcard;

};


ARMOR_QX_REGISTER_HPP(t_IpAddress, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_IpAddress> t_IpAddress_ptr;
typedef qx::QxCollection<long, t_IpAddress_ptr> t_IpAddressX;
typedef qx::dao::ptr<t_IpAddressX> t_IpAddressX_ptr;


#endif // T_IPADDRESS_H
