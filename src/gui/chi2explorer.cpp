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


#include <QDoubleValidator>
#include <QFormLayout>
#include <QStyledItemDelegate>

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "chi2parameterconfigmodel.h"
#include "chi2parameterslider.h"
#include "commons.h"
#include "contourplotdata.h"
#include "doubleeditorfactory.h"
#include "mainwindow.h"

#include "chi2explorer.h"

Chi2Explorer::Chi2Explorer(ContourPlotData* fitter,
                           const std::vector<std::string>& sample_names,
                           const std::vector<std::string>& parameter_names) :
    QMainWindow(nullptr),
    ui(new Ui::Chi2Explorer),
    parameter_names_(parameter_names),
    plot_data_(fitter),
    editor_factory_(new DoubleEditorFactory()),
    validator_(new QDoubleValidator(this)),
    parameter_config_model_(nullptr),
    slider_map_()
{
    if (parameter_names.size() < 2)
        throw std::invalid_argument("Chi2Explorer expects at least two "
                                    "parameters");
    if (sample_names.empty())
        throw std::invalid_argument("Sample list is empty.");
    
    MainWindow::theMainWindow->AddWindowToOpenWindowsVector(this);
    ui->setupUi(this);
    statusBar()->showMessage("");
    ui->plot->SetRasterData(plot_data_);
    dynamic_cast<QStyledItemDelegate*>(
            ui->parameter_config_tableview->itemDelegate())->
                setItemEditorFactory(editor_factory_);
    ui->parameter_config_tableview->setModel(
            new Chi2ParameterConfigModel(parameter_names, this));
    
    ui->parameter_config_tableview->resizeColumnsToContents();
    ui->parameter_config_tableview->resizeRowsToContents();
    validator_->setBottom(1e-50);
    ui->chi2_scale_minimum_lineedit->setValidator(validator_);
    ui->chi2_scale_maximum_lineedit->setValidator(validator_);
    
    InitializeSampleComboBox(sample_names);
    InitializeParameterConfigTable();
    
    connect(ui->update_plot_button, SIGNAL(pressed()), this, SLOT(RunFit()));
    connect(ui->autoupdate_plot_checkbox, SIGNAL(toggled(bool)),
            plot_data_, SLOT(SetAutoUpdatePlot(bool)));
    connect(ui->plot, SIGNAL(CursorPositionChanged(bool,QPointF)),
            this, SLOT(UpdateStatusBar(bool,QPointF)));
    connect(ui->chi2_scale_autoupdate_checkbox, SIGNAL(toggled(bool)),
            plot_data_, SLOT(SetAutoUpdateChi2Interval(bool)));
    connect(ui->chi2_scale_minimum_lineedit, SIGNAL(textChanged(QString)),
            this, SLOT(UpdateChi2Interval()));
    connect(ui->chi2_scale_maximum_lineedit, SIGNAL(textChanged(QString)),
            this, SLOT(UpdateChi2Interval()));
    connect(plot_data_, SIGNAL(ZIntervalChanged()),
            this, SLOT(UpdateChi2IntervalLineEdits()));
    
    setAttribute(Qt::WA_DeleteOnClose);
}

Chi2Explorer::~Chi2Explorer()
{
    delete editor_factory_;
}

void Chi2Explorer::RunFit()
{
    plot_data_->Fit();
}

void Chi2Explorer::InitializeSampleComboBox(
        const std::vector<std::string>& sample_names)
{
    for (const auto& name : sample_names)
        ui->sample_combobox->addItem(QString::fromStdString(name));
    if (!sample_names.empty())
        plot_data_->SetSampleNumber(ui->sample_combobox->currentIndex());
    connect(ui->sample_combobox, SIGNAL(currentIndexChanged(int)),
            plot_data_, SLOT(SetSampleNumber(int)));
    ui->plot->SetTitle(ui->sample_combobox->currentText());
    connect(ui->sample_combobox, SIGNAL(currentIndexChanged(QString)),
            ui->plot, SLOT(SetTitle(QString)));
}

