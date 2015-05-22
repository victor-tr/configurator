#ifndef ABSTRACTMANAGER_H
#define ABSTRACTMANAGER_H

#include <QWidget>


class QListWidgetItem;
class QListWidget;
class ManagerWidget;

template <typename T>
class bo_traits
{
public:
    typedef qx::dao::ptr<T>                  type_ptr;
    typedef type_ptr &                       type_ptr_ref;
    typedef const type_ptr &                 type_ptr_const_ref;
    typedef qx::QxCollection<int, type_ptr>  type_list;
    typedef qx::dao::ptr<type_list>          type_list_ptr;
};


template <typename T, typename Traits = bo_traits<T> >
class AbstractManager
{
public:

    typedef typename Traits::type_ptr            type_ptr;
    typedef typename Traits::type_ptr_ref        type_ptr_ref;
    typedef typename Traits::type_ptr_const_ref  type_ptr_const_ref;
    typedef typename Traits::type_list           type_list;
    typedef typename Traits::type_list_ptr       type_list_ptr;

    enum {
        IdRole   = Qt::UserRole,
        DataRole
    };

    explicit AbstractManager();
    virtual ~AbstractManager() {}

    static bool isModified() { return _bModified; }

protected:

    virtual void setModifiedHook(bool modified)      = 0;

    virtual void fillFormHook(type_ptr_const_ref pElement) = 0;
    virtual void clearForm()                     = 0;
    virtual void saveForm(type_ptr_ref pElement) = 0;

    virtual QListWidget *mainListWidget() = 0;
    virtual QObject *sender() const       = 0;

    virtual bool canBeDeleted(type_ptr_const_ref pElement);
    virtual void updateUIHook() {}

    void updateList(QObject *me);
    void updateUI();

    bool apply();
    void cancel();

    QListWidgetItem *currentItem(); ///< Current item of main manager's list
    type_ptr currentElement();

private:

    void setModified(bool modified);
    void fillForm();

    static bool _bModified;

};


class ManagerWidget : public QWidget
{
    Q_OBJECT
//    template<typename T, typename Traits> friend class AbstractManager;

public:

    explicit ManagerWidget(QWidget *parent = 0) : QWidget(parent) {}

private:

signals:

    void snlListChanged();

};


#include "abstractmanager.inl"


#endif // ABSTRACTMANAGER_H
