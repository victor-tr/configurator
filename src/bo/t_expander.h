#ifndef T_EXPANDER_H
#define T_EXPANDER_H

#include "commontrigger.h"


class t_Zone;
class s_ParentUnit;

class ARMOR_DLL_EXPORT t_Expander : public CommonTrigger
{
public:

    typedef qx::dao::ptr<t_Zone> t_Zone_ptr;
    typedef qx::QxCollection<long, t_Zone_ptr> t_ZoneX;
    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
    typedef qx::QxCollection<int, s_ParentUnit_ptr> s_ParentUnitX;

    t_Expander() : _id(-1)  {}
    virtual ~t_Expander() {}

    static QSharedPointer<QByteArray> fetchToByteArray(int expanderType);
    static void insertFromByteArray(int expanderType, QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    void appendZones(int amount);
    void removeZones();

    void appendRelays(int amount);
    void removeRelays();

    void appendLeds();
    void removeLeds();

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper *dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper *dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao);

// -- fields
    long  _id;
    long  _type;

// -- relations
//    t_ZoneX       _zones_list;
    s_ParentUnit_ptr _uin;

protected:

    virtual void makeAbstractClass() { ; }

};

//QX_REGISTER_PRIMARY_KEY(t_Expander, int)
ARMOR_QX_REGISTER_HPP(t_Expander, CommonTrigger, 0);

typedef qx::dao::ptr<t_Expander> t_Expander_ptr;
typedef qx::QxCollection<long, t_Expander_ptr> t_ExpanderX;
typedef qx::dao::ptr<t_ExpanderX> t_ExpanderX_ptr;

Q_DECLARE_METATYPE(t_Expander_ptr);


#endif // T_EXPANDER_H
