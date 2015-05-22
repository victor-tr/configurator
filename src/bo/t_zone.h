#ifndef T_ZONE_H
#define T_ZONE_H


#include "t_arminggroup.h"


class s_ParentUnit;

class ARMOR_DLL_EXPORT t_Zone : public CommonTrigger
{
public:

    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;

    t_Zone() : _id(-1) {}
    virtual ~t_Zone() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

// -- fields
    long _id;

    long    _humanizedId;
    QString _alias;
    long    _suin;
    bool    _bEnabled;
    bool    _bDamageNotificationEnabled;
    long    _zoneType;

// -- relations
    t_ArmingGroup_ptr _arming_group;
    s_ParentUnit_ptr  _pParent;

protected:

    virtual void makeAbstractClass() { ; }

};


ARMOR_QX_REGISTER_HPP(t_Zone, CommonTrigger, 0);

typedef qx::dao::ptr<t_Zone> t_Zone_ptr;
typedef qx::QxCollection<long, t_Zone_ptr> t_ZoneX;
typedef qx::dao::ptr<t_ZoneX> t_ZoneX_ptr;

QString createZoneAlias(QString pattern, t_ZoneX *list);

Q_DECLARE_METATYPE(t_Zone_ptr);


#endif // T_ZONE_H
