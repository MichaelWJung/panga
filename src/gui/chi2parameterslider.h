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


#ifndef CHI2PARAMETERSLIDER_H
#define CHI2PARAMETERSLIDER_H

#include <QWidget>

class QwtInterval;
namespace Ui {
    class Chi2ParameterSlider;
}

class Chi2ParameterSlider : public QWidget
{
    Q_OBJECT
    
public:
    explicit Chi2ParameterSlider(QString parameter_name,
                                 int parameter_index,
                                 QWidget* parent = nullptr);
    virtual ~Chi2ParameterSlider();
    
    void SetInterval(const QwtInterval& interval);
    void SetValue(double value);
    
signals:
    void ValueChanged(int index, double value);
    
private slots:
    void AdaptForChangedSlider();
    void SetValueFromSliderPosition();
    void SetSliderPositionFromValue();
    
private:
    Ui::Chi2ParameterSlider* ui;
    double min_;
    double max_;
    double value_;
    int parameter_index_;
    
    static const unsigned SLIDER_RESOLUTION;
};

#endif // CHI2PARAMETERSLIDER_H
