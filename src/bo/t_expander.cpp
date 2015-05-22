#include "precompiled.h"

#include "t_expander.h"

#include "t_zone.h"
#include "t_relay.h"
#include "t_led.h"

#include "s_parentunit.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Expander)

namespace qx {

template<>
void register_class(QxClass<t_Expander> &t)
{
    t.id(&t_Expander::_id, "id");
    t.data(&t_Expander::_type, "type");

    t.relationManyToOne(&t_Expander::_uin, "uin_id");
}

}

QSharedPointer<QByteArray> t_Expander::fetchToByteArray(int expanderType)
{
    Q_ASSERT(OBJ_RAE == expanderType || OBJ_RZE == expanderType);

    t_ExpanderX list;

    QStringList relation;
    relation << "uin_id";
    qx::dao::fetch_by_query_with_relation(relation,
                                          qx_query().where("type").isEqualTo(expanderType),
                                          list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(expanderType);
        ret->append(static_cast<char>(EPKT_MAX >> 8));
        ret->append(EPKT_MAX);

        t_Expander_ptr item;
        _foreach (item, list) {
            QByteArray temp(EPKT_MAX, '\0');

            long id = item->_id;
            temp[ EPKT_ID_H ] = (id >> 8);
            temp[ EPKT_ID_L ] = id;
            temp[ EPKT_TYPE ] = item->_type;

            if (!item->_uin.isNull()) {
                temp[ EPKT_PARENT_ID_H ] = item->_uin->_id >> 8;
                temp[ EPKT_PARENT_ID_L ] = item->_uin->_id;
                temp[ EPKT_UIN_H ] = (item->_uin->_puin >> 8);
                temp[ EPKT_UIN_L ] = item->_uin->_puin;

                strncpy(temp.data()+EPKT_ALIAS_H,
                        GET_TEXT_CODEC->fromUnicode(item->_uin->_palias).constData(),
                        ALIAS_LEN);
            } else {
                temp[ EPKT_PARENT_ID_H ] = DB_VALUE_NULL >> 8;
                temp[ EPKT_PARENT_ID_L ] = DB_VALUE_NULL;
                temp[ EPKT_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ EPKT_UIN_L ] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Expander::insertFromByteArray(int expanderType, QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / EPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Expander_ptr pExpander(new t_Expander);

        pExpander->_id =   (quint8)data->at(EPKT_ID_H + i*EPKT_MAX) << 8 | (quint8)data->at(EPKT_ID_L + i*EPKT_MAX);
        pExpander->_type = (qint8)data->at(EPKT_TYPE + i*EPKT_MAX);

        pExpander->_uin = s_ParentUnit_ptr(new s_ParentUnit);
        pExpander->_uin->_id =     (quint8)data->at(EPKT_PARENT_ID_H + i*EPKT_MAX) << 8 | (quint8)data->at(EPKT_PARENT_ID_L + i*EPKT_MAX);
        pExpander->_uin->_ptype =  expanderType;
        pExpander->_uin->_puin =   (quint8)data->at(EPKT_UIN_H + i*EPKT_MAX) << 8 | (quint8)data->at(EPKT_UIN_L + i*EPKT_MAX);
        pExpander->_uin->_pid =    pExpander->_id;
        pExpander->_uin->_palias = GET_TEXT_CODEC->toUnicode(data->mid(EPKT_ALIAS_H + i*EPKT_MAX, ALIAS_LEN).constData());


        qx_query q1("INSERT INTO t_Expander (id, uin_id, type) VALUES (:id, :uin_id, :type)");
        q1.bind(":id", static_cast<int>(pExpander->_id));
        q1.bind(":uin_id", pExpander->_uin->_id);
        q1.bind(":type", static_cast<int>(pExpander->_type));
        qx::dao::call_query(q1, db);

        qx_query q2("INSERT INTO s_Parent (id, ptype, pid, puin, palias) VALUES (:id, :ptype, :pid, :puin, :palias)");
        q2.bind(":id", pExpander->_uin->_id);
        q2.bind(":ptype", pExpander->_uin->_ptype);
        q2.bind(":pid", pExpander->_uin->_pid);
        q2.bind(":puin", pExpander->_uin->_puin);
        q2.bind(":palias", pExpander->_uin->_palias);
        qx::dao::call_query(q2, db);
    }
}

void t_Expander::appendZones(int amount)
{
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

void t_Expander::removeZones()
{
    // -- cascaded deletion of all Zones of the Expander
    qx_query q("DELETE FROM t_Zone WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all zones
    q = qx_query("UPDATE t_Zone SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Expander::appendRelays(int amount)
{
    qx_query q("SELECT MAX(id) FROM t_Relay");
    qx::dao::call_query(q);
    int next = q.getSqlResultAt(0,0).toInt() + 1;

    t_RelayX relaylist;

    for (int i = 0; i < amount; ++i) {
        t_Relay_ptr pRelay (new t_Relay());
        pRelay->_alias = createRelayAlias(QString("Relay %1").arg(next + i), &relaylist);
        pRelay->_pParent = _uin;
        pRelay->_humanizedId = next + i;
        pRelay->_id = next + i;
        pRelay->_suin = 1 + i;        // subUIN starts from 1
        relaylist.insert(pRelay->_id, pRelay);
    }

    QStringList relation;
    relation << "parent_id";
    qx::dao::save_with_relation(relation, relaylist);
}

void t_Expander::removeRelays()
{
    // -- cascaded deletion of all relays of the Expander
    qx_query q("DELETE FROM t_Relay WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all relays
    q = qx_query("UPDATE t_Relay SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Expander::appendLeds()
{
    qx_query q("SELECT MAX(id) FROM t_Led");
    qx::dao::call_query(q);
    int next = q.getSqlResultAt(0,0).toInt() + 1;
    int next_suin = 1;

    t_LedX ledlist;

    // ********************** begin ******************************
    // -- fill the list of leds here
    t_Led_ptr pLed (new t_Led());
    pLed->_alias = QString("System Led on %1").arg(_uin->_palias);
    pLed->_bEnabled = true;
    pLed->_pParent = _uin;
    pLed->_id = next;
    pLed->_humanizedId = next;
    pLed->_suin = next_suin;        // subUIN starts from 1
    pLed->_bArmingLed = false;
    ledlist.insert(pLed->_id, pLed);

    next++;         // will needed if there are more than one LEDs
    next_suin++;

    // ********************* end *******************************

    QStringList relation;
    relation << "parent_id";
    qx::dao::save_with_relation(relation, ledlist);
}

void t_Expander::removeLeds()
{
    // -- cascaded deletion of all relays of the Expander
    qx_query q("DELETE FROM t_Led WHERE parent_id = :id");
    q.bind(":id", _uin->_id);
    qx::dao::call_query(q);

    // -- update humanizedId of all relays
    q = qx_query("UPDATE t_Led SET humanizedId = id WHERE humanizedId != id");
    qx::dao::call_query(q);
}

void t_Expander::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)

    appendLeds();

    switch (_type) {
    case OBJ_RZE:
        appendZones(8); // WARNING: magic number - 8 arming loops
        break;
    case OBJ_RAE:
        appendRelays(6); // WARNING: magic number - 6 relays
        break;
    default:
        break;
    }
}

void t_Expander::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)

    removeLeds();

    switch (_type) {
    case OBJ_RZE:
        removeZones();
        break;
    case OBJ_RAE:
        removeRelays();
        break;
    default:
        break;
    }
}

void t_Expander::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}



//QSqlDatabase db = qx::QxSqlDatabase::getDatabase();
//bool bCommit = db.transaction();

// // -- cascaded deletion of all Zones of the Expander
//qx_query q("DELETE FROM t_Zone WHERE parentDevUIN = :uin AND parentDevType = :type");
//q.bind(":uin", (int)_uin);
//q.bind(":type", (int)_type);
//QSqlError err = qx::dao::call_query(q, &db);
//bCommit = bCommit && !err.isValid();

// // -- update Zones according to the changes of the Expander
//t_ZoneX zlist;
//err = qx::dao::fetch_all(zlist, &db);
//bCommit = bCommit && !err.isValid();
//err = qx::dao::delete_all<t_Zone>(&db);
//bCommit = bCommit && !err.isValid();
//err = qx::dao::insert(zlist, &db);
//bCommit = bCommit && !err.isValid();
//q = qx_query("UPDATE t_Zone SET pultId = id WHERE pultId != id");
//err = qx::dao::call_query(q, &db);
//bCommit = bCommit && !err.isValid();

//if (bCommit) db.commit();
//else db.rollback();
