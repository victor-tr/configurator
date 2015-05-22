#include "precompiled.h"

#include "t_key.h"
#include "t_arminggroup.h"

#include "configurator_protocol.h"

#include <algorithm>

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(t_Key)

namespace qx {

template<>
void register_class(QxClass<t_Key> &t)
{
    t.id(&t_Key::_id, "id");
    t.data(&t_Key::_type, "type");
    t.data(&t_Key::_action, "action");
    t.data(&t_Key::_alias, "alias");
    t.data(&t_Key::_value, "value");

    t.relationManyToOne(&t_Key::_arming_group, "arming_group_id");
}

}   // namespace qx


/*
 *  evaluate CRC8 checksum for Dallas iButton
 *  x^8 + x^5 + x^4 + 1
 */
quint8 evaluateTouchMemoryCRC(const QByteArray &data)
{
    quint8 crc = 0x00;
    quint8 len = 7;     // iButton key size
    const char *d = data.data();

    while (len--) {
        crc ^= *d++;
        for (quint8 i = 0; i < 8; i++)
            crc = crc & 0x01 ? (crc >> 1) ^ 0x8C : crc >> 1;
    }

    return crc;
}

QSharedPointer<QByteArray> t_Key::fetchToByteArray(int keyType)
{
    Q_ASSERT(OBJ_KEYBOARD_CODE == keyType || OBJ_TOUCHMEMORY_CODE == keyType);

    t_KeyX list;

    QStringList relation;
    relation << "arming_group_id";
    qx::dao::fetch_by_query_with_relation(relation, qx_query().where("type").isEqualTo(keyType), list);

    QTextCodec *codec = GET_TEXT_CODEC;

    QByteArray *ret = list.size() ? new QByteArray() : NULL;
    if (ret) {
        if (OBJ_KEYBOARD_CODE == keyType)
        {
            ret->append(static_cast<char>(OBJ_KEYBOARD_CODE));
            ret->append(static_cast<char>(KBDCPKT_MAX >> 8));
            ret->append(KBDCPKT_MAX);

            t_Key_ptr item;
            _foreach (item, list) {
                QByteArray temp(KBDCPKT_MAX, '\0');

                long id = item->_id;
                temp[ KBDCPKT_ID_H ] = (id >> 8);
                temp[ KBDCPKT_ID_L ] = id;
                temp[ KBDCPKT_KEY_SIZE ] = item->_value.size();
                temp[ KBDCPKT_ACTION ] = item->_action;

                long ag_id = item->_arming_group.isNull() ? DB_VALUE_NULL : item->_arming_group->_id;
                temp[ KBDCPKT_ARMING_GROUP_ID_H ] = (ag_id >> 8);
                temp[ KBDCPKT_ARMING_GROUP_ID_L ] = ag_id;

                for (int i = 0; i < KEYBOARD_MAX_KEY_SIZE; ++i) {
                    if (i < item->_value.size())
                        temp[KBDCPKT_KEY_H + i] = item->_value.at(i);
                }

                strncpy(temp.data()+KBDCPKT_ALIAS_H,
                        codec->fromUnicode(item->_alias).constData(),
                        ALIAS_LEN);

                ret->append(temp);
            }
        }
        else if (OBJ_TOUCHMEMORY_CODE == keyType)
        {
            ret->append(static_cast<char>(OBJ_TOUCHMEMORY_CODE));
            ret->append(static_cast<char>(TMCPKT_MAX >> 8));
            ret->append(TMCPKT_MAX);

            t_Key_ptr item;
            _foreach (item, list) {
                QByteArray temp(TMCPKT_MAX, '\0');

                long id = item->_id;
                temp[ TMCPKT_ID_H ] = (id >> 8);
                temp[ TMCPKT_ID_L ] = id;
                temp[ TMCPKT_ACTION ] = item->_action;

                long ag_id = item->_arming_group.isNull() ? DB_VALUE_NULL : item->_arming_group->_id;
                temp[ TMCPKT_ARMING_GROUP_ID_H ] = (ag_id >> 8);
                temp[ TMCPKT_ARMING_GROUP_ID_L ] = ag_id;

                QByteArray reversed;
                reversed.resize(item->_value.size());
                std::reverse_copy(item->_value.constBegin(),
                                  item->_value.constEnd(),
                                  reversed.begin());
                reversed.prepend(1);
                reversed.append(evaluateTouchMemoryCRC(reversed));

//qDebug() << "original:" << item->_value.toHex().toUpper();
//qDebug() << "reversed:" << reversed.toHex().toUpper();

                for (int i = 0; i < TOUCHMEMORY_MAX_KEY_SIZE; ++i) {
                    if (i < reversed.size())
                        temp[TMCPKT_KEY_H + i] = reversed.at(i);
                }

                temp[ TMCPKT_KEY_SIZE ] = reversed.size();

                strncpy(temp.data()+TMCPKT_ALIAS_H,
                        codec->fromUnicode(item->_alias).constData(),
                        ALIAS_LEN);

                ret->append(temp);
            }
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void t_Key::insertFromByteArray(int keyType, QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    Q_ASSERT(OBJ_KEYBOARD_CODE == keyType || OBJ_TOUCHMEMORY_CODE == keyType);

    int size = 0;
    qint16 groupId = 0;

    QTextCodec *codec = GET_TEXT_CODEC;

    if (OBJ_KEYBOARD_CODE == keyType) {
        size = data->size() / KBDCPKT_MAX;
        for (int i = 0; i < size; ++i) {
            t_Key_ptr pKey(new t_Key);
            pKey->_id =     (quint8)data->at(KBDCPKT_ID_H + i*KBDCPKT_MAX) << 8 | (quint8)data->at(KBDCPKT_ID_L + i*KBDCPKT_MAX);
            pKey->_type =   keyType;
            pKey->_action = (quint8)data->at(KBDCPKT_ACTION + i*KBDCPKT_MAX);
            pKey->_alias =  codec->toUnicode(data->mid(KBDCPKT_ALIAS_H + i*KBDCPKT_MAX, ALIAS_LEN).constData());
            pKey->_value =  data->mid(KBDCPKT_KEY_H + i*KBDCPKT_MAX, KEYBOARD_MAX_KEY_SIZE);

            groupId = (quint8)data->at(KBDCPKT_ARMING_GROUP_ID_H + i*KBDCPKT_MAX) << 8 |
                          (quint8)data->at(KBDCPKT_ARMING_GROUP_ID_L + i*KBDCPKT_MAX);

            qx_query q1("INSERT INTO t_Key (id, value, type, action, alias, arming_group_id) "
                        "VALUES (:id, :value, :type, :action, :alias, :arming_group_id)");
            q1.bind(":id", static_cast<int>(pKey->_id));
            q1.bind(":action", static_cast<int>(pKey->_action));
            q1.bind(":type", static_cast<int>(pKey->_type));
            q1.bind(":value", pKey->_value);
            q1.bind(":alias", pKey->_alias);
            q1.bind(":arming_group_id", DB_VALUE_NULL == groupId ? QVariant() : groupId);
            qx::dao::call_query(q1, db);
        }
    }
    else if (OBJ_TOUCHMEMORY_CODE == keyType) {
        size = data->size() / TMCPKT_MAX;
        for (int i = 0; i < size; ++i) {
            t_Key_ptr pKey(new t_Key);
            pKey->_id =     (quint8)data->at(TMCPKT_ID_H + i*TMCPKT_MAX) << 8 | (quint8)data->at(TMCPKT_ID_L + i*TMCPKT_MAX);
            pKey->_type =   keyType;
            pKey->_action = (quint8)data->at(TMCPKT_ACTION + i*TMCPKT_MAX);
            pKey->_alias =  codec->toUnicode(data->mid(TMCPKT_ALIAS_H + i*TMCPKT_MAX, ALIAS_LEN).constData());

            QByteArray reversed_key(data->mid(TMCPKT_KEY_H + 1 + i*TMCPKT_MAX, TOUCHMEMORY_MAX_KEY_SIZE - 2));
            pKey->_value.resize(reversed_key.size());
            std::reverse_copy(reversed_key.constBegin(), reversed_key.constEnd(), pKey->_value.begin());

            groupId = (quint8)data->at(TMCPKT_ARMING_GROUP_ID_H + i*TMCPKT_MAX) << 8 |
                          (quint8)data->at(TMCPKT_ARMING_GROUP_ID_L + i*TMCPKT_MAX);

            qx_query q1("INSERT INTO t_Key (id, value, type, action, alias, arming_group_id) "
                        "VALUES (:id, :value, :type, :action, :alias, :arming_group_id)");
            q1.bind(":id", static_cast<int>(pKey->_id));
            q1.bind(":action", static_cast<int>(pKey->_action));
            q1.bind(":type", static_cast<int>(pKey->_type));
            q1.bind(":value", pKey->_value);
            q1.bind(":alias", pKey->_alias);
            q1.bind(":arming_group_id", DB_VALUE_NULL == groupId ? QVariant() : groupId);
            qx::dao::call_query(q1, db);
        }
    }
}
