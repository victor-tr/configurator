#ifndef T_BELL_H
#define T_BELL_H

#include "commontrigger.h"


class s_ParentUnit;

class t_Bell : public CommonTrigger
{

public:

    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;

    t_Bell() : _id(-1) {}
    virtual ~t_Bell()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper * dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper * dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper * dao);

// -- fields
    int  _id;

    int     _humanizedId;
    QString _alias;
    int     _suin;
    bool    _bEnabled;
    bool    _bRemoteControl;

    s_ParentUnit_ptr _pParent;

protected:

    virtual void makeAbstractClass() { ; }

};

QX_REGISTER_PRIMARY_KEY(t_Bell, int);
ARMOR_QX_REGISTER_HPP(t_Bell, CommonTrigger, 0);

typedef qx::dao::ptr<t_Bell> t_Bell_ptr;
typedef qx::QxCollection<int, t_Bell_ptr> t_BellX;
typedef qx::dao::ptr<t_BellX> t_BellX_ptr;

Q_DECLARE_METATYPE(t_Bell_ptr);


#endif // T_BELL_H
