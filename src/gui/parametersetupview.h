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


#ifndef PARAMETERSETUPVIEW_H
#define PARAMETERSETUPVIEW_H

#include <QTableView>

class DoubleEditorFactory;
class HeaderComboBox;
class ParameterSetupModel;
class QMenu;

class ParameterSetupView : public QTableView
{
    Q_OBJECT
    
public:
    explicit ParameterSetupView(QWidget* parent = 0);
    virtual ~ParameterSetupView();
    
    void setModel(ParameterSetupModel* model);
    
private slots:
    void SetActiveHeader(QString parameter_name);
    void ShowHeaderComboBox(int section);
    void Paste();
    
private:
    HeaderComboBox* combo_box_;
    int border_;
    int active_section_;
    DoubleEditorFactory* editor_factory_;
    
signals:
    void HeaderParameterChanged(int section, QString parameter_name);
};

#endif // PARAMETERSETUPVIEW_H