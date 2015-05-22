#include "precompiled.h"
#include "t_simcard.h"
#include "t_commonsettings.h"
#include "t_phone.h"
#include "t_ipaddress.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_SimCard)

namespace qx {

template <>
void register_class(QxClass<t_SimCard> &t)
{
    t.id(&t_SimCard::_id, "id");

    t.data(&t_SimCard::_usable, "usable");
    t.data(&t_SimCard::_prefer_gprs, "prefer_gprs");
    t.data(&t_SimCard::_allow_sms, "allow_sms");

    t.data(&t_SimCard::_gprs_attempts_qty, "gprs_attempts_qty");
    t.data(&t_SimCard::_voicecall_attempts_qty, "voicecall_attempts_qty");
    t.data(&t_SimCard::_udp_iplist_default_size, "udp_iplist_default_size");
    t.data(&t_SimCard::_tcp_iplist_default_size, "tcp_iplist_default_size");
    t.data(&t_SimCard::_phonelist_default_size, "phonelist_default_size");
    t.data(&t_SimCard::_udp_dest_port, "udp_dest_port");
    t.data(&t_SimCard::_udp_local_port, "udp_local_port");
    t.data(&t_SimCard::_tcp_dest_port, "tcp_dest_port");
    t.data(&t_SimCard::_tcp_local_port, "tcp_local_port");

    t.data(&t_SimCard::_apn, "apn");
    t.data(&t_SimCard::_login, "login");
    t.data(&t_SimCard::_password, "password");

    t.relationManyToOne(&t_SimCard::_p_commonSettings, "common_settings_id");
    t.relationOneToMany(&t_SimCard::_phoneNumbers_list, "phoneNumbers_list", "simcard");
    t.relationOneToMany(&t_SimCard::_ipAddresses_list,  "ipAddresses_list",  "simcard");
}

} // namespace qx


