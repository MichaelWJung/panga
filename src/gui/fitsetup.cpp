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


#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>

#include <boost/lexical_cast.hpp>

#include <memory>
#include <stdexcept>

#include "core/fitting/defaultfitter.h"
#include "core/models/modelmanager.h"

#include "chi2explorer.h"
#include "commons.h"
#include "contourplotdata.h"
#include "ensemblefitconfigurationdialog.h"
#include "ensembleresultsprocessorproxy.h"
#include "fittingthread.h"
#include "resultswindow.h"
#include "montecarloresultsmodel.h"
#include "standardfitresultsmodel.h"

#include "fitsetup.h"

const QString FitSetup::DEFAULT_NAME = QString("unnamed");

FitSetup::FitSetup(QWidget* parent) :
    QObject(parent),
    parent_(parent),
    concentrations_model_(new ConcentrationsModel(this)),
    parameter_setup_model_(new ParameterSetupModel(this)),
    concentrations_(concentrations_model_->GetRunData()),
    gases_in_use_({{Gas::HE, false},
                   {Gas::NE, true},
                   {Gas::AR, true},
                   {Gas::KR, true},
                   {Gas::XE, true}}),
    n_monte_carlos_(0U),
    name_(DEFAULT_NAME)
{
    {
        SignalBlocker blocker(this);
        const auto& models = ModelManager::Get().GetAvailableModels();
        std::string first_model = models.empty() ?
                                  std::string("") :
                                  models.front();
        SetModel(QString::fromStdString(first_model));
    }
    
    ConnectSignalsAndSlots();
}

FitSetup::FitSetup(const FitSetup& other) :
    QObject(other.parent_),
    parent_(other.parent_),
    concentrations_model_(
            new ConcentrationsModel(*other.concentrations_model_)),
    parameter_setup_model_(
            new ParameterSetupModel(*other.parameter_setup_model_)),
    concentrations_(concentrations_model_->GetRunData()),
    gases_in_use_(other.gases_in_use_),
    parameter_fit_states_(other.parameter_fit_states_),
    parameter_values_(other.parameter_values_),
    individually_configured_parameters_(
            other.individually_configured_parameters_),
    n_monte_carlos_(other.n_monte_carlos_),
    name_(other.name_),
    model_(other.model_->clone())
{
    ConnectSignalsAndSlots();
}

FitSetup& FitSetup::operator=(const FitSetup& other)
{
    if (this != &other)
    {
        concentrations_model_ =
                new ConcentrationsModel(*other.concentrations_model_);
        parameter_setup_model_ =
                new ParameterSetupModel(*other.parameter_setup_model_);
        concentrations_model_->setParent(parent_);
        parameter_setup_model_->setParent(parent_);
        concentrations_ = concentrations_model_->GetRunData();
        gases_in_use_ = other.gases_in_use_;
        parameter_fit_states_ = other.parameter_fit_states_;
        parameter_values_ = other.parameter_values_;
        individually_configured_parameters_ =
                other.individually_configured_parameters_;
        n_monte_carlos_ = other.n_monte_carlos_;
        name_ = other.name_;
        model_ = other.model_->clone();
        
        ConnectSignalsAndSlots();
    }
    return *this;
}

void FitSetup::ConnectSignalsAndSlots()
{
    connect(concentrations_model_, 
            SIGNAL(SamplesChanged(std::vector<std::string>)),
            parameter_setup_model_,
            SLOT(SetSampleNames(std::vector<std::string>)));
    connect(concentrations_model_,
            SIGNAL(SampleDisabled(unsigned)),
            parameter_setup_model_,
            SLOT(DisableSample(unsigned)));
    connect(concentrations_model_,
            SIGNAL(SampleEnabled(unsigned)),
            parameter_setup_model_,
            SLOT(EnableSample(unsigned)));
}

void FitSetup::setParent(QWidget* parent)
{
    QObject::setParent(parent);
    parent_ = parent;
    concentrations_model_->setParent(parent);
    parameter_setup_model_->setParent(parent);
}

