//============================================================================
// Name        : customizeviewdialog.h
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Диалог под настройки
//============================================================================
#include "customizeviewdialog.h"
#include "ui_customizeviewdialog.h"

#ifdef __DEBUG
#include <QDebug>
#endif

#include "msettings.h"

CustomizeViewDialog::CustomizeViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomizeViewDialog),
    changingFlag(false)
{
    ui->setupUi(this);

    ui->maxScaleSpinBox->setMinimum(SCALE_LIMIT_MIN);

    connect(ui->scaleStepSpinBox,SIGNAL(valueChanged(double)),this,SLOT(changeScaleStepSilderValue()));
    connect(ui->scaleStepSlider,SIGNAL(valueChanged(int)),this,SLOT(changeScaleStepSpinBoxValue()));

    connect(ui->grabScaleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(changeGrabScaleSliderValue()));
    connect(ui->grabScaleSlider,SIGNAL(valueChanged(int)),this,SLOT(changeGrabScaleSpinBoxValue()));

    connect(ui->maxScaleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(changeMaxScaleSliderValue()));
    connect(ui->maxScaleSlider,SIGNAL(valueChanged(int)),this,SLOT(changeMaxScaleSpinBoxValue()));

    connect(ui->grabSpeedSpinBox,SIGNAL(valueChanged(double)),this,SLOT(changeGrabSpeedSliderValue()));
    connect(ui->grabSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(changeGrabSpeedSpinBoxValue()));

}

CustomizeViewDialog::~CustomizeViewDialog()
{
    delete ui;
}

//Не напартачить бы с точностью, а то будут друг другу рекурсивно значения менять
//Если возникнут проблемы, то код калибровки прийдётся добавить. Чёрт, не проверяя добавил
void CustomizeViewDialog::changeSpinBoxValue(QDoubleSpinBox *sb, QSlider *slider)
{
    if(!changingFlag)
        sb->setValue((sb->maximum()-sb->minimum())*slider->value()/100+sb->minimum());
}

void CustomizeViewDialog::changeSliderValue(QSlider *slider, QDoubleSpinBox *sb)
{
    changingFlag = true;
    int value = (sb->value()-sb->minimum())*100/(sb->maximum()-sb->minimum());
//    //Калибровка
//    int val1 = value;
//    int fl = 0;
//    double check;
//    while((check=(sb->maximum()-sb->minimum())*value/100+sb->minimum())!=sb->value()) {
//        if(check>sb->value()) {
//            value--;
//            if(!(fl==0 || fl==-1)) {
//                value = val1;
//                break;
//            }
//            fl = -1;
//        }
//        else {
//            value++;
//            if(!(fl==0 || fl==1)) {
//                value = val1;
//                break;
//            }
//            fl = 1;
//        }

//    }
    //
    slider->setValue(value);
    changingFlag = false;
}


void CustomizeViewDialog::init(const MSettings &settings)
{
    ui->maxScaleSpinBox->setValue(settings.scaleLimit());
    ui->grabScaleSpinBox->setValue(settings.grabScale());
    ui->scaleStepSpinBox->setValue(settings.scaleDelta());
    ui->grabSpeedSpinBox->setValue(settings.mouseGrabSpeed());
}

void CustomizeViewDialog::changeScaleStepSilderValue()
{
    changeSliderValue(ui->scaleStepSlider,ui->scaleStepSpinBox);
}

void CustomizeViewDialog::changeScaleStepSpinBoxValue()
{
    changeSpinBoxValue(ui->scaleStepSpinBox,ui->scaleStepSlider);
}

void CustomizeViewDialog::changeGrabScaleSliderValue()
{
    changeSliderValue(ui->grabScaleSlider,ui->grabScaleSpinBox);
}


void CustomizeViewDialog::changeGrabScaleSpinBoxValue()
{
    changeSpinBoxValue(ui->grabScaleSpinBox,ui->grabScaleSlider);
}


void CustomizeViewDialog::changeMaxScaleSliderValue()
{
    changeSliderValue(ui->maxScaleSlider,ui->maxScaleSpinBox);
    setMaxScaleRestrictions();
}

void CustomizeViewDialog::changeMaxScaleSpinBoxValue()
{
    changeSpinBoxValue(ui->maxScaleSpinBox,ui->maxScaleSlider);
}

void CustomizeViewDialog::setMaxScaleRestrictions()
{
    ui->grabScaleSpinBox->setMaximum(ui->maxScaleSpinBox->value());
    ui->scaleStepSpinBox->setMaximum(ui->maxScaleSpinBox->value());
    changeScaleStepSilderValue();
    changeGrabScaleSliderValue();
}

double CustomizeViewDialog::maxScale() const
{
    return ui->maxScaleSpinBox->value();
}

double CustomizeViewDialog::scaleStep() const
{
    return ui->scaleStepSpinBox->value();
}

double CustomizeViewDialog::grabScale() const
{
    return ui->grabScaleSpinBox->value();
}

double CustomizeViewDialog::grabSpeed() const
{
    return ui->grabSpeedSpinBox->value();
}


void CustomizeViewDialog::changeGrabSpeedSliderValue()
{
    changeSliderValue(ui->grabSpeedSlider,ui->grabSpeedSpinBox);
}

void CustomizeViewDialog::changeGrabSpeedSpinBoxValue()
{
    changeSpinBoxValue(ui->grabSpeedSpinBox,ui->grabSpeedSlider);
}
