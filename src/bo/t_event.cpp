#include "precompiled.h"

#include "t_event.h"
#include "t_reaction.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Event)

namespace qx {

template<>
void register_class(QxClass<t_Event> &t)
{
    t.id(&t_Event::_id, "id");

    t.data(&t_Event::_alias, "alias");
    t.data(&t_Event::_event, "event");
    t.data(&t_Event::_emitter_type, "emitter_type");
    t.data(&t_Event::_emitter_id, "emitter_id");

    t.relationOneToMany(&t_Event::_reactions_list, "reactions_list", "event_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_Event::fetchToByteArray()
{
    t_EventX list;

    qx::dao::fetch_all(list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_EVENT));
        ret->append(static_cast<char>(EVTPKT_MAX >> 8));
        ret->append(EVTPKT_MAX);

        t_Event_ptr item;
        _foreach (item, list) {
            QByteArray temp(EVTPKT_MAX, '\0');

            temp[ EVTPKT_ID_H ] = (item->_id >> 8);
            temp[ EVTPKT_ID_L ] = item->_id;

            temp[ EVTPKT_EVENT ] = item->_event;
            temp[ EVTPKT_EMITTER_TYPE ] = item->_emitter_type;
            temp[ EVTPKT_EMITTER_ID_H ] = (item->_emitter_id >> 8);
            temp[ EVTPKT_EMITTER_ID_L ] = item->_emitter_id;

            strncpy(temp.data()+EVTPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            // -- get amount of subscribers for this event
            qx_query q("SELECT COUNT(id) FROM t_Reaction WHERE event_id = :event_id");
            q.bind(":event_id", item->_id);
            qx::dao::call_query(q);

            temp[EVTPKT_SUBSCRIBERS_QTY] = q.getSqlResultAt(0,0).toInt();

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Event::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / EVTPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Event_ptr pEvent(new t_Event);
        pEvent->_id = (quint8)data->at(EVTPKT_ID_H + i*EVTPKT_MAX) << 8 | (quint8)data->at(EVTPKT_ID_L + i*EVTPKT_MAX);
        pEvent->_alias = GET_TEXT_CODEC->toUnicode(data->mid(EVTPKT_ALIAS_H + i*EVTPKT_MAX, ALIAS_LEN).constData());
        pEvent->_event = (quint8)data->at(EVTPKT_EVENT + i*EVTPKT_MAX);
        pEvent->_emitter_type = (quint8)data->at(EVTPKT_EMITTER_TYPE + i*EVTPKT_MAX);
        pEvent->_emitter_id = (quint8)data->at(EVTPKT_EMITTER_ID_H + i*EVTPKT_MAX) << 8 | (quint8)data->at(EVTPKT_EMITTER_ID_L + i*EVTPKT_MAX);

        qx_query q1("INSERT INTO t_Event (id, alias, event, emitter_type, emitter_id) "
                    "VALUES (:id, :alias, :event, :emitter_type, :emitter_id)");
        q1.bind(":id", static_cast<int>(pEvent->_id));
        q1.bind(":alias", pEvent->_alias);
        q1.bind(":event", pEvent->_event);
        q1.bind(":emitter_type", pEvent->_emitter_type);
        q1.bind(":emitter_id", pEvent->_emitter_id);
        qx::dao::call_query(q1, db);
    }
}
