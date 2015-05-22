#ifndef T_KEY_H
#define T_KEY_H


class t_ArmingGroup;

class ARMOR_DLL_EXPORT t_Key
{

public:

    typedef qx::dao::ptr<t_ArmingGroup> t_ArmingGroup_ptr;

    t_Key() : _id(-1) {}
    virtual ~t_Key()  {}

    static QSharedPointer<QByteArray> fetchToByteArray(int keyType);
    static void insertFromByteArray(int keyType, QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long        _id;
    long        _type;
    long        _action;
    QString     _alias;
    QByteArray  _value;

    t_ArmingGroup_ptr _arming_group;

};

ARMOR_QX_REGISTER_HPP(t_Key, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_Key> t_Key_ptr;
typedef qx::QxCollection<long, t_Key_ptr> t_KeyX;
typedef qx::dao::ptr< t_KeyX > t_KeyX_ptr;

Q_DECLARE_METATYPE(t_Key_ptr);


#endif // T_KEY_H
