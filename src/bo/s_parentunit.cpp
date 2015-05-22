#include "precompiled.h"
#include "s_parentunit.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(s_ParentUnit)

namespace qx {

template <>
void register_class(QxClass<s_ParentUnit> &t)
{
    t.setName("s_Parent");

    t.id(&s_ParentUnit::_id, "id");

    t.data(&s_ParentUnit::_ptype, "ptype");
    t.data(&s_ParentUnit::_puin, "puin");
    t.data(&s_ParentUnit::_pid, "pid");
    t.data(&s_ParentUnit::_palias, "palias");

    t.relationOneToMany(&s_ParentUnit::_systemboard_list, "systemboard_list", "uin_id");
    t.relationOneToMany(&s_ParentUnit::_expander_list,    "expander_list",    "uin_id");
    t.relationOneToMany(&s_ParentUnit::_etr_list,         "etr_list",         "uin_id");

    t.relationOneToMany(&s_ParentUnit::_zone_list,   "zone_list",   "parent_id");
    t.relationOneToMany(&s_ParentUnit::_relay_list,  "relay_list",  "parent_id");
    t.relationOneToMany(&s_ParentUnit::_led_list,    "led_list",    "parent_id");
    t.relationOneToMany(&s_ParentUnit::_bell_list,   "bell_list",   "parent_id");
    t.relationOneToMany(&s_ParentUnit::_button_list, "button_list", "parent_id");
}

}


QString s_ParentUnit::getParentDescription(int parentUnitID)
{
    QString desc("%3 &lt;<b>%1</b>&gt; %2");

    s_ParentUnit_ptr pParent(new s_ParentUnit());
    pParent->_id = parentUnitID;
    pParent->_ptype = OBJ_MAX;

    qx::dao::fetch_by_id(pParent);

    QString puin = QString("(UIN: %1)").arg(pParent->_puin);
    desc = desc.arg(pParent->_palias).arg(OBJ_MASTERBOARD != pParent->_ptype ? puin : "");

    switch (pParent->_ptype) {
    case OBJ_MASTERBOARD:
        desc = desc.arg(QObject::tr("System Board"));
        break;

    case OBJ_RAE:
        desc = desc.arg(QObject::tr("Relays Expander"));
        break;

    case OBJ_RZE:
        desc = desc.arg(QObject::tr("Zones Expander"));
        break;

    case OBJ_ETR:
        desc = desc.arg(QObject::tr("Touch Memory Reader"));
        break;

    default:
        return QString();
    }

    return desc;
}

QString s_ParentUnit::getParentDescription(const qx::dao::ptr<s_ParentUnit> &pParent)
{
    QString desc("%3 &lt;<b>%1</b>&gt; %2");

    QString puin = QString("(UIN: %1)").arg(pParent->_puin);
    desc = desc.arg(pParent->_palias).arg(OBJ_MASTERBOARD != pParent->_ptype ? puin : "");

    switch (pParent->_ptype) {
    case OBJ_MASTERBOARD:
        desc = desc.arg(QObject::tr("System Board"));
        break;

    case OBJ_RAE:
        desc = desc.arg(QObject::tr("Relays Expander"));
        break;

    case OBJ_RZE:
        desc = desc.arg(QObject::tr("Zones Expander"));
        break;

    case OBJ_ETR:
        desc = desc.arg(QObject::tr("Touch Memory Reader"));
        break;

    default:
        return QString();
    }

    return desc;
}

int s_ParentUnit::getParentId(int puin, int ptype, QSqlDatabase *db)
{
    qx_query q1("SELECT id FROM s_Parent WHERE ptype = :ptype AND puin = :puin");
    q1.bind(":ptype", ptype);
    q1.bind(":puin", puin);
    qx::dao::call_query(q1, db);
    return q1.getSqlResultAt(0, 0).toInt();
}


void s_ParentUnit::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void s_ParentUnit::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void s_ParentUnit::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

