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
#include <QGridLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include <algorithm>

#include "core/models/modelmanager.h"

#include "commons.h"
#include "concentrationsheaderview.h"
#include "fitsetup.h"
#include "montecarlosummaryproxymodel.h"
#include "parametercheckbox.h"

#include "fitsetupwidget.h"
#include "ui_fitsetupwidget.h"

FitSetupWidget::FitSetupWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FitSetupWidget),
    double_validator_(new QDoubleValidator(this)),
    fit_setup_(nullptr),
    parameter_line_edits_(),
    parameter_from_table_checkboxes_(),
    gas_checkbox_lookup_()
{
    ui->setupUi(this);
    InitializeModelBox();
    SetupNobleGasCheckBoxes();

    FitSetup* fit_setup = new FitSetup(this);
    SetFitSetup(fit_setup);

    ui->concentrations_view->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* action_paste = new QAction("&Load samples from clipboard", this);
    action_paste->setShortcut(QKeySequence::Paste);
    action_paste->setShortcutContext(Qt::WidgetShortcut);
    ui->concentrations_view->addAction(action_paste);
    connect(action_paste, SIGNAL(triggered()),
            this, SLOT(LoadSamplesFromClipboard()));

    // Sorgt dafür, dass die ScrollArea in x-Richtung die passende Breite erhält
    // und nur in y-Richtung scrollt.
    ui->scroll_area_vertical_layout->installEventFilter(this);

    setAttribute(Qt::WA_DeleteOnClose);
}

FitSetupWidget::~FitSetupWidget()
{
    ClearAndDeleteLayout(ui->initials_group_box->layout());
    ClearAndDeleteLayout(ui->values_group_box->layout());
    delete ui;
}

bool FitSetupWidget::eventFilter(QObject* o, QEvent* e)
{
    if (o == ui->scroll_area_vertical_layout && e->type() == QEvent::Resize)
        ui->scroll_area->setMinimumWidth(
                ui->scroll_area_vertical_layout->minimumSizeHint().width() +
                ui->scroll_area->verticalScrollBar()->width());

    return false;
}

void FitSetupWidget::SetFitSetup(FitSetup* fit_setup)
{
    if (fit_setup_ && fit_setup_->parent() == this)
        delete fit_setup_;
    fit_setup_ = fit_setup;

    LinkModelsAndViews();

    UpdateGasesInUse();
    int model_index = ui->model_combo_box->findText(fit_setup_->GetModelName());
    {
        SignalBlocker blocker(ui->model_combo_box);
        ui->model_combo_box->setCurrentIndex(model_index);
    }
    UpdateParameters();
    UpdateConstrainedFitCheckbox();
    ui->n_monte_carlos_spin_box->setValue(fit_setup_->GetNumberOfMonteCarlos());

    connect(fit_setup_->GetConcentrationsModel(), SIGNAL(modelReset()),
            ui->concentrations_view, SLOT(resizeColumnsToContents()));
    connect(fit_setup_->GetConcentrationsModel(), SIGNAL(modelReset()),
            ui->concentrations_view, SLOT(resizeRowsToContents()));
    connect(fit_setup_->GetParameterSetupModel(), SIGNAL(LineNumberChanged()),
            ui->parameter_setup_view, SLOT(resizeRowsToContents()));
    connect(fit_setup_, SIGNAL(ModelChanged()),
            this, SLOT(UpdateParameters()));
    connect(fit_setup_, SIGNAL(ModelChanged()),
            this, SLOT(UpdateConstrainedFitCheckbox()));
    connect(ui->n_monte_carlos_spin_box, SIGNAL(valueChanged(int)),
            fit_setup_, SLOT(SetNumberOfMonteCarlos(int)));
    connect(ui->constrained_fit_checkbox, SIGNAL(toggled(bool)),
            fit_setup_, SLOT(SetApplyConstraints(bool)));

    //ui->name_lineedit kann hier schon verbunden sein mit this->EmitNameChanged
    //da es immer erst nach fit_setup_->SetName ausgeführt werden soll, wird es
    //an dieser Stelle immer dann verbunden, wenn fit_setup_ geändert wird.
    disconnect(ui->name_lineedit, 0, 0, 0);
    connect(ui->name_lineedit, SIGNAL(textChanged(QString)),
            fit_setup_, SLOT(SetName(QString)));
    connect(ui->name_lineedit, SIGNAL(textChanged(QString)),
            this, SLOT(EmitNameChanged(QString)));
    ui->name_lineedit->setText(fit_setup_->GetName());
}

