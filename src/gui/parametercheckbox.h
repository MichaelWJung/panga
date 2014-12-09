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


#ifndef PARAMETERCHECKBOX_H
#define PARAMETERCHECKBOX_H

#include <QCheckBox>

#include "core/models/modelparameter.h"


class ParameterCheckBox : public QCheckBox
{
    Q_OBJECT
    
public:
    explicit ParameterCheckBox(
            ModelParameter parameter,
            bool show_parameter_name,
            QWidget* parent = 0);
    virtual ~ParameterCheckBox();
    
    const std::string& GetParameterName() const;
    
signals:
    void ParameterEnabled(QString name);
    void ParameterDisabled(QString name);
    void ToggledInverse(bool disabled);
    
private slots:
    void EmitSignals(bool checked);

private:
    ModelParameter parameter_;
};

#endif // PARAMETERCHECKBOX_H
