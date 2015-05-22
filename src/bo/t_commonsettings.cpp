#include "precompiled.h"

#include "t_commonsettings.h"
#include "t_simcard.h"
#include "t_auxphone.h"
#include "configurator_protocol.h"
#include "mainwindow.h"

#include <QxMemLeak.h>


#define DTMF_PULSE_FAST     100
#define DTMF_PAUSE_FAST     80
#define DTMF_PULSE_MID      160
#define DTMF_PAUSE_MID      100
#define DTMF_PULSE_SLOW     220
#define DTMF_PAUSE_SLOW     140


ARMOR_QX_REGISTER_CPP(t_CommonSettings)

namespace qx {

template<>
void register_class(QxClass<t_CommonSettings> &t)
{
    t.id(&t_CommonSettings::_id, "id");

//    t.data(&t_CommonSettings::_checkboxes, "checkboxes");

    t.data(&t_CommonSettings::_codecType, "codec_type");
    t.data(&t_CommonSettings::_entry_delay_sec, "entry_delay_sec");
    t.data(&t_CommonSettings::_arming_delay_sec, "arming_delay_sec");
    t.data(&t_CommonSettings::_debug_level, "debug_level");
    t.data(&t_CommonSettings::_aux_phonelist_default_size, "aux_phonelist_default_size");
    t.data(&t_CommonSettings::_dtmf_tx_rate, "dtmf_tx_rate");
    t.data(&t_CommonSettings::_adc_a, "adc_a");
    t.data(&t_CommonSettings::_adc_b, "adc_b");

    t.relationOneToMany(&t_CommonSettings::_simcards_list, "simcards_list", "common_settings_id");
    t.relationOneToMany(&t_CommonSettings::_aux_phones_list, "aux_phones_list", "common_settings_id");
}

}