void FitSetupWidget::AddButton(QPushButton* button)
{
    ui->scroll_area_vertical_layout->layout()->addWidget(button);
}

void FitSetupWidget::MakeImmutable()
{
    ui->noble_gases_label->setText("Gases used:");
    ui->noble_gases_group_box->setEnabled(false);

    ui->model_combo_box->setEnabled(false);

    ui->parameters_to_fit_label->setText("Parameters fitted:");
    ui->parameters_group_box->setEnabled(false);

    ui->initials_group_box->setEnabled(false);
    ui->values_group_box->setEnabled(false);

    ui->constrained_fit_checkbox->setEnabled(false);
    ui->n_monte_carlos_spin_box->setEnabled(false);

    ConcentrationsHeaderView* concentrations_header =
            qobject_cast<ConcentrationsHeaderView*>(
                ui->concentrations_view->horizontalHeader());
    if (concentrations_header)
        concentrations_header->MakeImmutable();
    fit_setup_->GetConcentrationsModel()->MakeImmutable();
    fit_setup_->GetParameterSetupModel()->MakeImmutable();
}

void FitSetupWidget::LinkModelsAndViews()
{
    ui->concentrations_view->setModel(fit_setup_->GetConcentrationsModel());
    ui->concentrations_view->resizeColumnsToContents();
    ui->concentrations_view->resizeRowsToContents();
    ui->parameter_setup_view->setModel(fit_setup_->GetParameterSetupModel());
    ui->parameter_setup_view->resizeRowsToContents();
}

void FitSetupWidget::InitializeModelBox()
{
    std::vector<std::string> available_models =
            ModelManager::Get().GetAvailableModels();

    if (available_models.empty())
    {
        //! \todo Irgendwie sicherstellen, dass die Fitknöpfe auch deaktiviert sind.
//         ui->standard_fit_button->setEnabled(false);
//         ui->ensemble_fit_button->setEnabled(false);
        ui->parameters_group_box->setVisible(false);
        ui->initials_group_box->setVisible(false);
        ui->values_group_box->setVisible(false);
        ui->parameters_to_fit_label->setVisible(false);
        ui->initials_label->setVisible(false);
        ui->values_label->setVisible(false);
        QMessageBox::critical(this, "Error", "Could not find any models");
        return;
    }

    ui->model_combo_box->clear();
    for (const auto& model : available_models)
        ui->model_combo_box->addItem(QString::fromStdString(model));

    connect(ui->model_combo_box, SIGNAL(currentIndexChanged(int)),
            this, SLOT(SetActiveModel()));
}

void FitSetupWidget::SetupNobleGasCheckBoxes()
{
    gas_checkbox_lookup_[Gas::HE] = ui->he_check_box;
    gas_checkbox_lookup_[Gas::NE] = ui->ne_check_box;
    gas_checkbox_lookup_[Gas::AR] = ui->ar_check_box;
    gas_checkbox_lookup_[Gas::KR] = ui->kr_check_box;
    gas_checkbox_lookup_[Gas::XE] = ui->xe_check_box;

    for (const auto& item : gas_checkbox_lookup_)
        connect(item.second, SIGNAL(toggled(bool)),
                this, SLOT(ChangeGasInUse()));
}

