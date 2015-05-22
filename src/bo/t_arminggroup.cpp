#include "precompiled.h"

#include "t_arminggroup.h"
#include "t_zone.h"
#include "t_key.h"
#include "t_etr.h"
#include "t_led.h"
#include "t_relay.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_ArmingGroup)

namespace qx {

template<>
void register_class(QxClass<t_ArmingGroup> &t)
{
    t.id(&t_ArmingGroup::_id, "id");
    t.data(&t_ArmingGroup::_alias, "alias");

    t.relationOneToMany(&t_ArmingGroup::_zones_list, "zones_list", "arming_group_id");
    t.relationOneToMany(&t_ArmingGroup::_keys_list,  "keys_list",  "arming_group_id");
    t.relationManyToMany(&t_ArmingGroup::_etr_list,  "etr_list", "s_ArmingGroup_ETR", "arming_group_id", "etr_id");
    t.relationOneToMany(&t_ArmingGroup::_led_list, "led_list", "group_id");
    t.relationOneToMany(&t_ArmingGroup::_relay_list, "relay_list", "group_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_ArmingGroup::fetchToByteArray()
{
    t_ArmingGroupX list;

    qx::dao::fetch_all(list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_ARMING_GROUP));
        ret->append(static_cast<char>(AGPKT_MAX >> 8));
        ret->append(AGPKT_MAX);

        t_ArmingGroup_ptr item;
        _foreach (item, list) {
            QByteArray temp(AGPKT_MAX, '\0');

            long id = item->_id;
            temp[ AGPKT_ID_H ] = (id >> 8);
            temp[ AGPKT_ID_L ] = id;

            strncpy(temp.data()+AGPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            // -- get amount of related ETRs
            qx_query q("SELECT COUNT(etr_id) FROM s_ArmingGroup_ETR WHERE arming_group_id = :group_id");
            q.bind(":group_id", (int)item->_id);
            qx::dao::call_query(q);

            int etr_qty = q.getSqlResultAt(0,0).toInt();
            temp[ AGPKT_RELATED_ETRs_QTY_H ] = (etr_qty >> 8);
            temp[ AGPKT_RELATED_ETRs_QTY_L ] = etr_qty;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_ArmingGroup::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / AGPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_ArmingGroup_ptr pGroup(new t_ArmingGroup);
        pGroup->_id = (quint8)data->at(AGPKT_ID_H + i*AGPKT_MAX) << 8 | (quint8)data->at(AGPKT_ID_L + i*AGPKT_MAX);
        pGroup->_alias = GET_TEXT_CODEC->toUnicode(data->mid(AGPKT_ALIAS_H + i*AGPKT_MAX, ALIAS_LEN).constData());

        qx_query q1("INSERT INTO t_ArmingGroup (id, alias) VALUES (:id, :alias)");
        q1.bind(":id", static_cast<int>(pGroup->_id));
        q1.bind(":alias", pGroup->_alias);
        qx::dao::call_query(q1, db);
    }
}
