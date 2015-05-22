#ifndef T_ARMINGGROUP_H
#define T_ARMINGGROUP_H

#include "commontrigger.h"


class t_Zone;
class t_Key;
class t_Etr;
class t_Led;
class t_Relay;


class ARMOR_DLL_EXPORT t_ArmingGroup : public CommonTrigger
{
    typedef qx::dao::ptr<t_Zone> t_Zone_ptr;
    typedef qx::QxCollection<long, t_Zone_ptr> t_ZoneX;
    typedef qx::dao::ptr<t_Key> t_Key_ptr;
    typedef qx::QxCollection<long, t_Key_ptr> t_KeyX;
    typedef qx::dao::ptr<t_Etr> t_Etr_ptr;
    typedef qx::QxCollection<long, t_Etr_ptr> t_EtrX;
    typedef qx::dao::ptr<t_Led> t_Led_ptr;
    typedef qx::QxCollection<int, t_Led_ptr> t_LedX;
    typedef qx::dao::ptr<t_Relay> t_Relay_ptr;
    typedef qx::QxCollection<long, t_Relay_ptr> t_RelayX;

public:

    t_ArmingGroup() : _id(-1) {}
    virtual ~t_ArmingGroup() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

// -- fields
    long       _id;
    QString    _alias;

// -- relations
    t_ZoneX  _zones_list;
    t_KeyX   _keys_list;
    t_EtrX   _etr_list;
    t_LedX   _led_list;
    t_RelayX _relay_list;

protected:

    virtual void makeAbstractClass() { ; }

};

ARMOR_QX_REGISTER_HPP(t_ArmingGroup, CommonTrigger, 0);

typedef qx::dao::ptr<t_ArmingGroup> t_ArmingGroup_ptr;
typedef qx::QxCollection<long, t_ArmingGroup_ptr> t_ArmingGroupX;
typedef qx::dao::ptr< t_ArmingGroupX > t_ArmingGroupX_ptr;

Q_DECLARE_METATYPE(t_ArmingGroup_ptr);


#endif // T_ARMINGGROUP_H
