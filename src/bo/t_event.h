#ifndef T_EVENT_H
#define T_EVENT_H

class t_Reaction;

class t_Event
{

    typedef qx::dao::ptr<t_Reaction> t_Reaction_ptr;
    typedef qx::QxCollection<int, t_Reaction_ptr> t_ReactionX;

public:

    t_Event() : _id(-1) {}
    virtual ~t_Event()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    int     _id;
    QString _alias;
    int     _event;
    int     _emitter_type;
    int     _emitter_id;

    t_ReactionX _reactions_list;

};

QX_REGISTER_PRIMARY_KEY(t_Event, int);
ARMOR_QX_REGISTER_HPP(t_Event, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_Event> t_Event_ptr;
typedef qx::QxCollection<int, t_Event_ptr> t_EventX;
typedef qx::dao::ptr< t_EventX > t_EventX_ptr;

Q_DECLARE_METATYPE(t_Event_ptr);


#endif // T_EVENT_H
