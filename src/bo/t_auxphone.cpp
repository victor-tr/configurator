#include "precompiled.h"
#include "t_auxphone.h"
#include "t_commonsettings.h"
#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_AuxPhone)

namespace qx {

template <> void register_class(QxClass<t_AuxPhone> &t)
{
    t.id(&t_AuxPhone::_id, "id");

    t.data(&t_AuxPhone::_phone, "phone");
    t.data(&t_AuxPhone::_allowed_simcard, "allowed_simcard");

    t.relationManyToOne(&t_AuxPhone::_p_commonSettings, "common_settings_id");
}


} // namespace qx

QSharedPointer<QByteArray> t_AuxPhone::fetchToByteArray()
{
    t_AuxPhoneX list;

    qx::dao::fetch_all(list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_PHONELIST_AUX));
        ret->append(static_cast<char>(PHNPKT_MAX >> 8));
        ret->append(PHNPKT_MAX);

        t_AuxPhone_ptr item;
        _foreach (item, list) {
            QByteArray temp(PHNPKT_MAX, '\0');

            temp[PHNPKT_ID] = item->_id;

            strncpy(temp.data()+PHNPKT_PHONENUMBER_H,
                    GET_TEXT_CODEC->fromUnicode(item->_phone).constData(),
                    PHONE_LEN);

            temp[PHNPKT_ALLOWED_SIMCARD] = item->_allowed_simcard;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_AuxPhone::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / PHNPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_AuxPhone_ptr pPhone(new t_AuxPhone);

        pPhone->_id = (quint8)data->at(PHNPKT_ID + i*PHNPKT_MAX) + 10;
        pPhone->_phone = GET_TEXT_CODEC->toUnicode(data->mid(PHNPKT_PHONENUMBER_H + i*PHNPKT_MAX, PHONE_LEN).constData());
        pPhone->_allowed_simcard = (quint8)data->at(PHNPKT_ALLOWED_SIMCARD + i*PHNPKT_MAX);

        qx_query q1("INSERT INTO t_AuxPhone (id, phone, allowed_simcard, common_settings_id) \
                    VALUES (:id, :phone, :allowed_simcard, :common_settings_id)");
        q1.bind(":id", static_cast<int>(pPhone->_id));
        q1.bind(":phone", pPhone->_phone);
        q1.bind(":allowed_simcard", static_cast<int>(pPhone->_allowed_simcard));
        q1.bind(":common_settings_id", 1);
        qx::dao::call_query(q1, db);
    }
}
