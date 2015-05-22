#ifndef S_PARENTUNIT_H
#define S_PARENTUNIT_H

#include "commontrigger.h"

#include "t_systemboard.h"
#include "t_expander.h"
#include "t_etr.h"

#include "t_zone.h"
#include "t_relay.h"
#include "t_led.h"
#include "t_bell.h"
#include "t_button.h"


class s_ParentUnit : public CommonTrigger
{
public:

    s_ParentUnit() : _id(-1) {}
    virtual ~s_ParentUnit()  {}

    static QString getParentDescription(int parentUnitID);
    static QString getParentDescription(const qx::dao::ptr<s_ParentUnit> &pParent);

    static int getParentId(int puin, int ptype, QSqlDatabase *db = 0);

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper * dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper * dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper * dao);

    int _id;

    int _ptype;
    int _puin;
    int _pid;
    QString _palias;

    t_SystemBoardX _systemboard_list;
    t_ExpanderX    _expander_list;
    t_EtrX         _etr_list;

    t_ZoneX   _zone_list;
    t_RelayX  _relay_list;
    t_LedX    _led_list;
    t_BellX   _bell_list;
    t_ButtonX _button_list;

protected:

    virtual void makeAbstractClass() { ; }

};

QX_REGISTER_PRIMARY_KEY(s_ParentUnit, int);
ARMOR_QX_REGISTER_HPP(s_ParentUnit, CommonTrigger, 0);

typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
typedef qx::QxCollection<int, s_ParentUnit_ptr> s_ParentUnitX;
typedef qx::dao::ptr<s_ParentUnitX> s_ParentUnitX_ptr;

Q_DECLARE_METATYPE(s_ParentUnit_ptr);


#endif // S_PARENTUNIT_H
