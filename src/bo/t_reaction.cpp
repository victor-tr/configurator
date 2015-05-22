#include "precompiled.h"

#include "t_reaction.h"
#include "t_behaviorpreset.h"
#include "t_event.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Reaction)

namespace qx {

template<>
void register_class(QxClass<t_Reaction> &t)
{
    t.id(&t_Reaction::_id, "id");

    t.data(&t_Reaction::_alias, "alias");
    t.data(&t_Reaction::_performer_type, "performer_type");
    t.data(&t_Reaction::_performer_id, "performer_id");
    t.data(&t_Reaction::_performer_behavior, "performer_behavior");
    t.data(&t_Reaction::_valid_states, "valid_states");
    t.data(&t_Reaction::_bReversible, "reversible");
    t.data(&t_Reaction::_reverse_performer_behavior, "reverse_performer_behavior");

    t.relationManyToMany(&t_Reaction::_behavior_preset_list, "behavior_preset_list",
                         "s_Reaction_BehaviorPreset", "reaction_id", "behavior_preset_id");
    t.relationManyToOne(&t_Reaction::_event, "event_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_Reaction::fetchToByteArray()
{
    t_ReactionX list;

    QStringList relation;
    relation << "behavior_preset_list";
    relation << "event_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_REACTION));
        ret->append(static_cast<char>(RCNPKT_MAX >> 8));
        ret->append(RCNPKT_MAX);

        t_Reaction_ptr item;
        _foreach (item, list) {
            QByteArray temp(RCNPKT_MAX, '\0');

            temp[ RCNPKT_ID_H ] = (item->_id >> 8);
            temp[ RCNPKT_ID_L ] = item->_id;

            temp[ RCNPKT_PERFORMER_TYPE ] = item->_performer_type;
            temp[ RCNPKT_PERFORMER_ID_H ] = (item->_performer_id >> 8);
            temp[ RCNPKT_PERFORMER_ID_L ] = item->_performer_id;
            temp[ RCNPKT_PERFORMER_BEHAVIOR ] = item->_performer_behavior;

            int preset_id = (item->_behavior_preset_list.size() > 0 &&
                                    PB_UsePreset == item->_performer_behavior) ?
                                item->_behavior_preset_list.getByIndex(0)->_id :
                                DB_VALUE_NULL;
            temp[ RCNPKT_BEHAVIOR_PRESET_ID_H ] = (preset_id >> 8);
            temp[ RCNPKT_BEHAVIOR_PRESET_ID_L ] = preset_id;

            int event_id = item->_event.isNull() ? DB_VALUE_NULL : item->_event->_id;
            temp[ RCNPKT_EVENT_ID_H ] = (event_id >> 8);
            temp[ RCNPKT_EVENT_ID_L ] = event_id;

            strncpy(temp.data()+RCNPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            for (int j = 0; j < STATE_MAX; ++j) {
                if (j < item->_valid_states.size())
                    temp[ RCNPKT_VALID_STATES_H + j ] = item->_valid_states.at(j);
            }

            temp[RCNPKT_REVERSIBLE_FLAG] = item->_bReversible;
            temp[RCNPKT_REVERSE_PERFORMER_BEHAVIOR] = item->_reverse_performer_behavior;

            preset_id = (item->_behavior_preset_list.size() > 1) ?
                            item->_behavior_preset_list.getByIndex(1)->_id :
                            (item->_behavior_preset_list.size() > 0 &&
                                    PB_UsePreset == item->_reverse_performer_behavior) ?
                                item->_behavior_preset_list.getByIndex(0)->_id :
                                DB_VALUE_NULL;
            temp[RCNPKT_REVERSE_BEHAVIOR_PRESET_ID_H] = preset_id >> 8;
            temp[RCNPKT_REVERSE_BEHAVIOR_PRESET_ID_L] = preset_id;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Reaction::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / RCNPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Reaction_ptr pReaction(new t_Reaction);
        pReaction->_id = (quint8)data->at(RCNPKT_ID_H + i*RCNPKT_MAX) << 8 | (quint8)data->at(RCNPKT_ID_L + i*RCNPKT_MAX);
        pReaction->_alias = GET_TEXT_CODEC->toUnicode(data->mid(RCNPKT_ALIAS_H + i*RCNPKT_MAX, ALIAS_LEN).constData());
        pReaction->_performer_type = (quint8)data->at(RCNPKT_PERFORMER_TYPE + i*RCNPKT_MAX);
        pReaction->_performer_id = (quint8)data->at(RCNPKT_PERFORMER_ID_H + i*RCNPKT_MAX) << 8 |
                                        (quint8)data->at(RCNPKT_PERFORMER_ID_L + i*RCNPKT_MAX);
        pReaction->_performer_behavior = (quint8)data->at(RCNPKT_PERFORMER_BEHAVIOR + i*RCNPKT_MAX);
        pReaction->_valid_states = data->mid(RCNPKT_VALID_STATES_H + i*RCNPKT_MAX, STATE_MAX);
        pReaction->_bReversible = (quint8)data->at(RCNPKT_REVERSIBLE_FLAG + i*RCNPKT_MAX);
        pReaction->_reverse_performer_behavior = (quint8)data->at(RCNPKT_REVERSE_PERFORMER_BEHAVIOR + i*RCNPKT_MAX);

        qint16 eventId = (quint8)data->at(RCNPKT_EVENT_ID_H + i*RCNPKT_MAX) << 8 | (quint8)data->at(RCNPKT_EVENT_ID_L + i*RCNPKT_MAX);

        qx_query q1("INSERT INTO t_Reaction (id, alias, performer_type, performer_id, performer_behavior, "
                    " valid_states, reversible, reverse_performer_behavior, event_id) "
                    "VALUES (:id, :alias, :performer_type, :performer_id, :performer_behavior, "
                    ":valid_states, :reversible, :reverse_performer_behavior, :event_id)");
        q1.bind(":id", static_cast<int>(pReaction->_id));
        q1.bind(":alias", pReaction->_alias);
        q1.bind(":performer_type", pReaction->_performer_type);
        q1.bind(":performer_id", pReaction->_performer_id);
        q1.bind(":performer_behavior", pReaction->_performer_behavior);
        q1.bind(":valid_states", pReaction->_valid_states);
        q1.bind(":reversible", pReaction->_bReversible);
        q1.bind(":reverse_performer_behavior", pReaction->_reverse_performer_behavior);
        q1.bind(":event_id", DB_VALUE_NULL == eventId ? QVariant() : eventId);
        qx::dao::call_query(q1, db);

        qint16 preset_id = (quint8)data->at(RCNPKT_BEHAVIOR_PRESET_ID_H + i*RCNPKT_MAX) << 8 |
                                 (quint8)data->at(RCNPKT_BEHAVIOR_PRESET_ID_L + i*RCNPKT_MAX);
        if (DB_VALUE_NULL != preset_id) {
            qx_query q2("INSERT INTO s_Reaction_BehaviorPreset (reaction_id, behavior_preset_id) "
                        "VALUES (:reaction_id, :behavior_preset_id)");
            q2.bind(":reaction_id", pReaction->_id);
            q2.bind(":behavior_preset_id", preset_id);
            qx::dao::call_query(q2, db);
        }

        qint16 reverse_preset_id = (quint8)data->at(RCNPKT_REVERSE_BEHAVIOR_PRESET_ID_H + i*RCNPKT_MAX) << 8 |
                                     (quint8)data->at(RCNPKT_REVERSE_BEHAVIOR_PRESET_ID_L + i*RCNPKT_MAX);
        if (DB_VALUE_NULL != reverse_preset_id) {
            qx_query q2("INSERT INTO s_Reaction_BehaviorPreset (reaction_id, behavior_preset_id) "
                        "VALUES (:reaction_id, :behavior_preset_id)");
            q2.bind(":reaction_id", pReaction->_id);
            q2.bind(":behavior_preset_id", reverse_preset_id);
            qx::dao::call_query(q2, db);
        }
    }
}
