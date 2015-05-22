#ifndef T_PHONE_H
#define T_PHONE_H


enum SimCard {
    SIM_1 = 1,
    SIM_2 = 2,

    SIM_MAX
};

class t_SimCard;

class ARMOR_DLL_EXPORT t_Phone
{
public:

    typedef qx::dao::ptr<t_SimCard> t_SimCard_ptr;

    t_Phone() : _id(-1) {}
    virtual ~t_Phone() {}

    static QSharedPointer<QByteArray> fetchToByteArray(SimCard number);
    static void insertFromByteArray(SimCard number, QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

    long _id;

    QString _phone;
    long    _idx;
    long    _allowed_simcard;

    t_SimCard_ptr _p_simcard;

};


ARMOR_QX_REGISTER_HPP(t_Phone, qx::trait::no_base_class_defined, 0);

typedef qx::dao::ptr<t_Phone> t_Phone_ptr;
typedef qx::QxCollection<long, t_Phone_ptr> t_PhoneX;
typedef qx::dao::ptr<t_PhoneX> t_PhoneX_ptr;


#endif // T_PHONE_H
