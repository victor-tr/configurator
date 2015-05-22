#include "precompiled.h"
#include "t_zone.h"
#include "s_parentunit.h"
#include "configurator_protocol.h"
#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Zone)

namespace qx {

template<>
void register_class(QxClass<t_Zone> &t)
{
    t.id(&t_Zone::_id, "id");

    t.data(&t_Zone::_humanizedId, "humanizedId");
    t.data(&t_Zone::_alias, "alias");
    t.data(&t_Zone::_suin, "suin");
    t.data(&t_Zone::_bEnabled, "bEnabled");
    t.data(&t_Zone::_bDamageNotificationEnabled, "bDamageNotification");
    t.data(&t_Zone::_zoneType, "zoneType");

    t.relationManyToOne(&t_Zone::_arming_group, "arming_group_id");
    t.relationManyToOne(&t_Zone::_pParent, "parent_id");
}

}


QSharedPointer<QByteArray> t_Zone::fetchToByteArray()
{
    t_ZoneX list;

    QStringList relation;
    relation << "arming_group_id" << "parent_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_ZONE);
        ret->append(static_cast<char>(ZPKT_MAX >> 8));
        ret->append(ZPKT_MAX);

        t_Zone_ptr item;
        _foreach (item, list) {
            QByteArray temp(ZPKT_MAX, '\0');

            long id = item->_id;
            temp[ ZPKT_ID_H ] = (id >> 8);
            temp[ ZPKT_ID_L ] = id;
            temp[ ZPKT_HUMANIZED_ID_H ] = (item->_humanizedId >> 8);
            temp[ ZPKT_HUMANIZED_ID_L ] = item->_humanizedId;

            strncpy(temp.data()+ZPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ ZPKT_SUIN ] = item->_suin;
            temp[ ZPKT_ENABLE_FLAG ] = item->_bEnabled;
            temp[ ZPKT_LOOP_DAMAGE_NOTIFICATION_FLAG ] = item->_bDamageNotificationEnabled;
            temp[ ZPKT_ZONE_TYPE ] = item->_zoneType;

            long ag_id = item->_arming_group.isNull() ? DB_VALUE_NULL : item->_arming_group->_id;
            temp[ ZPKT_ARMING_GROUP_ID_H ] = (ag_id >> 8);
            temp[ ZPKT_ARMING_GROUP_ID_L ] = ag_id;

            if (!item->_pParent.isNull()) {
                temp[ ZPKT_PARENTDEV_UIN_H ] = (item->_pParent->_puin >> 8);
                temp[ ZPKT_PARENTDEV_UIN_L ] = item->_pParent->_puin;
                temp[ ZPKT_PARENTDEV_TYPE ] = item->_pParent->_ptype;
            } else {
                temp[ ZPKT_PARENTDEV_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ ZPKT_PARENTDEV_UIN_L ] = DB_VALUE_NULL;
                temp[ ZPKT_PARENTDEV_TYPE ] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Zone::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / ZPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Zone_ptr pZone(new t_Zone);

        pZone->_id =          (quint8)data->at(ZPKT_ID_H + i*ZPKT_MAX) << 8 | (quint8)data->at(ZPKT_ID_L + i*ZPKT_MAX);
        pZone->_humanizedId = (quint8)data->at(ZPKT_HUMANIZED_ID_H + i*ZPKT_MAX) << 8 | (quint8)data->at(ZPKT_HUMANIZED_ID_L + i*ZPKT_MAX);
        pZone->_alias =       GET_TEXT_CODEC->toUnicode(data->mid(ZPKT_ALIAS_H + i*ZPKT_MAX, ALIAS_LEN).constData());
        pZone->_suin =        (quint8)data->at(ZPKT_SUIN + i*ZPKT_MAX);
        pZone->_bEnabled =    (quint8)data->at(ZPKT_ENABLE_FLAG + i*ZPKT_MAX);
        pZone->_bDamageNotificationEnabled = (quint8)data->at(ZPKT_LOOP_DAMAGE_NOTIFICATION_FLAG + i*ZPKT_MAX);
        pZone->_zoneType =    (quint8)data->at(ZPKT_ZONE_TYPE + i*ZPKT_MAX);

        qint16 groupId = (quint8)data->at(ZPKT_ARMING_GROUP_ID_H + i*ZPKT_MAX) << 8 | (quint8)data->at(ZPKT_ARMING_GROUP_ID_L + i*ZPKT_MAX);

        int pType = (quint8)data->at(ZPKT_PARENTDEV_TYPE + i*ZPKT_MAX);
        int pUin =  (quint8)data->at(ZPKT_PARENTDEV_UIN_H + i*ZPKT_MAX) << 8 | (quint8)data->at(ZPKT_PARENTDEV_UIN_L + i*ZPKT_MAX);
        int parentId = s_ParentUnit::getParentId(pUin, pType, db);

        qx_query q2("INSERT INTO t_Zone (id, parent_id, humanizedId, alias, suin, bEnabled, arming_group_id, zoneType, bDamageNotification) "
                    "VALUES (:id, :parent_id, :humanizedId, :alias, :suin, :bEnabled, :arming_group_id, :zoneType, :bDamageNotification)");
        q2.bind(":id", static_cast<int>(pZone->_id));
        q2.bind(":parent_id", parentId);
        q2.bind(":humanizedId", static_cast<int>(pZone->_humanizedId));
        q2.bind(":alias", pZone->_alias);
        q2.bind(":suin", static_cast<int>(pZone->_suin));
        q2.bind(":bEnabled", pZone->_bEnabled);
        q2.bind(":bDamageNotification", pZone->_bDamageNotificationEnabled);
        q2.bind(":zoneType", static_cast<int>(pZone->_zoneType));
        q2.bind(":arming_group_id", DB_VALUE_NULL == groupId ? QVariant() : groupId);
        qx::dao::call_query(q2, db);
    }
}

QString createZoneAlias(QString pattern, t_ZoneX *list)
{
    static QString temp;
    if (temp.isEmpty()) temp = pattern + "_";

    for (int i = 0; i < list->size(); ++i) {
        QString alias = list->getByIndex(i)->_alias;
        if (0 == alias.compare(pattern, Qt::CaseInsensitive)) {
            int sfx = alias.remove(temp).toInt() + 1;
            pattern = temp + QString::number(sfx);
            pattern = createZoneAlias(pattern, list);
            temp.clear();
            return pattern;
        }
    }

    temp.clear();
    return pattern;
}
