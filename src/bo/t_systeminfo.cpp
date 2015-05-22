#include "precompiled.h"
#include "t_systeminfo.h"
#include "configurator_protocol.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_SystemInfo)

namespace qx {

template <> void register_class(QxClass<t_SystemInfo> &t)
{
        t.id(&t_SystemInfo::_id, "id");

        t.data(&t_SystemInfo::_settings_ident_string, "settings_ident_string");
        t.data(&t_SystemInfo::_settings_signature, "settings_signature");
}

} // namespace qx


QSharedPointer<QByteArray> t_SystemInfo::fetchToByteArray()
{
    t_SystemInfoX list;

    qx::dao::fetch_all(list);

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_SYSTEM_INFO));
        ret->append(static_cast<char>(SINFPKT_MAX >> 8));  // struct size H-byte
        ret->append(SINFPKT_MAX);         // struct size L-byte

        t_SystemInfo_ptr item;
        _foreach (item, list) {
            QByteArray temp(SINFPKT_MAX, '\0');

            temp[SINFPKT_SETTINGS_SIGNATURE_0] = item->_settings_signature;
            temp[SINFPKT_SETTINGS_SIGNATURE_1] = item->_settings_signature >> 8;
            temp[SINFPKT_SETTINGS_SIGNATURE_2] = item->_settings_signature >> 16;
            temp[SINFPKT_SETTINGS_SIGNATURE_3] = item->_settings_signature >> 24;

            strncpy(temp.data()+SINFPKT_SETTINGS_IDENT_STR_H,
                    GET_TEXT_CODEC->fromUnicode(item->_settings_ident_string).constData(),
                    SYSTEM_INFO_STRINGS_LEN);

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_SystemInfo::insertFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / SINFPKT_MAX;

    for (int i = 0; i < size; ++i) {
        t_SystemInfo_ptr pSystemInfo(new t_SystemInfo);
        pSystemInfo->_id = 1;
        pSystemInfo->_settings_signature = (quint8)data->at(SINFPKT_SETTINGS_SIGNATURE_3 + i*SINFPKT_MAX) << 24 |
                                           (quint8)data->at(SINFPKT_SETTINGS_SIGNATURE_2 + i*SINFPKT_MAX) << 16 |
                                           (quint8)data->at(SINFPKT_SETTINGS_SIGNATURE_1 + i*SINFPKT_MAX) << 8 |
                                           (quint8)data->at(SINFPKT_SETTINGS_SIGNATURE_0 + i*SINFPKT_MAX);
        pSystemInfo->_settings_ident_string = GET_TEXT_CODEC->toUnicode(
                    data->mid(SINFPKT_SETTINGS_IDENT_STR_H + i*SINFPKT_MAX, SYSTEM_INFO_STRINGS_LEN).constData());

        qx_query q1("INSERT INTO t_SystemInfo (id, settings_ident_string, settings_signature) "
                    "VALUES (:id, :settings_ident_string, :settings_signature)");
        q1.bind(":id", static_cast<int>(pSystemInfo->_id));
        q1.bind(":settings_ident_string", pSystemInfo->_settings_ident_string);
        q1.bind(":settings_signature", static_cast<int>(pSystemInfo->_settings_signature));
        qx::dao::call_query(q1, db);
    }
}
