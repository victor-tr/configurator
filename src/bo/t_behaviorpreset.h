#ifndef T_BEHAVIORPRESET_H
#define T_BEHAVIORPRESET_H

class t_Reaction;

class t_BehaviorPreset
{

    typedef qx::dao::ptr<t_Reaction> t_Reaction_ptr;
    typedef qx::QxCollection<int, t_Reaction_ptr> t_ReactionX;

public:

    t_BehaviorPreset() : _id(-1) {}
    virtual ~t_BehaviorPreset()  {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    int     _id;
    QString _alias;
    int     _type;
    int     _pulses_in_batch;
    double  _pulse_len;
    double  _pause_len;
    double  _batch_pause_len;

    t_ReactionX _reactions_list;

};

QX_REGISTER_PRIMARY_KEY(t_BehaviorPreset, int);
ARMOR_QX_REGISTER_HPP(t_BehaviorPreset, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_BehaviorPreset> t_BehaviorPreset_ptr;
typedef qx::QxCollection<int, t_BehaviorPreset_ptr> t_BehaviorPresetX;
typedef qx::dao::ptr< t_BehaviorPresetX > t_BehaviorPresetX_ptr;

Q_DECLARE_METATYPE(t_BehaviorPreset_ptr);


#endif // T_BEHAVIORPRESET_H
