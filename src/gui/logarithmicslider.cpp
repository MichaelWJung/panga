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


#include <cmath>

#include "logarithmicslider.h"

LogarithmicSlider::LogarithmicSlider(QWidget* parent) :
    QSlider(parent),
    value_last_set_(0)
{
    ConnectToParent();
}

LogarithmicSlider::LogarithmicSlider(Qt::Orientation orientation,
                                     QWidget* parent) :
    QSlider(parent),
    value_last_set_(0)
{
    ConnectToParent();
}

void LogarithmicSlider::ConnectToParent()
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(ConvertToLogAndEmit(int)));
}

void LogarithmicSlider::setValue(int value)
{
    value = (maximum() - minimum()) *
            std::log(double(value)     / minimum()) /
            std::log(double(maximum()) / minimum()) +
            minimum();
    value_last_set_ = value;
    QSlider::setValue(value);
}

void LogarithmicSlider::ConvertToLogAndEmit(int value)
{
    if (value_last_set_ == value)
        return;
    double x = (double(value) - minimum()) / (maximum() - minimum());
    value = minimum() * std::pow(double(maximum()) / minimum(), x);
    emit valueChangedLog(value);
}
