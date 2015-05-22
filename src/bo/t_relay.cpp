#include "precompiled.h"
#include "t_relay.h"
#include "t_arminggroup.h"
#include "s_parentunit.h"
#include "configurator_protocol.h"
#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Relay)

namespace qx {

template<>
void register_class(QxClass<t_Relay> &t)
{
    t.id(&t_Relay::_id, "id");

    t.data(&t_Relay::_humanizedId, "humanizedId");
    t.data(&t_Relay::_alias, "alias");
    t.data(&t_Relay::_suin, "suin");
    t.data(&t_Relay::_bEnabled, "bEnabled");
    t.data(&t_Relay::_bRemoteControl, "bRemoteControl");
    t.data(&t_Relay::_bNotifyOnStateChanged, "bNotifyOnStateChanged");
    t.data(&t_Relay::_loadType, "load_type");

    t.relationManyToOne(&t_Relay::_pParent, "parent_id");
    t.relationManyToOne(&t_Relay::_pGroup, "group_id");
}

}


QSharedPointer<QByteArray> t_Relay::fetchToByteArray()
{
    t_RelayX list;

    QStringList relation;
    relation << "parent_id" << "group_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_RELAY);
        ret->append(static_cast<char>(RPKT_MAX >> 8));
        ret->append(RPKT_MAX);

        t_Relay_ptr item;
        _foreach (item, list) {
            QByteArray temp(RPKT_MAX, '\0');

            long id = item->_id;
            temp[ RPKT_ID_H ] = (id >> 8);
            temp[ RPKT_ID_L ] = id;
            temp[ RPKT_HUMANIZED_ID_H ] = (item->_humanizedId >> 8);
            temp[ RPKT_HUMANIZED_ID_L ] = item->_humanizedId;

            strncpy(temp.data()+RPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ RPKT_SUIN ] = item->_suin;

            temp[ RPKT_ENABLE_FLAG ] = item->_bEnabled;
            temp[ RPKT_REMOTE_CONTROL_FLAG ] = item->_bRemoteControl;
            temp[ RPKT_NOTIFY_ON_STATE_CHANGED_FLAG ] = item->_bNotifyOnStateChanged;

            if (!item->_pParent.isNull()) {
                temp[ RPKT_PARENTDEV_UIN_H ] = (item->_pParent->_puin >> 8);
                temp[ RPKT_PARENTDEV_UIN_L ] = item->_pParent->_puin;
                temp[ RPKT_PARENTDEV_TYPE ] = item->_pParent->_ptype;
            } else {
                temp[ RPKT_PARENTDEV_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ RPKT_PARENTDEV_UIN_L ] = DB_VALUE_NULL;
                temp[ RPKT_PARENTDEV_TYPE ] = DB_VALUE_NULL;
            }

            temp[RPKT_LOAD_TYPE] = item->_loadType;
            if (!item->_pGroup.isNull()) {
                temp[RPKT_GROUP_ID_H] = item->_pGroup->_id >> 8;
                temp[RPKT_GROUP_ID_L] = item->_pGroup->_id;
            } else {
                temp[RPKT_GROUP_ID_H] = DB_VALUE_NULL >> 8;
                temp[RPKT_GROUP_ID_L] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Relay::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / RPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Relay_ptr pRelay(new t_Relay);

        pRelay->_id =             (quint8)data->at(RPKT_ID_H + i*RPKT_MAX) << 8 | (quint8)data->at(RPKT_ID_L + i*RPKT_MAX);
        pRelay->_humanizedId =    (quint8)data->at(RPKT_HUMANIZED_ID_H + i*RPKT_MAX) << 8 | (quint8)data->at(RPKT_HUMANIZED_ID_L + i*RPKT_MAX);
        pRelay->_alias =          GET_TEXT_CODEC->toUnicode(data->mid(RPKT_ALIAS_H + i*RPKT_MAX, ALIAS_LEN).constData());
        pRelay->_suin =           (quint8)data->at(RPKT_SUIN + i*RPKT_MAX);
        pRelay->_bEnabled =       (quint8)data->at(RPKT_ENABLE_FLAG + i*RPKT_MAX);
        pRelay->_bRemoteControl = (quint8)data->at(RPKT_REMOTE_CONTROL_FLAG + i*RPKT_MAX);
        pRelay->_bNotifyOnStateChanged = (quint8)data->at(RPKT_NOTIFY_ON_STATE_CHANGED_FLAG + i*RPKT_MAX);
        pRelay->_loadType =       (quint8)data->at(RPKT_REMOTE_CONTROL_FLAG + i*RPKT_MAX);

        qint16 groupId = (quint8)data->at(RPKT_GROUP_ID_H + i*RPKT_MAX) << 8 | (quint8)data->at(RPKT_GROUP_ID_L + i*RPKT_MAX);;

        int pType = (quint8)data->at(RPKT_PARENTDEV_TYPE + i*RPKT_MAX);
        int pUin =  (quint8)data->at(RPKT_PARENTDEV_UIN_H + i*RPKT_MAX) << 8 | data->at(RPKT_PARENTDEV_UIN_L + i*RPKT_MAX);
        int parentId = s_ParentUnit::getParentId(pUin, pType, db);

        qx_query q2("INSERT INTO t_Relay (id, parent_id, humanizedId, alias, suin, bEnabled, bRemoteControl, bNotifyOnStateChanged, load_type, group_id) "
                    "VALUES (:id, :parent_id, :humanizedId, :alias, :suin, :bEnabled, :bRemoteControl, :bNotifyOnStateChanged, :load_type, :group_id)");
        q2.bind(":id", static_cast<int>(pRelay->_id));
        q2.bind(":parent_id", parentId);
        q2.bind(":humanizedId", static_cast<int>(pRelay->_humanizedId));
        q2.bind(":alias", pRelay->_alias);
        q2.bind(":suin", static_cast<int>(pRelay->_suin));
        q2.bind(":bEnabled", pRelay->_bEnabled);
        q2.bind(":bRemoteControl", pRelay->_bRemoteControl);
        q2.bind(":bNotifyOnStateChanged", pRelay->_bNotifyOnStateChanged);
        q2.bind(":load_type", static_cast<int>(pRelay->_loadType));
        q2.bind(":group_id", DB_VALUE_NULL == groupId ? QVariant() : groupId);

        qx::dao::call_query(q2, db);
    }
}

QString createRelayAlias(QString pattern, t_RelayX *list)
{
    static QString temp;

    if (temp.isEmpty()) temp = pattern + "_";

    for (int i = 0; i < list->size(); ++i) {
        QString alias = list->getByIndex(i)->_alias;
        if (0 == alias.compare(pattern, Qt::CaseInsensitive)) {
            int sfx = alias.remove(temp).toInt() + 1;
            pattern = temp + QString::number(sfx);
            pattern = createRelayAlias(pattern, list);
            temp.clear();
            return pattern;
        }
    }

    temp.clear();
    return pattern;
}