QSharedPointer<QByteArray> t_CommonSettings::fetchToByteArray()
{
    t_CommonSettingsX_ptr list(new t_CommonSettingsX());

    qx::dao::fetch_all(list);

    QByteArray *ret = list->size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_COMMON_SETTINGS));
        ret->append(static_cast<char>(CSPKT_MAX >> 8));  // struct size H-byte
        ret->append(CSPKT_MAX);         // struct size L-byte

        t_CommonSettings_ptr item;
        _foreach (item, *list.data()) {
            QByteArray temp(CSPKT_MAX, '\0');

            // pack bits to bytes (MS Bit placed at MAX index pos of the vector)
//            int checkboxes_bytes_qty = CSPKT_CHECKBOXES_H - CSPKT_CHECKBOXES_L + 1;
//            for (int j = checkboxes_bytes_qty - 1; j >= 0; --j) {
//                quint8 byte = 0;
//                for (int i = 7; i >= 0; --i) {
//                    if (j * 8 + i < item->_checkboxes.size())
//                        byte |= item->_checkboxes.at(j * 8 + i) ? 1 : 0;
//                    byte <<= 1;
//                }
//                temp[CSPKT_CHECKBOXES_H + j] = byte;
//            }

            temp[CSPKT_CODEC_TYPE] = item->_codecType;
            temp[CSPKT_ENTRY_DELAY_H] = (item->_entry_delay_sec >> 8);
            temp[CSPKT_ENTRY_DELAY_L] = item->_entry_delay_sec;
            temp[CSPKT_ARMING_DELAY_H] = (item->_arming_delay_sec >> 8);
            temp[CSPKT_ARMING_DELAY_L] = item->_arming_delay_sec;
            temp[CSPKT_DEBUG_LEVEL] = item->_debug_level;
            temp[CSPKT_AUX_PHONELIST_DEFAULT_SIZE] = item->_aux_phonelist_default_size;


            // -- DTMF Tx rate
            quint16 dtmf_pulse_ms = 0;
            quint16 dtmf_pause_ms = 0;

            if (DTMF_TX_RATE_FAST == item->_dtmf_tx_rate) {
                dtmf_pulse_ms = DTMF_PULSE_FAST;
                dtmf_pause_ms = DTMF_PAUSE_FAST;
            }
            else if (DTMF_TX_RATE_MID == item->_dtmf_tx_rate) {
                dtmf_pulse_ms = DTMF_PULSE_MID;
                dtmf_pause_ms = DTMF_PAUSE_MID;
            }
            else if (DTMF_TX_RATE_SLOW == item->_dtmf_tx_rate) {
                dtmf_pulse_ms = DTMF_PULSE_SLOW;
                dtmf_pause_ms = DTMF_PAUSE_SLOW;
            }

            temp[CSPKT_DTMF_PULSE_MS_H] = dtmf_pulse_ms >> 8;
            temp[CSPKT_DTMF_PULSE_MS_L] = dtmf_pulse_ms;
            temp[CSPKT_DTMF_PAUSE_MS_H] = dtmf_pause_ms >> 8;
            temp[CSPKT_DTMF_PAUSE_MS_L] = dtmf_pause_ms;

            int mantissa = static_cast<int>(item->_adc_a);
            int decimal = static_cast<int>((item->_adc_a - mantissa) * CSPKT_ADC_Precision);
            temp[CSPKT_ADC_A_MANTISSA_H] = mantissa >> 8;
            temp[CSPKT_ADC_A_MANTISSA_L] = mantissa;
            temp[CSPKT_ADC_A_DECIMAL_H] = decimal >> 8;
            temp[CSPKT_ADC_A_DECIMAL_L] = decimal;
            mantissa = static_cast<int>(item->_adc_b);
            decimal = static_cast<int>((item->_adc_b - mantissa) * CSPKT_ADC_Precision);
            temp[CSPKT_ADC_B_MANTISSA_H] = mantissa >> 8;
            temp[CSPKT_ADC_B_MANTISSA_L] = mantissa;
            temp[CSPKT_ADC_B_DECIMAL_H] = decimal >> 8;
            temp[CSPKT_ADC_B_DECIMAL_L] = decimal;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_CommonSettings::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / CSPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_CommonSettings_ptr pSettings(new t_CommonSettings);
        pSettings->_id = 1;
        pSettings->_codecType = (quint8)data->at(CSPKT_CODEC_TYPE + i*CSPKT_MAX);
        pSettings->_entry_delay_sec = (quint8)data->at(CSPKT_ENTRY_DELAY_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ENTRY_DELAY_L + i*CSPKT_MAX);
        pSettings->_arming_delay_sec = (quint8)data->at(CSPKT_ARMING_DELAY_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ARMING_DELAY_L + i*CSPKT_MAX);
        pSettings->_debug_level = (quint8)data->at(CSPKT_DEBUG_LEVEL + i*CSPKT_MAX);
        pSettings->_aux_phonelist_default_size = (quint8)data->at(CSPKT_AUX_PHONELIST_DEFAULT_SIZE + i*CSPKT_MAX);

        int mantissa = (quint8)data->at(CSPKT_ADC_A_MANTISSA_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ADC_A_MANTISSA_L + i*CSPKT_MAX);
        int decimal = (quint8)data->at(CSPKT_ADC_A_DECIMAL_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ADC_A_DECIMAL_L + i*CSPKT_MAX);
        pSettings->_adc_a = mantissa + (float)decimal / CSPKT_ADC_Precision;

        mantissa = (quint8)data->at(CSPKT_ADC_B_MANTISSA_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ADC_B_MANTISSA_L + i*CSPKT_MAX);
        decimal = (quint8)data->at(CSPKT_ADC_B_DECIMAL_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_ADC_B_DECIMAL_L + i*CSPKT_MAX);
        pSettings->_adc_b = mantissa + (float)decimal / CSPKT_ADC_Precision;

        int dtmf_pulse = (quint8)data->at(CSPKT_DTMF_PULSE_MS_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_DTMF_PULSE_MS_L + i*CSPKT_MAX);
//        int dtmf_pause = (quint8)data->at(CSPKT_DTMF_PAUSE_MS_H + i*CSPKT_MAX) << 8 | (quint8)data->at(CSPKT_DTMF_PAUSE_MS_L + i*CSPKT_MAX);
        QString dtmf_tx_rate_text;
        if (DTMF_PULSE_FAST == dtmf_pulse)
            dtmf_tx_rate_text = DTMF_TX_RATE_FAST;
        else if (DTMF_PULSE_MID == dtmf_pulse)
            dtmf_tx_rate_text = DTMF_TX_RATE_MID;
        else
            dtmf_tx_rate_text = DTMF_TX_RATE_SLOW;
        pSettings->_dtmf_tx_rate = dtmf_tx_rate_text;

        qx_query q1("INSERT INTO t_CommonSettings (id, checkboxes, codec_type, entry_delay_sec, "
                    "arming_delay_sec, debug_level, aux_phonelist_default_size, dtmf_tx_rate, "
                    "adc_a, adc_b) "
                    "VALUES (:id, :checkboxes, :codec_type, :entry_delay_sec, "
                    ":arming_delay_sec, :debug_level, :aux_phonelist_default_size, :dtmf_tx_rate, "
                    ":adc_a, :adc_b)");
        q1.bind(":id", static_cast<int>(pSettings->_id));
        q1.bind(":checkboxes", QVariant::fromValue(QVector<bool>()));
        q1.bind(":codec_type", static_cast<int>(pSettings->_codecType));
        q1.bind(":entry_delay_sec", static_cast<int>(pSettings->_entry_delay_sec));
        q1.bind(":arming_delay_sec", static_cast<int>(pSettings->_arming_delay_sec));
        q1.bind(":debug_level", static_cast<int>(pSettings->_debug_level));
        q1.bind(":aux_phonelist_default_size", static_cast<int>(pSettings->_aux_phonelist_default_size));
        q1.bind(":dtmf_tx_rate", pSettings->_dtmf_tx_rate);
        q1.bind(":adc_a", pSettings->_adc_a);
        q1.bind(":adc_b", pSettings->_adc_b);
        qx::dao::call_query(q1, db);
    }
}
