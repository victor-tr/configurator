#ifndef T_BUTTON_H
#define T_BUTTON_H

#include "commontrigger.h"


class s_ParentUnit;

class t_Button : public CommonTrigger
{

public:

    typedef qx::dao::ptr<s_ParentUnit> s_ParentUnit_ptr;

    t_Button() : _id(-1) {}
    virtual ~t_Button()  {}

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

    s_ParentUnit_ptr _pParent;

protected:

    virtual void makeAbstractClass() { ; }

};

QX_REGISTER_PRIMARY_KEY(t_Button, int);
ARMOR_QX_REGISTER_HPP(t_Button, CommonTrigger, 0);

typedef qx::dao::ptr<t_Button> t_Button_ptr;
typedef qx::QxCollection<int, t_Button_ptr> t_ButtonX;
typedef qx::dao::ptr<t_ButtonX> t_ButtonX_ptr;

Q_DECLARE_METATYPE(t_Button_ptr);


#endif // T_BUTTON_H
