#ifndef T_REACTION_H
#define T_REACTION_H

class t_Event;
class t_BehaviorPreset;

class t_Reaction
{

    typedef qx::dao::ptr<t_BehaviorPreset> t_BehaviorPreset_ptr;
    typedef qx::QxCollection<int, t_BehaviorPreset_ptr> t_BehaviorPresetX;
    typedef qx::dao::ptr<t_Event> t_Event_ptr;

public:

    t_Reaction() : _id(-1) {}
    virtual ~t_Reaction()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    int        _id;
    QString    _alias;
    int        _performer_type;
    int        _performer_id;
    int        _performer_behavior;
    QByteArray _valid_states;
    bool       _bReversible;
    int        _reverse_performer_behavior;

    t_BehaviorPresetX  _behavior_preset_list;
    t_Event_ptr        _event;

};

QX_REGISTER_PRIMARY_KEY(t_Reaction, int);
ARMOR_QX_REGISTER_HPP(t_Reaction, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_Reaction> t_Reaction_ptr;
typedef qx::QxCollection<int, t_Reaction_ptr> t_ReactionX;
typedef qx::dao::ptr< t_ReactionX > t_ReactionX_ptr;

Q_DECLARE_METATYPE(t_Reaction_ptr);


#endif // T_REACTION_H
