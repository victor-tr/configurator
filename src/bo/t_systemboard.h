#ifndef T_SYSTEMBOARD_H
#define T_SYSTEMBOARD_H

#include "commontrigger.h"


class s_ParentUnit;

class t_SystemBoard : public CommonTrigger
{
public:
    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;
    typedef qx::QxCollection<int, s_ParentUnit_ptr> s_ParentUnitX;

    t_SystemBoard() : _id(-1) {}
    virtual ~t_SystemBoard()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper *dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper *dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao);

    long _id;

    s_ParentUnit_ptr _uin;

protected:

    virtual void makeAbstractClass() { ; }

};

ARMOR_QX_REGISTER_HPP(t_SystemBoard, CommonTrigger, 0);

typedef qx::dao::ptr<t_SystemBoard> t_SystemBoard_ptr;
typedef qx::QxCollection<long, t_SystemBoard_ptr> t_SystemBoardX;
typedef qx::dao::ptr< t_SystemBoardX > t_SystemBoardX_ptr;

Q_DECLARE_METATYPE(t_SystemBoard_ptr);


#endif // T_SYSTEMBOARD_H
