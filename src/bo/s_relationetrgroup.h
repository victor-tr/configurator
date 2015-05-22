#ifndef S_RELATIONETRGROUP_H
#define S_RELATIONETRGROUP_H

namespace bo {

QSharedPointer<QByteArray> fetchRelationsEtrGroupToByteArray();
void insertRelationsEtrGroupFromByteArray(QSharedPointer<QByteArray> data, QSqlDatabase *db = 0);

} // namespace bo


#endif // S_RELATIONETRGROUP_H
