#include "precompiled.h"
#include "adccalibrator.h"
#include "ui_adccalibrator.h"
#include "abstractcommunicator.h"
#include "bo/t_commonsettings.h"



AdcCalibrator::AdcCalibrator(AbstractCommunicator *pCommunicator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdcCalibrator),
    _currentStep(Step_SetFirstPoint),
    _pCommunicator(pCommunicator),
    _a(0), _b(0)
{
    ui->setupUi(this);

    connect(ui->btn_cancel, &QPushButton::pressed, this, &AdcCalibrator::close);
    connect(ui->btn_ok,     &QPushButton::pressed, this, &AdcCalibrator::doCurrentAction);
    connect(pCommunicator,  &AbstractCommunicator::snlAdcDataReceived, this, &AdcCalibrator::handleAdcData);
    connect(ui->le_Vin,     &QLineEdit::textChanged, this, &AdcCalibrator::updateUI);

    ui->btn_ok->setText(tr("Set 1st point"));

    QDoubleValidator *v = new QDoubleValidator(0.1, 15.0, 1, this);
    v->setLocale(QLocale("C"));
    ui->le_Vin->setValidator(v);
    updateUI();
}

AdcCalibrator::~AdcCalibrator()
{
    delete ui;
}

void AdcCalibrator::doCurrentAction()
{
    switch (_currentStep) {
    case Step_SetFirstPoint:
        _pCommunicator->requestAdcData(this);
        break;
    case Step_SetSecondPoint:
    {
        const float currentVin = ui->le_Vin->text().toFloat();
        if (currentVin <= _first.Vin) {
            ui->le_Vin->clear();
            // TODO: notify user about incorrect Vin value
        } else {
            _pCommunicator->requestAdcData(this);
        }
        break;
    }
    case Step_Finish:
        if (updateDB()) {
            Q_EMIT snCalibrationDone(_a, _b);
        } else {
            // TODO: notify user about fail
        }
        close();
        break;
    default:
        break;
    }

    ui->btn_ok->setEnabled(false);
}

void AdcCalibrator::handleAdcData(quint16 adcValue)
{
    ui->btn_ok->setEnabled(true);
    ui->btn_ok->setFocus();

    switch (_currentStep) {
    case Step_SetFirstPoint:
        _first.Vin = ui->le_Vin->text().toFloat();
        _first.adcValue = adcValue;
        ui->le_first_point->setText(QString::number(adcValue));
        ui->btn_ok->setText(tr("Set 2nd point"));
        _currentStep = Step_SetSecondPoint;
        break;
    case Step_SetSecondPoint:
        _second.Vin = ui->le_Vin->text().toFloat();
        _second.adcValue = adcValue;
        ui->le_second_point->setText(QString::number(adcValue));
        evaluateQuotients();
        ui->btn_ok->setText(tr("Finish"));
        _currentStep = Step_Finish;
        break;
    default:
        break;
    }
}

void AdcCalibrator::evaluateQuotients()
{
    _a = (_second.adcValue - _first.adcValue) / (_second.Vin - _first.Vin);
    _b = _first.adcValue - _first.Vin * _a;
    ui->le_a->setText(QString::number(_a));
    ui->le_b->setText(QString::number(_b));
}

bool AdcCalibrator::updateDB()
{
    t_CommonSettings_ptr pSettings(new t_CommonSettings);
    pSettings->_id = 1;
    QSqlError err = qx::dao::fetch_by_id(pSettings);
    if (err.isValid())
        return false;
    pSettings->_adc_a = _a;
    pSettings->_adc_b = _b;
    err = qx::dao::update(pSettings);
    return !err.isValid();
}

void AdcCalibrator::updateUI()
{ ui->btn_ok->setEnabled(!ui->le_Vin->text().isEmpty()); }
