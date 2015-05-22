#ifndef T_RELAY_H
#define T_RELAY_H

#include "commontrigger.h"


class s_ParentUnit;
class t_ArmingGroup;


class t_Relay : public CommonTrigger
{
public:

    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
    typedef qx::dao::ptr<t_ArmingGroup> t_ArmingGroup_ptr;

    enum RelayLoadType {
        RelayLoad_Common,
        RelayLoad_ArmingLed,
        RelayLoad_Bell
    };

    t_Relay() :
        _id(-1),
        _bEnabled(true),
        _bRemoteControl(false),
        _bNotifyOnStateChanged(false),
        _loadType(RelayLoad_Common)
    {}
    virtual ~t_Relay()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

// -- fields
    long _id;

    long    _humanizedId;
    QString _alias;
    long    _suin;
    bool    _bEnabled;
    bool    _bRemoteControl;
    bool    _bNotifyOnStateChanged;
    long    _loadType;

    s_ParentUnit_ptr  _pParent;
    t_ArmingGroup_ptr _pGroup;

protected:

    virtual void makeAbstractClass() { ; }

};


ARMOR_QX_REGISTER_HPP(t_Relay, CommonTrigger, 0);

typedef qx::dao::ptr<t_Relay> t_Relay_ptr;
typedef qx::QxCollection<long, t_Relay_ptr> t_RelayX;
typedef qx::dao::ptr<t_RelayX> t_RelayX_ptr;

QString createRelayAlias(QString pattern, t_RelayX *list);


Q_DECLARE_METATYPE(t_Relay_ptr);


#endif // T_RELAY_H
