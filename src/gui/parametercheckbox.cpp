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


#include "parametercheckbox.h"

ParameterCheckBox::ParameterCheckBox(
        ModelParameter parameter,
        bool show_parameter_name,
        QWidget* parent) :
    QCheckBox(show_parameter_name ? parameter.GetNameWithUnit() : QString(),
              parent),
    parameter_(parameter)
{
    setChecked(false);
    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(EmitSignals(bool)));
}

ParameterCheckBox::~ParameterCheckBox()
{
    if (isChecked())
        emit ParameterDisabled(QString::fromStdString(parameter_.name));
}

const std::string& ParameterCheckBox::GetParameterName() const
{
    return parameter_.name;
}

void ParameterCheckBox::EmitSignals(bool checked)
{
    if (checked)
        emit ParameterEnabled(QString::fromStdString(parameter_.name));
    else
        emit ParameterDisabled(QString::fromStdString(parameter_.name));
    emit ToggledInverse(!checked);
}