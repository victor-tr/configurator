#include "precompiled.h"
#include "t_led.h"
#include "t_arminggroup.h"
#include "s_parentunit.h"
#include "configurator_protocol.h"
#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Led)

namespace qx {

template<>
void register_class(QxClass<t_Led> &t)
{
    t.id(&t_Led::_id, "id");

    t.data(&t_Led::_humanizedId, "humanizedId");
    t.data(&t_Led::_alias, "alias");
    t.data(&t_Led::_suin, "suin");
    t.data(&t_Led::_bEnabled, "bEnabled");
    t.data(&t_Led::_bArmingLed, "bArmingLed");

    t.relationManyToOne(&t_Led::_pParent, "parent_id");
    t.relationManyToOne(&t_Led::_pGroup, "group_id");
}

}


QSharedPointer<QByteArray> t_Led::fetchToByteArray()
{
    t_LedX list;

    QStringList relation;
    relation << "parent_id" << "group_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_LED);
        ret->append(static_cast<char>(LEDPKT_MAX >> 8));
        ret->append(LEDPKT_MAX);

        t_Led_ptr item;
        _foreach (item, list) {
            QByteArray temp(LEDPKT_MAX, '\0');

            int id = item->_id;
            temp[ LEDPKT_ID_H ] = (id >> 8);
            temp[ LEDPKT_ID_L ] = id;
            temp[ LEDPKT_HUMANIZED_ID_H ] = (item->_humanizedId >> 8);
            temp[ LEDPKT_HUMANIZED_ID_L ] = item->_humanizedId;

            strncpy(temp.data()+LEDPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ LEDPKT_SUIN ] = item->_suin;
            temp[ LEDPKT_ENABLE_FLAG ] = item->_bEnabled;

            if (!item->_pParent.isNull()) {
                temp[ LEDPKT_PARENTDEV_UIN_H ] = (item->_pParent->_puin >> 8);
                temp[ LEDPKT_PARENTDEV_UIN_L ] = item->_pParent->_puin;
                temp[ LEDPKT_PARENTDEV_TYPE ] = item->_pParent->_ptype;
            } else {
                temp[ LEDPKT_PARENTDEV_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ LEDPKT_PARENTDEV_UIN_L ] = DB_VALUE_NULL;
                temp[ LEDPKT_PARENTDEV_TYPE ] = DB_VALUE_NULL;
            }

            temp[LEDPKT_ARMING_LED_FLAG] = item->_bArmingLed;
            if (!item->_pGroup.isNull()) {
                temp[LEDPKT_GROUP_ID_H] = item->_pGroup->_id >> 8;
                temp[LEDPKT_GROUP_ID_L] = item->_pGroup->_id;
            } else {
                temp[LEDPKT_GROUP_ID_H] = DB_VALUE_NULL >> 8;
                temp[LEDPKT_GROUP_ID_L] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Led::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / LEDPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Led_ptr pLed(new t_Led);

        pLed->_id          = (quint8)data->at(LEDPKT_ID_H + i*LEDPKT_MAX) << 8 | (quint8)data->at(LEDPKT_ID_L + i*LEDPKT_MAX);
        pLed->_humanizedId = (quint8)data->at(LEDPKT_HUMANIZED_ID_H + i*LEDPKT_MAX) << 8 | (quint8)data->at(LEDPKT_HUMANIZED_ID_L + i*LEDPKT_MAX);
        pLed->_alias       = GET_TEXT_CODEC->toUnicode(data->mid(LEDPKT_ALIAS_H + i*LEDPKT_MAX, ALIAS_LEN).constData());
        pLed->_suin        = (quint8)data->at(LEDPKT_SUIN + i*LEDPKT_MAX);
        pLed->_bEnabled    = (quint8)data->at(LEDPKT_ENABLE_FLAG + i*LEDPKT_MAX);
        pLed->_bArmingLed  = (quint8)data->at(LEDPKT_ARMING_LED_FLAG + i*LEDPKT_MAX);

        int pType = (quint8)data->at(LEDPKT_PARENTDEV_TYPE + i*LEDPKT_MAX);
        int pUin = (quint8)data->at(LEDPKT_PARENTDEV_UIN_H + i*LEDPKT_MAX) << 8 | (quint8)data->at(LEDPKT_PARENTDEV_UIN_L + i*LEDPKT_MAX);
        int parentId = s_ParentUnit::getParentId(pUin, pType, db);

        qint16 groupId = (quint8)data->at(LEDPKT_GROUP_ID_H + i*LEDPKT_MAX) << 8 | (quint8)data->at(LEDPKT_GROUP_ID_L + i*LEDPKT_MAX);

        qx_query q1("INSERT INTO t_Led (id, parent_id, humanizedId, alias, suin, bEnabled, bArmingLed, group_id) "
                    "VALUES (:id, :parent_id, :humanizedId, :alias, :suin, :bEnabled, :bArmingLed, :group_id)");
        q1.bind(":id", pLed->_id);
        q1.bind(":parent_id", parentId);
        q1.bind(":humanizedId", pLed->_humanizedId);
        q1.bind(":alias", pLed->_alias);
        q1.bind(":suin", pLed->_suin);
        q1.bind(":bEnabled", pLed->_bEnabled);
        q1.bind(":bArmingLed", pLed->_bArmingLed);
        q1.bind(":group_id", DB_VALUE_NULL == groupId ? QVariant() : groupId);
        qx::dao::call_query(q1, db);
    }
}

void t_Led::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Led::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Led::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
