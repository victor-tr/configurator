#include "precompiled.h"

#include "s_relationetrgroup.h"

#include "configurator_protocol.h"

#include <QxMemLeak.h>


namespace bo {

QSharedPointer<QByteArray> fetchRelationsEtrGroupToByteArray()
{
    qx_query query("SELECT * FROM s_ArmingGroup_ETR");
    qx::dao::call_query(query);

    int listsize = query.getSqlResultRowCount();

    QByteArray *ret = listsize ? new QByteArray() : NULL;
    if (ret) {
        ret->append(static_cast<char>(OBJ_RELATION_ETR_AGROUP));
        ret->append(static_cast<char>(REAGPKT_MAX >> 8));
        ret->append(REAGPKT_MAX);

        for (int i = 0; i < listsize; ++i) {
            QByteArray temp(REAGPKT_MAX, '\0');

            int etr_id = query.getSqlResultAt(i, "etr_id").toInt();
            temp[REAGPKT_ETR_ID_H] = etr_id >> 8;
            temp[REAGPKT_ETR_ID_L] = etr_id;

            int group_id = query.getSqlResultAt(i, "arming_group_id").toInt();
            temp[REAGPKT_GROUP_ID_H] = group_id >> 8;
            temp[REAGPKT_GROUP_ID_L] = group_id;

            ret->append(temp);
        }
    }

    return QSharedPointer<QByteArray>(ret);
}

void insertRelationsEtrGroupFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db)
{
    int size = data->size() / REAGPKT_MAX;

    for (int i = 0; i < size; ++i) {
        qx_query query("INSERT INTO s_ArmingGroup_ETR (arming_group_id, etr_id) VALUES (:group_id, :etr_id)");
        query.bind(":group_id", (quint8)data->at(REAGPKT_GROUP_ID_H + i*REAGPKT_MAX) << 8 | (quint8)data->at(REAGPKT_GROUP_ID_L + i*REAGPKT_MAX));
        query.bind(":etr_id",   (quint8)data->at(REAGPKT_ETR_ID_H + i*REAGPKT_MAX) << 8   | (quint8)data->at(REAGPKT_ETR_ID_L + i*REAGPKT_MAX));
        qx::dao::call_query(query, db);
    }
}

} // namespace bo
