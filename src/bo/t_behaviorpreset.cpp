#include "precompiled.h"

#include "t_behaviorpreset.h"
#include "t_reaction.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_BehaviorPreset)

namespace qx {

template<>
void register_class(QxClass<t_BehaviorPreset> &t)
{
    t.id(&t_BehaviorPreset::_id, "id");

    t.data(&t_BehaviorPreset::_alias, "alias");
    t.data(&t_BehaviorPreset::_type, "type");
    t.data(&t_BehaviorPreset::_pulses_in_batch, "pulses_in_batch_qty");
    t.data(&t_BehaviorPreset::_pulse_len, "pulse_len");
    t.data(&t_BehaviorPreset::_pause_len, "pause_len");
    t.data(&t_BehaviorPreset::_batch_pause_len, "batch_pause_len");

    t.relationManyToMany(&t_BehaviorPreset::_reactions_list, "reactions_list",
                         "s_Reaction_BehaviorPreset", "behavior_preset_id", "reaction_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_BehaviorPreset::fetchToByteArray()
{
    t_BehaviorPresetX list;

    qx::dao::fetch_all(list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_BEHAVIOR_PRESET));
        ret->append(static_cast<char>(BHPPKT_MAX >> 8));
        ret->append(BHPPKT_MAX);

        t_BehaviorPreset_ptr item;
        _foreach (item, list) {
            QByteArray temp(BHPPKT_MAX, '\0');

            temp[ BHPPKT_ID_H ] = (item->_id >> 8);
            temp[ BHPPKT_ID_L ] = item->_id;

            temp[ BHPPKT_PULSES_IN_BATCH ] = item->_pulses_in_batch;
            temp[ BHPPKT_PULSE_LEN ] = (item->_pulse_len * 10);   // max 255 steps, one step == 100 msec
            temp[ BHPPKT_PAUSE_LEN ] = (item->_pause_len * 10);
            temp[ BHPPKT_BATCH_PAUSE_LEN ] = (item->_batch_pause_len * 10);

            strncpy(temp.data()+BHPPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ BHPPKT_BEHAVIOR_TYPE ] = item->_type;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_BehaviorPreset::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / BHPPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_BehaviorPreset_ptr pBehavior(new t_BehaviorPreset);
        pBehavior->_id = (quint8)data->at(BHPPKT_ID_H + i*BHPPKT_MAX) << 8 | (quint8)data->at(BHPPKT_ID_L + i*BHPPKT_MAX);
        pBehavior->_alias = GET_TEXT_CODEC->toUnicode(data->mid(BHPPKT_ALIAS_H + i*BHPPKT_MAX, ALIAS_LEN).constData());
        pBehavior->_type = (quint8)data->at(BHPPKT_BEHAVIOR_TYPE + i*BHPPKT_MAX);
        pBehavior->_pulses_in_batch = (quint8)data->at(BHPPKT_PULSES_IN_BATCH + i*BHPPKT_MAX);
        pBehavior->_pulse_len = (quint8)data->at(BHPPKT_PULSE_LEN + i*BHPPKT_MAX) / (double)10;
        pBehavior->_pause_len = (quint8)data->at(BHPPKT_PAUSE_LEN + i*BHPPKT_MAX) / (double)10;
        pBehavior->_batch_pause_len = (quint8)data->at(BHPPKT_BATCH_PAUSE_LEN + i*BHPPKT_MAX) / (double)10;

        qx_query q1("INSERT INTO t_BehaviorPreset (id, alias, type, pulses_in_batch_qty, pulse_len, pause_len, batch_pause_len) "
                    "VALUES (:id, :alias, :type, :pulses_in_batch_qty, :pulse_len, :pause_len, :batch_pause_len)");
        q1.bind(":id", static_cast<int>(pBehavior->_id));
        q1.bind(":alias", pBehavior->_alias);
        q1.bind(":type", pBehavior->_type);
        q1.bind(":pulses_in_batch_qty", pBehavior->_pulses_in_batch);
        q1.bind(":pulse_len", pBehavior->_pulse_len);
        q1.bind(":pause_len", pBehavior->_pause_len);
        q1.bind(":batch_pause_len", pBehavior->_batch_pause_len);
        qx::dao::call_query(q1, db);
    }
}
