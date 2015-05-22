#ifndef T_ETR_H
#define T_ETR_H

#include "commontrigger.h"


class t_ArmingGroup;
class s_ParentUnit;

class ARMOR_DLL_EXPORT t_Etr : public CommonTrigger
{
public:

    typedef qx::dao::ptr<t_ArmingGroup> t_ArmingGroup_ptr;
    typedef qx::QxCollection<long, t_ArmingGroup_ptr> t_ArmingGroupX;
    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
    typedef qx::QxCollection<int, s_ParentUnit_ptr> s_ParentUnitX;

    enum EtrType {
        Etr_type_simple         = 1,
        Etr_type_with_4_zones
    };

    t_Etr() : _id(-1) {}
    virtual ~t_Etr()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    void appendZones(int amount);
    void removeZones();

    void appendLeds();
    void removeLeds();

    void appendButtons();
    void removeButtons();

    void appendLedIndicators();
    void removeLedIndicators();

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper *dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper *dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao);

    long    _id;
    long    _etr_type;

    t_ArmingGroupX   _groups_list;
    s_ParentUnit_ptr _uin;

protected:

    virtual void makeAbstractClass() { ; }
};

ARMOR_QX_REGISTER_HPP(t_Etr, CommonTrigger, 0);

typedef qx::dao::ptr<t_Etr> t_Etr_ptr;
typedef qx::QxCollection<long, t_Etr_ptr> t_EtrX;
typedef qx::dao::ptr< t_EtrX > t_EtrX_ptr;

Q_DECLARE_METATYPE(t_Etr_ptr);


#endif // T_ETR_H
