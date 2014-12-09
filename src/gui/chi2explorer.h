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


#ifndef CHI2EXPLORER_H
#define CHI2EXPLORER_H

#include <QMainWindow>

#include <map>
#include <memory>

#include "ui_chi2explorer.h"

class Chi2ParameterSlider;
class Chi2ParameterConfigModel;
class ContourPlotData;
class DoubleEditorFactory;
class QDoubleValidator;

class Chi2Explorer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Chi2Explorer(ContourPlotData* fitter,
                          const std::vector<std::string>& sample_names,
                          const std::vector<std::string>& parameter_names);
    virtual ~Chi2Explorer();

private slots:
    void RunFit();
    void UpdateStatusBar(bool cursor_over_plot,
                         QPointF cursor_coordinates);
    void UpdateChi2Interval();
    void UpdateChi2IntervalLineEdits();
    void UpdateParameterSliders();
    void UpdateParameterSliderValues();

private:
    void ClearSliderLayout();
    void InitializeSampleComboBox(const std::vector<std::string>& sample_names);
    void InitializeParameterConfigTable();
    
    std::shared_ptr<Ui::Chi2Explorer> ui;
    const std::vector<std::string> parameter_names_;
    ContourPlotData* plot_data_;
    DoubleEditorFactory* editor_factory_;
    QDoubleValidator* validator_;
    Chi2ParameterConfigModel* parameter_config_model_;
    std::map<int, Chi2ParameterSlider*> slider_map_;
};

#endif // CHI2EXPLORER_H
