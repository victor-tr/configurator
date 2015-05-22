#include "precompiled.h"

#include "t_systemboard.h"
#include "t_zone.h"
#include "t_relay.h"
#include "t_led.h"
#include "t_bell.h"

#include "s_parentunit.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_SystemBoard)

namespace qx {

template<>
void register_class(QxClass<t_SystemBoard> &t)
{
    t.id(&t_SystemBoard::_id, "id");

    t.relationManyToOne(&t_SystemBoard::_uin, "uin_id");
}

}   // namespace qx


QSharedPointer<QByteArray> t_SystemBoard::fetchToByteArray()
{
    t_SystemBoardX list;

    QStringList relation;
    relation << "uin_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_MASTERBOARD);
        ret->append(static_cast<char>(MBPKT_MAX >> 8));
        ret->append(MBPKT_MAX);

        t_SystemBoard_ptr item;
        _foreach (item, list) {
            QByteArray temp(MBPKT_MAX, '\0');

            long id = item->_id;
            temp[ MBPKT_ID_H ] = (id >> 8);
            temp[ MBPKT_ID_L ] = id;

            if (!item->_uin.isNull()) {
                temp[ MBPKT_PARENT_ID_H ] = item->_uin->_id >> 8;
                temp[ MBPKT_PARENT_ID_L ] = item->_uin->_id;
                temp[ MBPKT_UIN_H ] = (item->_uin->_puin >> 8);
                temp[ MBPKT_UIN_L ] = item->_uin->_puin;

                strncpy(temp.data()+MBPKT_ALIAS_H,
                        GET_TEXT_CODEC->fromUnicode(item->_uin->_palias).constData(),
                        ALIAS_LEN);
            } else {
                temp[ MBPKT_PARENT_ID_H ] = DB_VALUE_NULL >> 8;
                temp[ MBPKT_PARENT_ID_L ] = DB_VALUE_NULL;
                temp[ MBPKT_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ MBPKT_UIN_L ] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_SystemBoard::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / MBPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_SystemBoard_ptr pMasterBoard(new t_SystemBoard);

        pMasterBoard->_id = 1;

        pMasterBoard->_uin = s_ParentUnit_ptr(new s_ParentUnit);
        pMasterBoard->_uin->_id =     (quint8)data->at(MBPKT_PARENT_ID_H + i*MBPKT_MAX) << 8 | (quint8)data->at(MBPKT_PARENT_ID_L + i*MBPKT_MAX);
        pMasterBoard->_uin->_ptype =  OBJ_MASTERBOARD;
        pMasterBoard->_uin->_puin =   (quint8)data->at(MBPKT_UIN_H + i*MBPKT_MAX) << 8 | (quint8)data->at(MBPKT_UIN_L + i*MBPKT_MAX);
        pMasterBoard->_uin->_pid =    pMasterBoard->_id;
        pMasterBoard->_uin->_palias = GET_TEXT_CODEC->toUnicode(data->mid(MBPKT_ALIAS_H + i*MBPKT_MAX, ALIAS_LEN).constData());


        qx_query q1("INSERT INTO t_SystemBoard (id, uin_id) VALUES (:id, :uin_id)");
        q1.bind(":id", static_cast<int>(pMasterBoard->_id));
        q1.bind(":uin_id", pMasterBoard->_uin->_id);
        qx::dao::call_query(q1, db);

        qx_query q2("INSERT INTO s_Parent (id, ptype, pid, puin, palias) VALUES (:id, :ptype, :pid, :puin, :palias)");
        q2.bind(":id", pMasterBoard->_uin->_id);
        q2.bind(":ptype", pMasterBoard->_uin->_ptype);
        q2.bind(":pid", pMasterBoard->_uin->_pid);
        q2.bind(":puin", pMasterBoard->_uin->_puin);
        q2.bind(":palias", pMasterBoard->_uin->_palias);
        qx::dao::call_query(q2, db);
    }
}

void t_SystemBoard::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_SystemBoard::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_SystemBoard::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