void FitSetup::Fit()
{
    RunData concentrations_in_use(PrepareGasConcentrations());
    std::vector<FitConfiguration> configurations;
    std::shared_ptr<GuiResultsProcessor> results_processor(
            SetupResultsProcessor());
    try
    {
        configurations = PrepareFitConfigurations();
    }
    catch (ParameterSetupModel::ParameterNotInHeaderError err)
    {
        ShowParameterNotInHeaderError(err);
        return;
    }
    RunFitAndShowResults(
            results_processor,
            results_processor,
            concentrations_in_use,
            configurations);
}

void FitSetup::EnsembleFit()
{
    RunData concentrations_in_use(PrepareGasConcentrations());
    std::vector<FitConfiguration> configurations;
    std::shared_ptr<GuiResultsProcessor> results_processor(
            SetupResultsProcessor());
    std::shared_ptr<EnsembleResultsProcessorProxy> proxy(
            std::make_shared<EnsembleResultsProcessorProxy>(
                GetFittedParametersInOrder(),
                results_processor));
    try
    {
        configurations = {PrepareEnsembleFitConfiguration()};
        RunFitAndShowResults(
                proxy,
                results_processor,
                concentrations_in_use,
                configurations);
    }
    catch (ParameterSetupModel::ParameterNotInHeaderError err)
    {
        ShowParameterNotInHeaderError(err);
    }
    catch (FitSetup::DialogCanceled)
    {
    }
}

void FitSetup::OpenChi2Explorer()
{
    RunData concentrations_in_use(PrepareGasConcentrations());
    std::vector<FitConfiguration> configurations;
    try
    {
        configurations = PrepareFitConfigurations();
    }
    catch (ParameterSetupModel::ParameterNotInHeaderError err)
    {
        ShowParameterNotInHeaderError(err);
        return;
    }
    
    std::vector<std::string> fit_parameters = GetFittedParametersInOrder();
    assert(configurations.size() == concentrations_in_use.EnabledSize());
    if (concentrations_in_use.EnabledSize() < 1)
    {
        QMessageBox::critical(parent_, APPLICATION_NAME,
                              "Error: sample list is empty");
        return;
    }
    if (fit_parameters.size() < 2)
    {
        QMessageBox::critical(
                parent_,
                APPLICATION_NAME,
                "Error: you need to select at least two parameters");
        return;
    }
    
    ContourPlotData* fitter = new ContourPlotData();
    fitter->SetConcentrations(concentrations_in_use);
    fitter->SetFitConfigurations(configurations);
        
    Chi2Explorer* explorer = new Chi2Explorer(
            fitter,
            concentrations_in_use.GetEnabledSampleNames(),
            fit_parameters);
    explorer->show();
}

ConcentrationsModel* FitSetup::GetConcentrationsModel() const
{
    return concentrations_model_;
}

ParameterSetupModel* FitSetup::GetParameterSetupModel() const
{
    return parameter_setup_model_;
}

unsigned int FitSetup::GetNumberOfMonteCarlos() const
{
    return n_monte_carlos_;
}

void FitSetup::SetNumberOfMonteCarlos(int n)
{
    n_monte_carlos_ = n;
}

void FitSetup::SetName(QString name)
{
    name_ = name;
}

const std::map<GasType, bool>& FitSetup::GetGasesInUse() const
{
    return gases_in_use_;
}

void FitSetup::SetGasInUse(GasType gas, bool use)
{
    if (!gases_in_use_.count(gas))
        throw std::invalid_argument("Only HE, NE, AR, KR and XE are "
                                    "valid gases.");
    gases_in_use_[gas] = use;
}

std::vector<ModelParameter> FitSetup::GetParametersInOrder() const
{
    if (!model_)
        throw std::runtime_error("No active model");
    return model_->GetParametersInOrder();
}

bool FitSetup::GetParameterFitState(const std::string& parameter_name) const
{
    return parameter_fit_states_.at(parameter_name);
}

const std::map<std::string, bool>& FitSetup::GetParameterFitStates() const
{
    return parameter_fit_states_;
}

void FitSetup::SetParameterFitState(const std::string& parameter_name, bool fit)
{
    if (!parameter_fit_states_.count(parameter_name))
        throw std::invalid_argument("Invalid parameter name.");
    parameter_fit_states_[parameter_name] = fit;
}

