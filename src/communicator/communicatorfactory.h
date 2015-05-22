#ifndef CUMMUNICATORFACTORY_H
#define CUMMUNICATORFACTORY_H


class AbstractCommunicator;
class MainWindow;

class CommunicatorCreator
{
public:

    static CommunicatorCreator *instance()
    {
        static CommunicatorCreator _instance;
        return &_instance;
    }

    template <typename T>
    static AbstractCommunicator *createCommunicator(MainWindow *mainwindow)
    {
        T *ac = new T(mainwindow);
        Q_ASSERT_X(qobject_cast<AbstractCommunicator *>(ac),
                   "CommunicatorCreator::createCommunicator()",
                   "Unknown communicator type");
        return ac;
    }

protected:

    explicit CommunicatorCreator() { ; }
    virtual ~CommunicatorCreator() { ; }

private:

    CommunicatorCreator(const CommunicatorCreator &) ;
    void operator=(const CommunicatorCreator &) ;

};


#endif // CUMMUNICATORFACTORY_H
