#ifndef COMMONTRIGGER_H
#define COMMONTRIGGER_H


class ARMOR_DLL_EXPORT CommonTrigger
{

    QX_REGISTER_FRIEND_CLASS(CommonTrigger);

public:

    CommonTrigger()          { ; }
    virtual ~CommonTrigger() { ; }

    virtual void onAfterInsert(qx::dao::detail::IxDao_Helper * dao);
    virtual void onAfterDelete(qx::dao::detail::IxDao_Helper * dao);
    virtual void onBeforeUpdate(qx::dao::detail::IxDao_Helper * dao);

protected:

    virtual void makeAbstractClass() = 0;

};

QX_REGISTER_ABSTRACT_CLASS(CommonTrigger);
ARMOR_QX_REGISTER_HPP(CommonTrigger, qx::trait::no_base_class_defined, 0);

namespace qx {
namespace dao {
namespace detail {

template <>
struct QxDao_Trigger<CommonTrigger>
{

   static inline void onBeforeInsert(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao) { Q_UNUSED(t); Q_UNUSED(dao); }
   static inline void onBeforeUpdate(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao) { if (t) { t->onBeforeUpdate(dao); } }
   static inline void onBeforeDelete(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao) { Q_UNUSED(t); Q_UNUSED(dao); }
   static inline void onBeforeFetch(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao)  { Q_UNUSED(t); Q_UNUSED(dao); }
   static inline void onAfterInsert(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao)  { if (t) { t->onAfterInsert(dao); } }
   static inline void onAfterUpdate(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao)  { Q_UNUSED(t); Q_UNUSED(dao); }
   static inline void onAfterDelete(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao)  { if (t) { t->onAfterDelete(dao); } }
   static inline void onAfterFetch(CommonTrigger * t, qx::dao::detail::IxDao_Helper * dao)   { Q_UNUSED(t); Q_UNUSED(dao); }

};

}  // namespace detail
}  // namespace dao
}  // namespace qx

#endif // COMMONTRIGGER_H
