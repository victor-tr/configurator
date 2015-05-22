#include "precompiled.h"
#include "t_bell.h"
#include "s_parentunit.h"
#include "configurator_protocol.h"
#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Bell)

namespace qx {

template<>
void register_class(QxClass<t_Bell> &t)
{
    t.id(&t_Bell::_id, "id");

    t.data(&t_Bell::_humanizedId, "humanizedId");
    t.data(&t_Bell::_alias, "alias");
    t.data(&t_Bell::_suin, "suin");
    t.data(&t_Bell::_bEnabled, "bEnabled");
    t.data(&t_Bell::_bRemoteControl, "bRemoteControl");

    t.relationManyToOne(&t_Bell::_pParent, "parent_id");
}

}


QSharedPointer<QByteArray> t_Bell::fetchToByteArray()
{
    t_BellX list;

    QStringList relation;
    relation << "parent_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_BELL);
        ret->append(static_cast<char>(BELLPKT_MAX >> 8));
        ret->append(BELLPKT_MAX);

        t_Bell_ptr item;
        _foreach (item, list) {
            QByteArray temp(BELLPKT_MAX, '\0');

            int id = item->_id;
            temp[ BELLPKT_ID_H ] = (id >> 8);
            temp[ BELLPKT_ID_L ] = id;
            temp[ BELLPKT_HUMANIZED_ID_H ] = (item->_humanizedId >> 8);
            temp[ BELLPKT_HUMANIZED_ID_L ] = item->_humanizedId;

            strncpy(temp.data()+BELLPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ BELLPKT_SUIN ] = item->_suin;

            temp[ BELLPKT_ENABLE_FLAG ] = item->_bEnabled;
            temp[ BELLPKT_REMOTE_CONTROL_FLAG ] = item->_bRemoteControl;

            if (!item->_pParent.isNull()) {
                temp[ BELLPKT_PARENTDEV_UIN_H ] = (item->_pParent->_puin >> 8);
                temp[ BELLPKT_PARENTDEV_UIN_L ] = item->_pParent->_puin;
                temp[ BELLPKT_PARENTDEV_TYPE ] = item->_pParent->_ptype;
            } else {
                temp[ BELLPKT_PARENTDEV_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ BELLPKT_PARENTDEV_UIN_L ] = DB_VALUE_NULL;
                temp[ BELLPKT_PARENTDEV_TYPE ] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Bell::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / BELLPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Bell_ptr pBell(new t_Bell);

        pBell->_id =             (quint8)data->at(BELLPKT_ID_H + i*BELLPKT_MAX) << 8 | (quint8)data->at(BELLPKT_ID_L + i*BELLPKT_MAX);
        pBell->_humanizedId =    (quint8)data->at(BELLPKT_HUMANIZED_ID_H + i*BELLPKT_MAX) << 8 | (quint8)data->at(BELLPKT_HUMANIZED_ID_L + i*BELLPKT_MAX);
        pBell->_alias =          GET_TEXT_CODEC->toUnicode(data->mid(BELLPKT_ALIAS_H + i*BELLPKT_MAX, ALIAS_LEN).constData());
        pBell->_suin =           (quint8)data->at(BELLPKT_SUIN + i*BELLPKT_MAX);
        pBell->_bEnabled =       (quint8)data->at(BELLPKT_ENABLE_FLAG + i*BELLPKT_MAX);
        pBell->_bRemoteControl = (quint8)data->at(BELLPKT_REMOTE_CONTROL_FLAG + i*BELLPKT_MAX);

        int pType = (quint8)data->at(BELLPKT_PARENTDEV_TYPE + i*BELLPKT_MAX);
        int pUin = (quint8)data->at(BELLPKT_PARENTDEV_UIN_H + i*BELLPKT_MAX) << 8 | (quint8)data->at(BELLPKT_PARENTDEV_UIN_L + i*BELLPKT_MAX);
        int parentId = s_ParentUnit::getParentId(pUin, pType, db);

        qx_query q2("INSERT INTO t_Bell (id, parent_id, humanizedId, alias, suin, bEnabled, bRemoteControl) "
                    "VALUES (:id, :parent_id, :humanizedId, :alias, :suin, :bEnabled, :bRemoteControl)");
        q2.bind(":id", pBell->_id);
        q2.bind(":parent_id", parentId);
        q2.bind(":humanizedId", pBell->_humanizedId);
        q2.bind(":alias", pBell->_alias);
        q2.bind(":suin", pBell->_suin);
        q2.bind(":bEnabled", pBell->_bEnabled);
        q2.bind(":bRemoteControl", pBell->_bRemoteControl);
        qx::dao::call_query(q2, db);
    }
}

void t_Bell::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Bell::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Bell::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
