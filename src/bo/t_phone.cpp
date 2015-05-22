#include "precompiled.h"
#include "t_phone.h"
#include "t_simcard.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Phone)

namespace qx {

template <> void register_class(QxClass<t_Phone> &t)
{
    t.id(&t_Phone::_id, "id");

    t.data(&t_Phone::_phone, "phone");
    t.data(&t_Phone::_idx, "idx");
    t.data(&t_Phone::_allowed_simcard, "allowed_simcard");

    t.relationManyToOne(&t_Phone::_p_simcard, "simcard");
}


} // namespace qx

QSharedPointer<QByteArray> t_Phone::fetchToByteArray(SimCard number)
{
    t_PhoneX list;

    qx::QxSqlQuery query("WHERE simcard = :sim");
    query.bind(":sim", static_cast<int>(number));
    qx::dao::fetch_by_query(query, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        switch (number) {
        case SIM_1:
            ret->append(static_cast<char>(OBJ_PHONELIST_SIM1));
            break;
        case SIM_2:
            ret->append(static_cast<char>(OBJ_PHONELIST_SIM2));
            break;
        default:
            return QSharedPointer<QByteArray>();
        }
        ret->append(static_cast<char>(PHNPKT_MAX >> 8));
        ret->append(PHNPKT_MAX);

        t_Phone_ptr item;
        _foreach (item, list) {
            QByteArray temp(PHNPKT_MAX, '\0');

            temp[PHNPKT_ID] = item->_idx;

            strncpy(temp.data()+PHNPKT_PHONENUMBER_H,
                    GET_TEXT_CODEC->fromUnicode(item->_phone).constData(),
                    PHONE_LEN);

            temp[PHNPKT_ALLOWED_SIMCARD] = item->_allowed_simcard;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Phone::insertFromByteArray(SimCard number, QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / PHNPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Phone_ptr pPhone(new t_Phone);

        pPhone->_id = SIM_1 == number ? i+1 : i+3;
        pPhone->_idx = (quint8)data->at(PHNPKT_ID + i*PHNPKT_MAX);
        pPhone->_phone = GET_TEXT_CODEC->toUnicode(data->mid(PHNPKT_PHONENUMBER_H + i*PHNPKT_MAX, PHONE_LEN).constData());
        pPhone->_allowed_simcard = (quint8)data->at(PHNPKT_ALLOWED_SIMCARD + i*PHNPKT_MAX);

        qx_query q1("INSERT INTO t_Phone (id, phone, simcard, idx, allowed_simcard) \
                    VALUES (:id, :phone, :simcard, :idx, :allowed_simcard)");
        q1.bind(":id", static_cast<int>(pPhone->_id));
        q1.bind(":phone", pPhone->_phone);
        q1.bind(":allowed_simcard", static_cast<int>(pPhone->_allowed_simcard));
        q1.bind(":simcard", number);
        q1.bind(":idx", static_cast<int>(pPhone->_idx));
        qx::dao::call_query(q1, db);
    }
}