void FitSetupWidget::UpdateGasesInUse()
{
    const std::map<GasType, bool>& gases_in_use = fit_setup_->GetGasesInUse();
    for (const auto& gas : gases_in_use)
    {
        QCheckBox* checkbox = gas_checkbox_lookup_.at(gas.first);
        SignalBlocker blocker(checkbox);
        checkbox->setChecked(gas.second);
    }
}

void FitSetupWidget::ChangeGasInUse()
{
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    auto it = std::find_if(gas_checkbox_lookup_.cbegin(),
                           gas_checkbox_lookup_.cend(),
                           [&](const std::pair<GasType, QCheckBox*>& item)
                           {
                               return item.second == checkbox;
                           });
    if (it == gas_checkbox_lookup_.cend()) return;

    fit_setup_->SetGasInUse(it->first, checkbox->isChecked());
}

void FitSetupWidget::ChangeParameterValue()
{
    QLineEdit* lineedit = qobject_cast<QLineEdit*>(sender());
    auto it = std::find_if(parameter_line_edits_.cbegin(),
                           parameter_line_edits_.cend(),
                           [&](const std::pair<std::string, QLineEdit*>& item)
                           {
                               return item.second == lineedit;
                           });
    if (it == parameter_line_edits_.cend()) return;

    fit_setup_->SetParameterValue(it->first, lineedit->text().toDouble());
}

void FitSetupWidget::ChangeParameterIndividuallyConfigured()
{
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    auto it = std::find_if(parameter_from_table_checkboxes_.cbegin(),
                           parameter_from_table_checkboxes_.cend(),
                           [&](const std::pair<std::string, QCheckBox*>& item)
                           {
                               return item.second == checkbox;
                           });
    if (it == parameter_from_table_checkboxes_.cend()) return;

    fit_setup_->SetParameterIndividuallyConfigured(it->first,
                                                   checkbox->isChecked());
}

void FitSetupWidget::EmitNameChanged(QString name)
{
    emit NameChanged(name);
}

void FitSetupWidget::SetActiveModel()
{
    fit_setup_->SetModel(ui->model_combo_box->currentText());
}

void FitSetupWidget::UpdateParameters()
{
    ClearAndDeleteLayout(ui->parameters_group_box->layout());

    if (!fit_setup_->HasModel())
        return;

    std::vector<ModelParameter> parameters =
            fit_setup_->GetParametersInOrder();

    QVBoxLayout* layout = new QVBoxLayout();

    for (const auto& parameter : parameters)
    {
        ParameterCheckBox* checkbox =
                new ParameterCheckBox(parameter, true);
        layout->addWidget(checkbox);
        checkbox->setChecked(fit_setup_->GetParameterFitState(parameter.name));
        connect(checkbox, SIGNAL(toggled(bool)),
                this, SLOT(UpdateParameterFitStatus()));
        connect(checkbox, SIGNAL(toggled(bool)),
                this, SLOT(UpdateInitialsAndValues()));
    }

    ui->parameters_group_box->setLayout(layout);

    InitializeParameterInitialsAndValues();
    UpdateInitialsAndValues();
}