void FitSetup::InitializeParameters()
{
    parameter_fit_states_.clear();
    parameter_values_.clear();
    individually_configured_parameters_.clear();
    
    std::vector<ModelParameter> parameters =
            model_->GetParametersInOrder();
    for (const auto& parameter : parameters)
    {
        parameter_fit_states_[parameter.name] = false;
        parameter_values_[parameter.name] = parameter.default_value;
    }
}

double FitSetup::GetParameterValue(const std::string& parameter_name) const
{
    return parameter_values_.at(parameter_name);
}

const std::map<std::string, double>& FitSetup::GetParameterValues() const
{
    return parameter_values_;
}

void FitSetup::SetParameterValue(const std::string& parameter_name,
                                 double value)
{
    //! \todo Prüfen ob Parameter vorhanden.
    parameter_values_[parameter_name] = value;
}

bool FitSetup::IsParameterIndividuallyConfigured(
        const std::string& parameter_name) const
{
    return individually_configured_parameters_.count(parameter_name);
}

const std::set<std::string>&
FitSetup::GetIndividuallyConfiguredParameters() const
{
    return individually_configured_parameters_;
}

void FitSetup::SetParameterIndividuallyConfigured(
        const std::string& parameter_name,
        bool individually_configured)
{
    if (individually_configured)
        individually_configured_parameters_.insert(parameter_name);
    else
        individually_configured_parameters_.erase(parameter_name);
}

bool FitSetup::HasModel() const
{
    return static_cast<bool>(model_);
}

void FitSetup::ResetModel()
{
    model_.reset();
    emit ModelChanged();
}

void FitSetup::SetModel(QString name)
{
    if (name.isEmpty())
    {
        ResetModel();
        return;
    }
    
    ModelFactory* factory;
    try
    {
        factory = ModelManager::Get().GetModelFactory(name.toStdString());
    }
    catch (ModelManager::ModelNotFoundError)
    {
        ResetModel();
        return;
    }

    ModelFactory* ceqmethod_factory;
    try
    {
        factory = ModelManager::Get().GetModelFactory(name.toStdString());
    }
    catch (ModelManager::ModelNotFoundError)
    {
        ResetModel();
        return;
    }    

    model_ = CombinedModelFactory(factory, ceqmethod_factory).CreateModel();
    InitializeParameters();
    emit ModelChanged();
}

QString FitSetup::GetModelName() const
{
    if (!HasModel())
        return "";
    return QString::fromStdString(model_->GetExcessAirModelName());
}

QString FitSetup::GetName() const
{
    return name_;
}

bool FitSetup::AreConstraintsApplied() const
{
    if (HasModel())
        return model_->AreConstraintsApplied();
    return false;
}

void FitSetup::SetApplyConstraints(bool apply)
{
    if (HasModel())
        model_->SetApplyConstraints(apply);
}

void FitSetup::LoadSamplesFromFile()
{
    concentrations_model_->LoadSamplesFromFile();
}

void FitSetup::LoadSamplesFromClipboard()
{
    concentrations_model_->LoadSamplesFromClipboard();
}

template<class A> void FitSetup::DoForEachUnusedGas(A a) const
{
    std::for_each(gases_in_use_.cbegin(), gases_in_use_.cend(),
            [&](const std::pair<const GasType, bool>& elem)
            {
                if (!elem.second) a(elem.first);
            });
}

RunData FitSetup::PrepareGasConcentrations() const
{
    RunData reduced_concentrations(*concentrations_);
    DoForEachUnusedGas(
            std::bind(&RunData::RemoveGas, &reduced_concentrations, 
                      std::placeholders::_1));
    return reduced_concentrations;
}