QSharedPointer<QByteArray> t_SimCard::fetchToByteArray()
{
    t_SimCardX_ptr list(new t_SimCardX());

    qx::dao::fetch_all(list);

    QByteArray *ret = list->size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_SIM_CARD);
        ret->append(static_cast<char>(SIMPKT_MAX >> 8));
        ret->append(SIMPKT_MAX);

        t_SimCard_ptr item;
        _foreach (item, *list.data()) {
            QByteArray temp(SIMPKT_MAX, '\0');

            temp[SIMPKT_ID] = item->_id;

            temp[SIMPKT_USABLE] = item->_usable ? 1 : 0;
            temp[SIMPKT_PREFER_GPRS] = item->_prefer_gprs ? 1 : 0;
            temp[SIMPKT_ALLOW_SMS] = item->_allow_sms ? 1 : 0;

            temp[SIMPKT_GPRS_ATTEMPTS_QTY] = item->_gprs_attempts_qty;
            temp[SIMPKT_VOICECALL_ATTEMPTS_QTY] = item->_voicecall_attempts_qty;
            temp[SIMPKT_UDP_IPLIST_DEFAULT_SIZE] = item->_udp_iplist_default_size;
            temp[SIMPKT_TCP_IPLIST_DEFAULT_SIZE] = item->_tcp_iplist_default_size;
            temp[SIMPKT_PHONELIST_DEFAULT_SIZE] = item->_phonelist_default_size;

            temp[SIMPKT_UDP_DESTPORT_H] = item->_udp_dest_port >> 8;
            temp[SIMPKT_UDP_DESTPORT_L] = item->_udp_dest_port;
            temp[SIMPKT_UDP_LOCALPORT_H] = item->_udp_local_port >> 8;
            temp[SIMPKT_UDP_LOCALPORT_L] = item->_udp_local_port;
            temp[SIMPKT_TCP_DESTPORT_H] = item->_tcp_dest_port >> 8;
            temp[SIMPKT_TCP_DESTPORT_L] = item->_tcp_dest_port;
            temp[SIMPKT_TCP_LOCALPORT_H] = item->_tcp_local_port >> 8;
            temp[SIMPKT_TCP_LOCALPORT_L] = item->_tcp_local_port;

            strncpy(temp.data()+SIMPKT_APN_H,
                    GET_TEXT_CODEC->fromUnicode(item->_apn).constData(),
                    APN_LEN);
            strncpy(temp.data()+SIMPKT_LOGIN_H,
                    GET_TEXT_CODEC->fromUnicode(item->_login).constData(),
                    CREDENTIALS_LEN);
            strncpy(temp.data()+SIMPKT_PASS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_password).constData(),
                    CREDENTIALS_LEN);

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_SimCard::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / SIMPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_SimCard_ptr pSimCard(new t_SimCard);
        pSimCard->_id =          (quint8)data->at(SIMPKT_ID + i*SIMPKT_MAX);
        pSimCard->_usable =      (quint8)data->at(SIMPKT_USABLE + i*SIMPKT_MAX);
        pSimCard->_prefer_gprs = (quint8)data->at(SIMPKT_PREFER_GPRS + i*SIMPKT_MAX);
        pSimCard->_allow_sms =   (quint8)data->at(SIMPKT_ALLOW_SMS + i*SIMPKT_MAX);
        pSimCard->_gprs_attempts_qty =       (quint8)data->at(SIMPKT_GPRS_ATTEMPTS_QTY + i*SIMPKT_MAX);
        pSimCard->_voicecall_attempts_qty =  (quint8)data->at(SIMPKT_VOICECALL_ATTEMPTS_QTY + i*SIMPKT_MAX);
        pSimCard->_udp_iplist_default_size = (quint8)data->at(SIMPKT_UDP_IPLIST_DEFAULT_SIZE + i*SIMPKT_MAX);
        pSimCard->_tcp_iplist_default_size = (quint8)data->at(SIMPKT_TCP_IPLIST_DEFAULT_SIZE + i*SIMPKT_MAX);
        pSimCard->_phonelist_default_size =  (quint8)data->at(SIMPKT_PHONELIST_DEFAULT_SIZE + i*SIMPKT_MAX);
        pSimCard->_udp_dest_port =  (quint8)data->at(SIMPKT_UDP_DESTPORT_H + i*SIMPKT_MAX) << 8  | (quint8)data->at(SIMPKT_UDP_DESTPORT_L + i*SIMPKT_MAX);
        pSimCard->_udp_local_port = (quint8)data->at(SIMPKT_UDP_LOCALPORT_H + i*SIMPKT_MAX) << 8 | (quint8)data->at(SIMPKT_UDP_LOCALPORT_L + i*SIMPKT_MAX);
        pSimCard->_tcp_dest_port =  (quint8)data->at(SIMPKT_TCP_DESTPORT_H + i*SIMPKT_MAX) << 8  | (quint8)data->at(SIMPKT_TCP_DESTPORT_L + i*SIMPKT_MAX);
        pSimCard->_tcp_local_port = (quint8)data->at(SIMPKT_TCP_LOCALPORT_H + i*SIMPKT_MAX) << 8 | (quint8)data->at(SIMPKT_TCP_LOCALPORT_L + i*SIMPKT_MAX);

        pSimCard->_apn = GET_TEXT_CODEC->toUnicode(data->mid(SIMPKT_APN_H + i*SIMPKT_MAX, APN_LEN).constData());
        pSimCard->_login = GET_TEXT_CODEC->toUnicode(data->mid(SIMPKT_LOGIN_H + i*SIMPKT_MAX, CREDENTIALS_LEN).constData());
        pSimCard->_password = GET_TEXT_CODEC->toUnicode(data->mid(SIMPKT_PASS_H + i*SIMPKT_MAX, CREDENTIALS_LEN).constData());

        qx_query q1("INSERT INTO t_SimCard (id, usable, prefer_gprs, allow_sms, gprs_attempts_qty, "
                    "voicecall_attempts_qty, udp_iplist_default_size, tcp_iplist_default_size, "
                    "phonelist_default_size, udp_dest_port, udp_local_port, tcp_dest_port, tcp_local_port, "
                    "apn, login, password, common_settings_id) "
                    "VALUES (:id, :usable, :prefer_gprs, :allow_sms, :gprs_attempts_qty, "
                    ":voicecall_attempts_qty, :udp_iplist_default_size, :tcp_iplist_default_size, "
                    ":phonelist_default_size, :udp_dest_port, :udp_local_port, :tcp_dest_port, :tcp_local_port, "
                    ":apn, :login, :password, :common_settings_id)");
        q1.bind(":id", static_cast<int>(pSimCard->_id));
        q1.bind(":usable", static_cast<int>(pSimCard->_usable));
        q1.bind(":prefer_gprs", static_cast<int>(pSimCard->_prefer_gprs));
        q1.bind(":allow_sms", static_cast<int>(pSimCard->_allow_sms));
        q1.bind(":gprs_attempts_qty", static_cast<int>(pSimCard->_gprs_attempts_qty));
        q1.bind(":voicecall_attempts_qty", static_cast<int>(pSimCard->_voicecall_attempts_qty));
        q1.bind(":udp_iplist_default_size", static_cast<int>(pSimCard->_udp_iplist_default_size));
        q1.bind(":tcp_iplist_default_size", static_cast<int>(pSimCard->_tcp_iplist_default_size));
        q1.bind(":phonelist_default_size", static_cast<int>(pSimCard->_phonelist_default_size));
        q1.bind(":udp_dest_port", static_cast<int>(pSimCard->_udp_dest_port));
        q1.bind(":udp_local_port", static_cast<int>(pSimCard->_udp_local_port));
        q1.bind(":tcp_dest_port", static_cast<int>(pSimCard->_tcp_dest_port));
        q1.bind(":tcp_local_port", static_cast<int>(pSimCard->_tcp_local_port));
        q1.bind(":apn", pSimCard->_apn);
        q1.bind(":login", pSimCard->_login);
        q1.bind(":password", pSimCard->_password);
        q1.bind(":common_settings_id", 1);
        qx::dao::call_query(q1, db);
    }
}