void FitSetupWidget::ClearAndDeleteLayout(QLayout* layout)
{
    if (!layout)
        return;

    QLayoutItem* item;
    while ((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }
    delete layout;
}

void FitSetupWidget::ClearLayout(QLayout* layout)
{
    if (!layout)
        return;

    QLayoutItem* item;
    while ((item = layout->takeAt(0)))
    {
        item->widget()->hide();
        delete item;
    }
}

void FitSetupWidget::InitializeParameterInitialsAndValues()
{
    ClearAndDeleteLayout(ui->initials_group_box->layout());
    QGridLayout* initials_layout = new QGridLayout();
    ui->initials_group_box->setLayout(initials_layout);

    ClearAndDeleteLayout(ui->values_group_box->layout());
    QGridLayout* values_layout = new QGridLayout();
    ui->values_group_box->setLayout(values_layout);

    // Die Widgets sind hier schon von ClearAndDeleteLayout gelöscht worden.
    parameter_line_edits_.clear();
    parameter_from_table_checkboxes_.clear();
    if (fit_setup_->HasModel())
    {
        std::vector<ModelParameter> parameters =
                fit_setup_->GetParametersInOrder();
        for (const auto& parameter : parameters)
        {
            QLineEdit* lineedit = new QLineEdit();
            lineedit->setValidator(double_validator_);
            lineedit->setText(QString::number(
                    fit_setup_->GetParameterValue(parameter.name)));
            lineedit->setEnabled(!fit_setup_->IsParameterIndividuallyConfigured(
                    parameter.name));
            parameter_line_edits_[parameter.name] = lineedit;
            ParameterCheckBox* checkbox =
                    new ParameterCheckBox(parameter, false);
            {
                SignalBlocker blocker(checkbox);
                checkbox->setChecked(fit_setup_->
                          IsParameterIndividuallyConfigured(parameter.name));
            }
            parameter_from_table_checkboxes_[parameter.name] = checkbox;

            connect(lineedit, SIGNAL(textChanged(QString)),
                    this, SLOT(ChangeParameterValue()));
            connect(checkbox, SIGNAL(toggled(bool)),
                    this, SLOT(ChangeParameterIndividuallyConfigured()));
            connect(checkbox, SIGNAL(ParameterEnabled(QString)),
                    fit_setup_->GetParameterSetupModel(),
                            SLOT(AddParameter(QString)));
            connect(checkbox, SIGNAL(ParameterDisabled(QString)),
                    fit_setup_->GetParameterSetupModel(),
                            SLOT(RemoveParameter(QString)));
            connect(checkbox, SIGNAL(ToggledInverse(bool)),
                    lineedit, SLOT(setEnabled(bool)));
        }
    }
}

void FitSetupWidget::UpdateInitialsAndValues()
{
    ClearLayout(ui->initials_group_box->layout());
    ClearLayout(ui->values_group_box->layout());

    for (const auto& parameter : fit_setup_->GetParameterFitStates())
    {
        QGridLayout* layout = dynamic_cast<QGridLayout*>(
                parameter.second ?
                ui->initials_group_box->layout() :
                ui->values_group_box  ->layout());
        int row_count = layout->rowCount();
        layout->addWidget(new QLabel(QString::fromStdString(parameter.first)),
                          row_count, 0);
        layout->addWidget(parameter_line_edits_[parameter.first],
                          row_count, 1);
        layout->addWidget(parameter_from_table_checkboxes_[parameter.first],
                          row_count, 2);
        parameter_line_edits_[parameter.first]->show();
        parameter_from_table_checkboxes_[parameter.first]->show();
    }

    if (ui->initials_group_box->layout()->count())
    {
        ui->initials_group_box->show();
        ui->initials_label->show();
    }
    else
    {
        ui->initials_group_box->hide();
        ui->initials_label->hide();
    }
    if (ui->values_group_box->layout()->count())
    {
        ui->values_group_box->show();
        ui->values_label->show();
    }
    else
    {
        ui->values_group_box->hide();
        ui->values_label->hide();
    }
}

void FitSetupWidget::UpdateParameterFitStatus()
{
    ParameterCheckBox* checkbox = dynamic_cast<ParameterCheckBox*>(sender());
    if (!checkbox)
        return;

    fit_setup_->SetParameterFitState(checkbox->GetParameterName(),
                                     checkbox->isChecked());
}

void FitSetupWidget::UpdateConstrainedFitCheckbox()
{
    ui->constrained_fit_checkbox->
            setChecked(fit_setup_->AreConstraintsApplied());
}

void FitSetupWidget::LoadSamplesFromFile()
{
    fit_setup_->LoadSamplesFromFile();
}

void FitSetupWidget::LoadSamplesFromClipboard()
{
    fit_setup_->LoadSamplesFromClipboard();
}