// Copyright © 2014 Michael Jung
// 
// This file is part of Panga.
// 
// Panga is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Panga is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Panga.  If not, see <http://www.gnu.org/licenses/>.


#include "qwt_interval.h"

#include "chi2parameterslider.h"
#include "ui_chi2parameterslider.h"

const unsigned Chi2ParameterSlider::SLIDER_RESOLUTION = 1000;

Chi2ParameterSlider::Chi2ParameterSlider(
        QString parameter_name,
        int parameter_index,
        QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Chi2ParameterSlider)
    , min_(0.)
    , max_(40.)
    , value_(20.)
    , parameter_index_(parameter_index)
{
    ui->setupUi(this);
    ui->label->setText(parameter_name);
    ui->min_label->setText(QString::number(min_));
    ui->max_label->setText(QString::number(max_));
    ui->slider->setMinimum(0);
    ui->slider->setMaximum(SLIDER_RESOLUTION);
    
    connect(ui->slider, SIGNAL(valueChanged(int)),
            this, SLOT(AdaptForChangedSlider()));
}

Chi2ParameterSlider::~Chi2ParameterSlider()
{
    delete ui;
}

void Chi2ParameterSlider::SetInterval(const QwtInterval& interval)
{
    min_ = interval.minValue();
    max_ = interval.maxValue();
    ui->min_label->setText(QString::number(min_));
    ui->max_label->setText(QString::number(max_));
    if (value_ < min_) value_ = min_;
    if (value_ > max_) value_ = max_;
    SetSliderPositionFromValue();
}

void Chi2ParameterSlider::SetValue(double value)
{
    value_ = value;
    SetSliderPositionFromValue();
}

void Chi2ParameterSlider::AdaptForChangedSlider()
{
    SetValueFromSliderPosition();
    emit ValueChanged(parameter_index_, value_);
}

void Chi2ParameterSlider::SetValueFromSliderPosition()
{
    value_ = min_ + (max_ - min_) * ui->slider->value() / SLIDER_RESOLUTION;
}

void Chi2ParameterSlider::SetSliderPositionFromValue()
{
    ui->slider->setValue((value_ - min_) / (max_ - min_) * SLIDER_RESOLUTION);
}
