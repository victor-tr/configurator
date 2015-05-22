#ifndef LEDINDICATOR_H
#define LEDINDICATOR_H

#include "qled.h"
#include <QHBoxLayout>
#include <QTimer>

#define ON_INTERVAL 150


class LedIndicator : public QWidget
{
    Q_OBJECT

public:

    explicit LedIndicator(QWidget *parent = 0)
        : QWidget(parent)
        , _txLed(new QLed())
        , _rxLed(new QLed())
    {
        _onTxTimer.setSingleShot(true);
        _onTxTimer.setInterval(ON_INTERVAL);
        connect(&_onTxTimer, &QTimer::timeout, _txLed, &QLed::toggleValue);

        _onRxTimer.setSingleShot(true);
        _onRxTimer.setInterval(ON_INTERVAL);
        connect(&_onRxTimer, &QTimer::timeout, _rxLed, &QLed::toggleValue);

        _txLed->setOnColor(QLed::Green);
        _rxLed->setOnColor(QLed::Blue);

        QHBoxLayout *l = new QHBoxLayout;
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(5);
        l->setMargin(0);
        l->addWidget(_txLed);
        l->addWidget(_rxLed);
        l->setSizeConstraint(QLayout::SetFixedSize);
        setLayout(l);
    }
    
public Q_SLOTS:

    void indicateTx() { _txLed->setValue(true); _onTxTimer.start(); }
    void indicateRx() { _rxLed->setValue(true); _onRxTimer.start(); }

private:

    QTimer _onTxTimer;
    QTimer _onRxTimer;
    QLed *_txLed;
    QLed *_rxLed;
    
};

#endif // LEDINDICATOR_H
