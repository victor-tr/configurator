#ifndef T_COMMONSETTINGS_H
#define T_COMMONSETTINGS_H


#define DTMF_TX_RATE_FAST   QT_TRANSLATE_NOOP("t_CommonSettings", "Fast")
#define DTMF_TX_RATE_MID    QT_TRANSLATE_NOOP("t_CommonSettings", "Middle")
#define DTMF_TX_RATE_SLOW   QT_TRANSLATE_NOOP("t_CommonSettings", "Slow")


class t_SimCard;
class t_AuxPhone;

class ARMOR_DLL_EXPORT t_CommonSettings
{
//    typedef qx::dao::ptr<t_PultMessageBuilder> t_PultMessageBuilder_ptr;

public:
    typedef qx::dao::ptr<t_SimCard> t_SimCard_ptr;
    typedef qx::QxCollection<long, t_SimCard_ptr> t_SimCardX;
    typedef qx::dao::ptr<t_AuxPhone> t_AuxPhone_ptr;
    typedef qx::QxCollection<long, t_AuxPhone_ptr> t_AuxPhoneX;

    t_CommonSettings() : _id(-1)  {}
    virtual ~t_CommonSettings() {}

    static QSharedPointer<QByteArray> fetchToByteArray();
    static void insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

// -- fields
    long _id;

//    QVector<bool> _checkboxes;

    long       _codecType;
    long       _entry_delay_sec;
    long       _arming_delay_sec;
    long       _debug_level;
    long       _aux_phonelist_default_size;

    QString    _dtmf_tx_rate;

    float     _adc_a;
    float     _adc_b;

    t_SimCardX  _simcards_list;
    t_AuxPhoneX _aux_phones_list;

};


ARMOR_QX_REGISTER_HPP(t_CommonSettings, qx::trait::no_base_class_defined, 0);

typedef boost::shared_ptr<t_CommonSettings> t_CommonSettings_ptr;
typedef qx::QxCollection<long, t_CommonSettings_ptr> t_CommonSettingsX;
typedef qx::dao::ptr<t_CommonSettingsX> t_CommonSettingsX_ptr;


#endif // T_COMMONSETTINGS_H
