#include "precompiled.h"
#include "t_button.h"
#include "s_parentunit.h"
#include "configurator_protocol.h"
#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Button)

namespace qx {

template<>
void register_class(QxClass<t_Button> &t)
{
    t.id(&t_Button::_id, "id");

    t.data(&t_Button::_humanizedId, "humanizedId");
    t.data(&t_Button::_alias, "alias");
    t.data(&t_Button::_suin, "suin");
    t.data(&t_Button::_bEnabled, "bEnabled");

    t.relationManyToOne(&t_Button::_pParent, "parent_id");
}

}


QSharedPointer<QByteArray> t_Button::fetchToByteArray()
{
    t_ButtonX list;

    QStringList relation;
    relation << "parent_id";
    qx::dao::fetch_all_with_relation(relation, list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(OBJ_BUTTON);
        ret->append(static_cast<char>(BTNPKT_MAX >> 8));
        ret->append(BTNPKT_MAX);

        t_Button_ptr item;
        _foreach (item, list) {
            QByteArray temp(BTNPKT_MAX, '\0');

            int id = item->_id;
            temp[ BTNPKT_ID_H ] = (id >> 8);
            temp[ BTNPKT_ID_L ] = id;
            temp[ BTNPKT_HUMANIZED_ID_H ] = (item->_humanizedId >> 8);
            temp[ BTNPKT_HUMANIZED_ID_L ] = item->_humanizedId;

            strncpy(temp.data()+BTNPKT_ALIAS_H,
                    GET_TEXT_CODEC->fromUnicode(item->_alias).constData(),
                    ALIAS_LEN);

            temp[ BTNPKT_SUIN ] = item->_suin;
            temp[ BTNPKT_ENABLE_FLAG ] = item->_bEnabled;

            if (!item->_pParent.isNull()) {
                temp[ BTNPKT_PARENTDEV_UIN_H ] = (item->_pParent->_puin >> 8);
                temp[ BTNPKT_PARENTDEV_UIN_L ] = item->_pParent->_puin;
                temp[ BTNPKT_PARENTDEV_TYPE ] = item->_pParent->_ptype;
            } else {
                temp[ BTNPKT_PARENTDEV_UIN_H ] = (DB_VALUE_NULL >> 8);
                temp[ BTNPKT_PARENTDEV_UIN_L ] = DB_VALUE_NULL;
                temp[ BTNPKT_PARENTDEV_TYPE ] = DB_VALUE_NULL;
            }

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Button::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / BTNPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_Button_ptr pButton(new t_Button);

        pButton->_id =          (quint8)data->at(BTNPKT_ID_H + i*BTNPKT_MAX) << 8 | (quint8)data->at(BTNPKT_ID_L + i*BTNPKT_MAX);
        pButton->_humanizedId = (quint8)data->at(BTNPKT_HUMANIZED_ID_H + i*BTNPKT_MAX) << 8 | (quint8)data->at(BTNPKT_HUMANIZED_ID_L + i*BTNPKT_MAX);
        pButton->_alias =       GET_TEXT_CODEC->toUnicode(data->mid(BTNPKT_ALIAS_H + i*BTNPKT_MAX, ALIAS_LEN).constData());
        pButton->_suin =        (quint8)data->at(BTNPKT_SUIN + i*BTNPKT_MAX);
        pButton->_bEnabled =    (quint8)data->at(BTNPKT_ENABLE_FLAG + i*BTNPKT_MAX);

        int pType = (quint8)data->at(BTNPKT_PARENTDEV_TYPE + i*BTNPKT_MAX);
        int pUin = (quint8)data->at(BTNPKT_PARENTDEV_UIN_H + i*BTNPKT_MAX) << 8 | (quint8)data->at(BTNPKT_PARENTDEV_UIN_L + i*BTNPKT_MAX);
        int parentId = s_ParentUnit::getParentId(pUin, pType, db);

        qx_query q2("INSERT INTO t_Button (id, parent_id, humanizedId, alias, suin, bEnabled) "
                    "VALUES (:id, :parent_id, :humanizedId, :alias, :suin, :bEnabled)");
        q2.bind(":id", pButton->_id);
        q2.bind(":parent_id", parentId);
        q2.bind(":humanizedId", pButton->_humanizedId);
        q2.bind(":alias", pButton->_alias);
        q2.bind(":suin", pButton->_suin);
        q2.bind(":bEnabled", pButton->_bEnabled);
        qx::dao::call_query(q2, db);
    }
}

void t_Button::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Button::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void t_Button::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
