#include "precompiled.h"
#include "t_ipaddress.h"
#include "t_simcard.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_IpAddress)

namespace qx {

template <> void register_class(QxClass<t_IpAddress> &t)
{
    t.id(&t_IpAddress::_id, "id");

    t.data(&t_IpAddress::_ip, "ip");
    t.data(&t_IpAddress::_idx, "idx");

    t.relationManyToOne(&t_IpAddress::_p_simcard, "simcard");
}


} // namespace qx

QSharedPointer<QByteArray> t_IpAddress::fetchToByteArray(SimCard number)
{
    t_IpAddressX list;

    qx::QxSqlQuery query("WHERE simcard = :sim");
    query.bind(":sim", number);
    qx::dao::fetch_by_query(query, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append((number == SIM_1 ? OBJ_IPLIST_SIM1 : OBJ_IPLIST_SIM2));
        ret->append(static_cast<char>(IPAPKT_MAX >> 8));
        ret->append(IPAPKT_MAX);

        t_IpAddress_ptr item;
        _foreach (item, list) {
            QByteArray temp(IPAPKT_MAX, '\0');

            temp[IPAPKT_ID] = item->_idx;

            QStringList ip_parts = item->_ip.split('.');
            if (ip_parts.size() == 4) {
                temp[IPAPKT_IPADDRESS_0] = ip_parts.at(0).toInt();
                temp[IPAPKT_IPADDRESS_1] = ip_parts.at(1).toInt();
                temp[IPAPKT_IPADDRESS_2] = ip_parts.at(2).toInt();
                temp[IPAPKT_IPADDRESS_3] = ip_parts.at(3).toInt();
            } else {
                temp[IPAPKT_IPADDRESS_0] = 0;
                temp[IPAPKT_IPADDRESS_1] = 0;
                temp[IPAPKT_IPADDRESS_2] = 0;
                temp[IPAPKT_IPADDRESS_3] = 0;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_IpAddress::insertFromByteArray(SimCard number, QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / IPAPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_IpAddress_ptr pIpAddress(new t_IpAddress);

        pIpAddress->_id = SIM_1 == number ? i+1 : i+3;
        pIpAddress->_idx = (quint8)data->at(IPAPKT_ID + i*IPAPKT_MAX);

        QStringList temp;
        temp.append(QString::number((quint8)data->at(IPAPKT_IPADDRESS_0 + i*IPAPKT_MAX)));
        temp.append(QString::number((quint8)data->at(IPAPKT_IPADDRESS_1 + i*IPAPKT_MAX)));
        temp.append(QString::number((quint8)data->at(IPAPKT_IPADDRESS_2 + i*IPAPKT_MAX)));
        temp.append(QString::number((quint8)data->at(IPAPKT_IPADDRESS_3 + i*IPAPKT_MAX)));
        pIpAddress->_ip = temp.join('.');

        qx_query q1("INSERT INTO t_IpAddress (id, ip, simcard, idx) VALUES (:id, :ip, :simcard, :idx)");
        q1.bind(":id", static_cast<int>(pIpAddress->_id));
        q1.bind(":ip", pIpAddress->_ip);
        q1.bind(":idx", static_cast<int>(pIpAddress->_idx));
        q1.bind(":simcard", number);
        qx::dao::call_query(q1, db);
    }
}
