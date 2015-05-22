#ifndef ADCCALIBRATOR_H
#define ADCCALIBRATOR_H

#include <QDialog>

namespace Ui {
class AdcCalibrator;
}

class AbstractCommunicator;

class AdcCalibrator : public QDialog
{
    Q_OBJECT

    enum CalibrationStep {
        Step_SetFirstPoint,
        Step_SetSecondPoint,
        Step_Finish
    };

    struct Point {
        Point(): Vin(0), adcValue(0) {}
        float Vin;
        int adcValue;
    };

public:
    explicit AdcCalibrator(AbstractCommunicator *pCommunicator, QWidget *parent = 0);
    ~AdcCalibrator();

Q_SIGNALS:
    void snCalibrationDone(float a, float b);

private:
    void doCurrentAction();
    void handleAdcData(quint16 adcValue);
    void evaluateQuotients();
    bool updateDB();
    void updateUI();

    Ui::AdcCalibrator    *ui;
    CalibrationStep       _currentStep;
    AbstractCommunicator *_pCommunicator;

    Point _first;
    Point _second;
    float _a;
    float _b;
};

#endif // ADCCALIBRATOR_H
