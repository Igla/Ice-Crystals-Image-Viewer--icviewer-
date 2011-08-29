//============================================================================
// Name        : customizeviewdialog.h
// Author      : Igla
// Version     :
// Copyright   : 2011 Ice Crystals, Lopasoft corporation and this fucking world
// Description : Диалог под настройки
//============================================================================
#ifndef CUSTOMIZEVIEWDIALOG_H
#define CUSTOMIZEVIEWDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QSlider;
QT_END_NAMESPACE

class MSettings;

namespace Ui {
    class CustomizeViewDialog;
}

class CustomizeViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomizeViewDialog(QWidget *parent = 0);
    ~CustomizeViewDialog();

    void init(const MSettings &settings);

    double maxScale() const;
    double scaleStep() const;
    double grabScale() const;
    double grabSpeed() const;


private slots:
    void changeScaleStepSilderValue();
    void changeScaleStepSpinBoxValue();

    void changeGrabScaleSliderValue();
    void changeGrabScaleSpinBoxValue();

    void changeMaxScaleSliderValue();
    void changeMaxScaleSpinBoxValue();

    void changeGrabSpeedSliderValue();
    void changeGrabSpeedSpinBoxValue();
private:
    void setMaxScaleRestrictions();
    void changeSpinBoxValue(QDoubleSpinBox *sb, QSlider *slider);
    void changeSliderValue(QSlider *slider, QDoubleSpinBox *sb);
private:
    //Такая ерунда, которая нафиг не нужна, но поскольку я решил использовать бегунок с процентами скреплённый
    // с spinBox-ом c double value. В ообщеи, чтобы при установке на spinBox нормальных значений они таковыми оставались.
    // Памятуя о погрешности вычислений и прочем.
    bool changingFlag;

    Ui::CustomizeViewDialog *ui;
};

#endif // CUSTOMIZEVIEWDIALOG_H