std::shared_ptr<GuiResultsProcessor> FitSetup::SetupResultsProcessor()
{
    std::vector<ModelParameter> parameters;
    for (const auto& parameter : model_->GetParametersInOrder())
        if (parameter_fit_states_.at(parameter.name))
            parameters.push_back(parameter);
    
    typedef std::set<GasType> GasSet;
    GasSet gases_in_use = {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE};
    DoForEachUnusedGas(std::bind(
            // Das hier ist einfach nur ein Functor auf GasSet::erase
            static_cast<GasSet::size_type (GasSet::*)(const GasSet::key_type&)>(
                &std::set<GasType>::erase),
            &gases_in_use,
            std::placeholders::_1));
    
    return std::make_shared<GuiResultsProcessor>(
            concentrations_->GetEnabledSampleNames(),
            parameters,
            gases_in_use,
            n_monte_carlos_);
}

std::vector<FitConfiguration> FitSetup::PrepareFitConfigurations() const
{
    FitConfiguration common_fit_config(PrepareFitConfigurationCommons());
    std::vector<FitConfiguration> configurations(
            concentrations_->EnabledSize(),
            common_fit_config);
    
    for (unsigned i = 0; i < concentrations_->EnabledSize(); ++i)
    {
        configurations[i].sample_numbers.push_back(i);
        ModelParameterConfigs& parameters =
                configurations[i].model_parameter_configs.at(0);
        for (const auto& name : individually_configured_parameters_)
        {
            double value = parameter_setup_model_->GetValue(
                    QString::fromStdString(name), i);
            std::replace_if(
                    parameters.begin(), parameters.end(),
                    [&](ModelParameterConfig& p)
                    {
                        return (p.model_parameter == name &&
                                !p.is_fitted);
                    },
                    ModelParameterConfig(name, value));
            try
            {
                configurations[i].fit_parameter_config.ChangeParameterInitial(
                        name, value);
            }
            catch(std::invalid_argument)
            {
            }
        }
    }
        
    return configurations;
}

FitConfiguration FitSetup::PrepareFitConfigurationCommons() const
{
    FitConfiguration config;
    config.model = model_;
    ModelParameterConfigs model_parameters;
    for (const auto& parameter : model_->GetParameterNamesInOrder())
    {
        double value = parameter_values_.at(parameter);
        if (parameter_fit_states_.at(parameter))
        {
            config.fit_parameter_config.AddParameter(
                    FitParameter(parameter, value));
            model_parameters.push_back(ModelParameterConfig(parameter,
                                                            parameter));
        }
        else
            model_parameters.push_back(ModelParameterConfig(parameter, value));
    }
    config.model_parameter_configs = {model_parameters};
    config.n_monte_carlos = n_monte_carlos_;
    
    return config;
}

FitConfiguration FitSetup::PrepareEnsembleFitConfiguration()
{
    std::vector<std::string> ensemble_fitted;
    std::vector<std::string> individually_fitted;
    DetermineEnsembleAndIndividuallyFittedParameters(ensemble_fitted,
                                                     individually_fitted);
    unsigned n_samples = concentrations_->EnabledSize();
    
    FitConfiguration config;
    config.model = model_;
    if (!n_samples) return config;
    config.n_monte_carlos = n_monte_carlos_;
    config.model_parameter_configs.resize(n_samples);
    
    for (const auto& parameter : ensemble_fitted)
    {
        double value = parameter_values_.at(parameter);
        config.fit_parameter_config.AddParameter(
                FitParameter(parameter, value));
    }
    
    for (unsigned i = 0; i < n_samples; ++i)
    {
        config.sample_numbers.push_back(i);
        for (const auto& parameter :
                model_->GetParameterNamesInOrder())
        {
            double value;
            if (individually_configured_parameters_.count(parameter))
                value = parameter_setup_model_->GetValue(
                            QString::fromStdString(parameter), i);
            else
                value = parameter_values_.at(parameter);
            
            if (std::count(individually_fitted.begin(),
                           individually_fitted.end(),
                           parameter))
                config.fit_parameter_config.AddParameter(
                        FitParameter(parameter +
                                         boost::lexical_cast<std::string>(i),
                                     value));
                
            if (parameter_fit_states_.at(parameter))
            {
                if (std::count(ensemble_fitted.begin(),
                               ensemble_fitted.end(),
                               parameter))
                    config.model_parameter_configs.at(i).push_back(
                            ModelParameterConfig(parameter, parameter));
                else
                    config.model_parameter_configs.at(i).push_back(
                            ModelParameterConfig(parameter,
                                parameter + boost::lexical_cast<std::string>(i)));
            }
            else
                config.model_parameter_configs.at(i).push_back(
                        ModelParameterConfig(parameter, value));
        }
    }
    return config;
}

