#include "precompiled.h"

#include "commontrigger.h"

#include <QxMemLeak.h>


ARMOR_QX_REGISTER_CPP(CommonTrigger)

namespace qx {
template <>
void register_class(QxClass<CommonTrigger> &t)
{
    //IxDataMember * pData = NULL;
    Q_UNUSED(t)

}
} // namespace qx


void CommonTrigger::onAfterInsert(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void CommonTrigger::onAfterDelete(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}

void CommonTrigger::onBeforeUpdate(qx::dao::detail::IxDao_Helper *dao)
{
    Q_UNUSED(dao)
}
