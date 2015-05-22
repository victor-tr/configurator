#include "precompiled.h"

#include "t_etr.h"
#include "t_arminggroup.h"
#include "t_zone.h"
#include "t_button.h"
#include "t_led.h"

#include "s_parentunit.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Etr)

namespace qx {

template<>
void register_class(QxClass<t_Etr> &t)
{
    t.id(&t_Etr::_id, "id");
    t.data(&t_Etr::_etr_type, "etr_type");

    t.relationManyToMany(&t_Etr::_groups_list, "groups_list", "s_ArmingGroup_ETR", "etr_id", "arming_group_id");
    t.relationManyToOne(&t_Etr::_uin, "uin_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_Etr::fetchToByteArray()
{
    t_EtrX list;

    QStringList relation;
    relation << "groups_list" << "uin_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_ETR);
        ret->append(static_cast<char>(ETRPKT_MAX >> 8));
        ret->append(ETRPKT_MAX);

        t_Etr_ptr item;
        _foreach (item, list) {
            QByteArray temp(ETRPKT_MAX, '\0');

            temp[ ETRPKT_ID_H ] = (item->_id >> 8);
            temp[ ETRPKT_ID_L ] = item->_id;

            if (!item->_uin.isNull()) {
                temp[ ETRPKT_PARENT_ID_H ] = item->_uin->_id >> 8;
                temp[ ETRPKT_PARENT_ID_L ] = item->_uin->_id;
                temp[ ETRPKT_UIN_H ] = (item->_uin->_puin >> 8);
                temp[ ETRPKT_UIN_L ] = item->_uin->_puin;

                strncpy(temp.data()+ETRPKT_ALIAS_H,
                        GET_TEXT_CODEC->fromUnicode(item->_uin->_palias).constData(),
                        ALIAS_LEN);
            } else {
                temp[ ETRPKT_PARENT_ID_H ] = DB_VALUE_NULL >> 8;
                temp[ ETRPKT_PARENT_ID_L ] = DB_VALUE_NULL;
                temp[ ETRPKT_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ ETRPKT_UIN_L ] = DB_VALUE_NULL;
            }

            temp[ ETRPKT_ETR_TYPE ] = item->_etr_type;

            // -- get amount of related arming groups
            qx_query q("SELECT COUNT(arming_group_id) FROM s_ArmingGroup_ETR WHERE etr_id = :etr_id");
            q.bind(":etr_id", (int)item->_id);
            qx::dao::call_query(q);

            int group_qty = q.getSqlResultAt(0,0).toInt();
            temp[ ETRPKT_RELATED_GROUPS_QTY_H ] = (group_qty >> 8);
            temp[ ETRPKT_RELATED_GROUPS_QTY_L ] = group_qty;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Etr::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / ETRPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Etr_ptr pEtr(new t_Etr);

        pEtr->_id = (quint8)data->at(ETRPKT_ID_H + i*ETRPKT_MAX) << 8 | (quint8)data->at(ETRPKT_ID_L + i*ETRPKT_MAX);
        pEtr->_etr_type = (qint8)data->at(ETRPKT_ETR_TYPE + i*ETRPKT_MAX);

        pEtr->_uin = s_ParentUnit_ptr(new s_ParentUnit);
        pEtr->_uin->_id =     (quint8)data->at(ETRPKT_PARENT_ID_H + i*ETRPKT_MAX) << 8 | (quint8)data->at(ETRPKT_PARENT_ID_L + i*ETRPKT_MAX);
        pEtr->_uin->_ptype =  OBJ_ETR;
        pEtr->_uin->_puin =   (quint8)data->at(ETRPKT_UIN_H + i*ETRPKT_MAX) << 8 | (quint8)data->at(ETRPKT_UIN_L + i*ETRPKT_MAX);
        pEtr->_uin->_pid =    pEtr->_id;
        pEtr->_uin->_palias = GET_TEXT_CODEC->toUnicode(data->mid(ETRPKT_ALIAS_H + i*ETRPKT_MAX, ALIAS_LEN).constData());


        qx_query q1("INSERT INTO t_ETR (id, uin_id, etr_type) VALUES (:id, :uin_id, :etr_type)");
        q1.bind(":id", static_cast<int>(pEtr->_id));
        q1.bind(":uin_id", pEtr->_uin->_id);
        q1.bind(":etr_type", static_cast<int>(pEtr->_etr_type));
        qx::dao::call_query(q1, db);

        qx_query q2("INSERT INTO s_Parent (id, ptype, pid, puin, palias) VALUES (:id, :ptype, :pid, :puin, :palias)");
        q2.bind(":id", pEtr->_uin->_id);
        q2.bind(":ptype", pEtr->_uin->_ptype);
        q2.bind(":pid", pEtr->_uin->_pid);
        q2.bind(":puin", pEtr->_uin->_puin);
        q2.bind(":palias", pEtr->_uin->_palias);
        qx::dao::call_query(q2, db);
    }
}

void t_Etr::appendZones(int amount)
{
    if (!amount)
        return;

    qx_query q("SELECT MAX(id) FROM t_Zone");
    qx::dao::call_query(q);
    int next = q.getSqlResultAt(0,0).toInt() + 1;

    t_ArmingGroup_ptr pGroup(new t_ArmingGroup());
    pGroup->_id = 1;    // first group in list => default group

    t_ZoneX zonelist;

    for (int i = 0; i < amount; ++i) {
        t_Zone_ptr pZone (new t_Zone());
        pZone->_alias = createZoneAlias(QString("Zone %1").arg(next + i), &zonelist);
        pZone->_bDamageNotificationEnabled = true;
        pZone->_bEnabled = true;
        pZone->_arming_group = pGroup;
        pZone->_zoneType = 0;
        pZone->_pParent = _uin;
        pZone->_humanizedId = next + i;
        pZone->_id = next + i;
        pZone->_suin = 1 + i;        // subUIN starts from 1
        zonelist.insert(pZone->_id, pZone);
    }

    QStringList relation;
    relation << "parent_id";
    qx::dao::save_with_relation(relation, zonelist);
}

void t_Etr::removeZones()
{
    // -- cascaded deletion of all Zones of the ETR
    qx_query q("DELETE FROM t_Zone WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all zones
    q = qx_query("UPDATE t_Zone SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Etr::appendLeds()
{
    qx_query q("SELECT MAX(id) FROM t_Led");
    qx::dao::call_query(q);
    int next = q.getSqlResultAt(0,0).toInt() + 1;
    int next_suin = 1;

    t_LedX ledlist;

    // ********************** begin ******************************
    // -- fill the list of leds here
    for (int i = 0; i < 4; ++i) {
        t_Led_ptr pLed (new t_Led());
        pLed->_bEnabled = true;
        pLed->_pParent = _uin;
        pLed->_id = next;
        pLed->_humanizedId = next;
        pLed->_suin = next_suin;        // subUIN starts from 1
        pLed->_bArmingLed = false;

        switch (pLed->_suin) {
        case 1:
            pLed->_alias = QString("Arming Led on %1").arg(_uin->_palias);
            break;
        case 2:
            pLed->_alias = QString("Power Led on %1").arg(_uin->_palias);
            break;
        case 3:
            pLed->_alias = QString("Battery Led on %1").arg(_uin->_palias);
            break;
        case 4:
            pLed->_alias = QString("System Led on %1").arg(_uin->_palias);
            break;

        }

        ledlist.insert(pLed->_id, pLed);

        next++;         // will needed if there are more than one LED
        next_suin++;
    }
    // ********************* end *******************************

    QStringList relation;
    relation << "parent_id";
    qx::dao::save_with_relation(relation, ledlist);
}

void t_Etr::removeLeds()
{
    // -- cascaded deletion of all relays of the ETR
    qx_query q("DELETE FROM t_Led WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all relays
    q = qx_query("UPDATE t_Led SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Etr::appendButtons()
{
    qx_query q("SELECT MAX(id) FROM t_Button");
    qx::dao::call_query(q);
    int next = q.getSqlResultAt(0,0).toInt() + 1;
    int next_suin = 1;

    t_ButtonX buttonlist;

    // ********************** begin ******************************
    // -- fill the list of leds here
    for (int i = 0; i < 4; ++i) {
        t_Button_ptr pButton (new t_Button());
        pButton->_bEnabled = true;
        pButton->_pParent = _uin;
        pButton->_id = next;
        pButton->_humanizedId = next;
        pButton->_suin = next_suin;        // subUIN starts from 1

        switch (pButton->_suin) {
        case 1:
            pButton->_alias = QString("Violation Button on %1").arg(_uin->_palias);
            break;
        case 2:
            pButton->_alias = QString("Call Button on %1").arg(_uin->_palias);
            break;
        case 3:
            pButton->_alias = QString("UP Button on %1").arg(_uin->_palias);
            break;
        case 4:
            pButton->_alias = QString("DOWN Button on %1").arg(_uin->_palias);
            break;

        }

        buttonlist.insert(pButton->_id, pButton);

        next++;         // will needed if there are more than one Button
        next_suin++;
    }
    // ********************* end *******************************

    QStringList relation;
    relation << "parent_id";
    qx::dao::save_with_relation(relation, buttonlist);
}

void t_Etr::removeButtons()
{
    // -- cascaded deletion of all relays of the ETR
    qx_query q("DELETE FROM t_Button WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all relays
    q = qx_query("UPDATE t_Button SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Etr::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)

    if (Etr_type_with_4_zones == _etr_type)
        appendZones(4);
    appendLeds();
    appendButtons();
}

void t_Etr::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)

    removeZones();
    removeLeds();
    removeButtons();
}

void t_Etr::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