std::vector<std::string> FitSetup::GetFittedParametersInOrder() const
{
    std::vector<std::string> parameters;
    for (const auto& parameter :
            model_->GetParameterNamesInOrder())
        if (parameter_fit_states_.at(parameter))
            parameters.push_back(parameter);
    return parameters;
}

void FitSetup::DetermineEnsembleAndIndividuallyFittedParameters(
        std::vector<std::string>& ensemble_fitted,
        std::vector<std::string>& individually_fitted)
{
    auto fit_parameters = GetFittedParametersInOrder();
    std::map<std::string, bool> parameter_ensemble_states;
    for (auto parameter : fit_parameters)
        parameter_ensemble_states[parameter] = false;
    if (!parameter_ensemble_states.empty())
    {
        EnsembleFitConfigurationDialog dialog(parameter_ensemble_states,
                                              parent_);
        if (dialog.exec() != QDialog::Accepted)
            throw DialogCanceled();
    }
    
    for (const auto& parameter :
            model_->GetParameterNamesInOrder())
    {
        if (!parameter_ensemble_states.count(parameter)) continue;
        if (parameter_ensemble_states[parameter])
            ensemble_fitted.push_back(parameter);
        else
            individually_fitted.push_back(parameter);
    }
}

void FitSetup::RunFitAndShowResults(
        std::shared_ptr<FitResultsProcessor> results_processor_for_fit,
        std::shared_ptr<GuiResultsProcessor> results_processor_for_progress,
        const RunData& concentrations_in_use,
        const std::vector<FitConfiguration> configurations)
{
    WindowWaitCursor wait_cursor(parent_);
    unsigned n_monte_carlos = n_monte_carlos_ * concentrations_->EnabledSize();
    
    QProgressDialog progressdialog(parent_);
    progressdialog.setWindowModality(Qt::WindowModal);
    progressdialog.setLabelText("Fitting samples...");
    progressdialog.setCancelButtonText("&Cancel");
    progressdialog.setRange(0, n_monte_carlos);
    progressdialog.setMinimumDuration(0);
    progressdialog.setValue(0);
    
    DefaultFitter fitter(results_processor_for_fit);
    fitter.SetConcentrations(concentrations_in_use);
    fitter.SetFitConfigurations(configurations);
    
    if (n_monte_carlos)
    {
        FittingThread thread(fitter);
        results_processor_for_progress->moveToThread(&thread);
        connect(results_processor_for_progress.get(),
                SIGNAL(MonteCarloResultProcessed(int)),
                &progressdialog, SLOT(setValue(int)),
                Qt::BlockingQueuedConnection);
        connect(&progressdialog, SIGNAL(canceled()),
                results_processor_for_progress.get(), SLOT(Interrupt()));
        
        thread.start();
        progressdialog.exec();
        while (!thread.wait(50))
            // Events abarbeiten: thread könnte geblockt sein da er darauf
            // wartet, dass progressdialog den setValue-Slot ausführt.
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents |
                                            QEventLoop::ExcludeSocketNotifiers);
        if (!thread.SuccessfullyCompleted()) return;
    }
    else
        fitter.Fit();
    
    ResultsWindow* results_window = new ResultsWindow();
    results_window->SetupResultsModelsAndViews(
            qobject_cast<StandardFitResultsModel*>(
                results_processor_for_progress->GetResultsModel()),
            qobject_cast<MonteCarloResultsModel*>(
                results_processor_for_progress->GetMonteCarloResultsModel()),
            this);
    results_window->setWindowModified(true);
    results_window->show();
}

void FitSetup::ShowParameterNotInHeaderError(
        const ParameterSetupModel::ParameterNotInHeaderError& err)
{
    QMessageBox::warning(parent_, APPLICATION_NAME, 
                         "Parameter " + err.parameter_name + " is setup to "
                         "have individual values but could not be found "
                         "in the table.");
}