void Chi2Explorer::InitializeParameterConfigTable(
        )
{
    parameter_config_model_ = 
            new Chi2ParameterConfigModel(parameter_names_, this);
    ui->parameter_config_tableview->setModel(parameter_config_model_);
    ui->parameter_config_tableview->resizeColumnsToContents();
    ui->parameter_config_tableview->resizeRowsToContents();
    
    plot_data_->SetAxesParameters(parameter_config_model_->GetXParameter(),
                                  parameter_config_model_->GetYParameter());
    ui->plot->SetAxesScales(parameter_config_model_->GetXInterval(),
                            parameter_config_model_->GetYInterval());
    ui->plot->SetAxesTitles(parameter_config_model_->GetXParameterName(),
                            parameter_config_model_->GetYParameterName());
    
    connect(parameter_config_model_, SIGNAL(AxesParametersChanged(int,int)),
            plot_data_, SLOT(SetAxesParameters(int,int)));
    connect(parameter_config_model_,
                SIGNAL(AxesParametersChanged(QString,QString)),
            ui->plot, SLOT(SetAxesTitles(QString,QString)));
    connect(parameter_config_model_,
                SIGNAL(AxesIntervalsChanged(QwtInterval,QwtInterval)),
            ui->plot, SLOT(SetAxesScales(QwtInterval,QwtInterval)));
    connect(parameter_config_model_,
                SIGNAL(AxesIntervalsChanged(QwtInterval,QwtInterval)),
            plot_data_, SLOT(AutoUpdateWithNextInitRaster()));
    connect(parameter_config_model_,
            SIGNAL(FixedParametersChanged(std::vector<std::pair<int, double>>)),
            plot_data_,
            SLOT(SetFixedParameters(std::vector<std::pair<int,double>>)));
    connect(parameter_config_model_,
            SIGNAL(FixedParametersChanged(std::vector<std::pair<int, double>>)),
            ui->plot, SLOT(replot()));
    connect(parameter_config_model_, SIGNAL(FixedParametersChanged()),
            this, SLOT(UpdateParameterSliders()));
    connect(parameter_config_model_, SIGNAL(AxesParametersChanged(int,int)),
            this, SLOT(UpdateParameterSliders()));
}

void Chi2Explorer::UpdateStatusBar(bool cursor_over_plot,
                                   QPointF cursor_coordinates)
{
    QString message;
    if (cursor_over_plot)
        message = plot_data_->GetInfoForPoint(cursor_coordinates.x(),
                                              cursor_coordinates.y());
    statusBar()->showMessage(message);
}

void Chi2Explorer::UpdateChi2Interval()
{
    const double min = ui->chi2_scale_minimum_lineedit->text().toDouble();
    const double max = ui->chi2_scale_maximum_lineedit->text().toDouble();
    ui->chi2_scale_autoupdate_checkbox->setChecked(false);
    if (min <= 0 || max <= 0)
        return;
    plot_data_->SetCustomChi2Interval(QwtInterval(min, max));
}

void Chi2Explorer::UpdateChi2IntervalLineEdits()
{
    if (!plot_data_->AutoUpdateChi2Interval()) return;
    SignalBlocker blocker1(ui->chi2_scale_minimum_lineedit);
    SignalBlocker blocker2(ui->chi2_scale_maximum_lineedit);
    QwtInterval chi2_interval = plot_data_->interval(Qt::ZAxis);
    ui->chi2_scale_minimum_lineedit->setText(
            QString::number(chi2_interval.minValue()));
    ui->chi2_scale_maximum_lineedit->setText(
            QString::number(chi2_interval.maxValue()));
}

void Chi2Explorer::UpdateParameterSliders()
{
    std::vector<std::pair<int, double>> fixed_parameters =
            parameter_config_model_->GetFixedParameters();
    int x_param = parameter_config_model_->GetXParameter();
    int y_param = parameter_config_model_->GetYParameter();
    fixed_parameters.erase(
        std::remove_if(fixed_parameters.begin(),
                       fixed_parameters.end(),
                       [&](const std::pair<int, double>& p)
                       {
                           return (p.first == x_param || p.first == y_param);
                       }),
        fixed_parameters.end());
    ClearSliderLayout();
    slider_map_.clear();
    for (const auto& param : fixed_parameters)
    {
        Chi2ParameterSlider* slider = new Chi2ParameterSlider(
                QString::fromStdString(parameter_names_.at(param.first)),
                param.first);
        slider->SetInterval(
                parameter_config_model_->GetParameterInterval(param.first));
        slider->SetValue(parameter_config_model_->GetValue(param.first));
        ui->inner_slider_layout->addWidget(slider);
        slider_map_[param.first] = slider;
        connect(slider, SIGNAL(ValueChanged(int,double)),
                parameter_config_model_, SLOT(SetValue(int,double)));
        connect(parameter_config_model_, SIGNAL(FixedParameterValuesChanged()),
                this, SLOT(UpdateParameterSliderValues()));
    }
}

void Chi2Explorer::UpdateParameterSliderValues()
{
    for (const auto& slider : slider_map_)
        slider.second->SetValue(
                parameter_config_model_->GetValue(slider.first));
}

void Chi2Explorer::ClearSliderLayout()
{
    QLayoutItem *child;
    while ((child = ui->inner_slider_layout->takeAt(0)))
    {
        delete child->widget();
        delete child;
    }
}
