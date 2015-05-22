#ifndef T_LED_H
#define T_LED_H

#include "commontrigger.h"


class s_ParentUnit;
class t_ArmingGroup;


class t_Led : public CommonTrigger
{

public:

    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
    typedef qx::dao::ptr<t_ArmingGroup> t_ArmingGroup_ptr;

    t_Led() : _id(-1) {}
    virtual ~t_Led()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper * dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper * dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper * dao);

// -- fields
    int _id;

    int     _humanizedId;
    QString _alias;
    int     _suin;
    bool    _bEnabled;
    bool    _bArmingLed;

    s_ParentUnit_ptr  _pParent;
    t_ArmingGroup_ptr _pGroup;

protected:

    virtual void makeAbstractClass() { ; }

};

QX_REGISTER_PRIMARY_KEY(t_Led, int);
ARMOR_QX_REGISTER_HPP(t_Led, CommonTrigger, 0);

typedef qx::dao::ptr<t_Led> t_Led_ptr;
typedef qx::QxCollection<int, t_Led_ptr> t_LedX;
typedef qx::dao::ptr<t_LedX> t_LedX_ptr;

Q_DECLARE_METATYPE(t_Led_ptr);


#endif // T_LED_H
