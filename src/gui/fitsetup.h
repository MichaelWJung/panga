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


#ifndef FITSETUP_H
#define FITSETUP_H

#include <QString>

#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>

#include <algorithm>
#include <map>

#include "core/fitting/fitconfiguration.h"
#include "core/misc/gas.h"
#include "core/misc/rundata.h"
#include "core/models/combinedmodel.h"

#include "concentrationsmodel.h"
#include "guiresultsprocessor.h"
#include "parametersetupmodel.h"

class FitSetup : public QObject
{
    Q_OBJECT
    
public:
    FitSetup(QWidget* parent = nullptr);
    FitSetup(const FitSetup& other);
    //! \brief Zuweisungsoperator. Parent der Klasse bleibt unverändert und wird
    //! auch für ConcentrationsModel und ParameterSetupModel verwendet.
    FitSetup& operator=(const FitSetup& other);
    
    void setParent(QWidget* parent);
        
    ConcentrationsModel* GetConcentrationsModel() const;
    ParameterSetupModel* GetParameterSetupModel() const;
    
    unsigned GetNumberOfMonteCarlos() const;
    
    const std::map<GasType, bool>& GetGasesInUse() const;
    void SetGasInUse(GasType gas, bool use);
    
    std::vector<ModelParameter> GetParametersInOrder() const;
    std::vector<std::string> GetFittedParametersInOrder() const;
    
    bool GetParameterFitState(const std::string& parameter_name) const;
    const std::map<std::string, bool>& GetParameterFitStates() const;
    void SetParameterFitState(const std::string& parameter_name, bool fit);
    
    double GetParameterValue(const std::string& parameter_name) const;
    const std::map<std::string, double>& GetParameterValues() const;
    void SetParameterValue(const std::string& parameter_name, double value);
    
    bool IsParameterIndividuallyConfigured(
            const std::string& parameter_name) const;
    const std::set<std::string>& GetIndividuallyConfiguredParameters() const;
    void SetParameterIndividuallyConfigured(const std::string& parameter_name,
                                            bool individually_configured);

    bool HasModel() const;
    void SetModel(QString name, QString ceqmethodname);
    QString GetModelName() const;
    QString GetCEqMethodName() const;
    QString GetName() const;
    bool AreConstraintsApplied() const;
    
public slots:
    void Fit();
    void EnsembleFit();
    void OpenChi2Explorer();
    void LoadSamplesFromFile();
    void LoadSamplesFromClipboard();
    void SetNumberOfMonteCarlos(int n);
    void SetName(QString name);
    void SetApplyConstraints(bool apply);
    
signals:
    void ModelChanged();

private:
    void ConnectSignalsAndSlots();
    void InitializeParameters();
    void ResetModel();
    
    template<class A> void DoForEachUnusedGas(A a) const;
    RunData PrepareGasConcentrations() const;
    std::shared_ptr<GuiResultsProcessor> SetupResultsProcessor();
    std::vector<FitConfiguration> PrepareFitConfigurations() const;
    FitConfiguration PrepareFitConfigurationCommons() const;
    FitConfiguration PrepareEnsembleFitConfiguration();
    void DetermineEnsembleAndIndividuallyFittedParameters(
            std::vector<std::string>& ensemble_fitted,
            std::vector<std::string>& individually_fitted);
    void RunFitAndShowResults(
            std::shared_ptr<FitResultsProcessor> results_processor_for_fit,
            std::shared_ptr<GuiResultsProcessor> results_processor_for_progress,
            const RunData& concentrations_in_use,
            const std::vector<FitConfiguration> configurations);
    void ShowParameterNotInHeaderError(
            const ParameterSetupModel::ParameterNotInHeaderError& err);

    
    QWidget* parent_;

    ConcentrationsModel* concentrations_model_;
    ParameterSetupModel* parameter_setup_model_;

    const RunData* concentrations_;

    std::map<GasType, bool> gases_in_use_;

    //! Hier wird festgehalten ob ein Parameter gefittet werden soll oder nicht.
    std::map<std::string, bool> parameter_fit_states_;

    //! Parameter-Start- oder -Fix-Werte.
    std::map<std::string, double> parameter_values_;
   
    //! Die Parameter, die für jede Probe einen eigenen Start-/Fix-Wert haben.
    std::set<std::string> individually_configured_parameters_;

    unsigned n_monte_carlos_;
    
    QString name_;

    std::shared_ptr<CombinedModel> combined_model_;
    
    static const QString DEFAULT_NAME;
    
    class DialogCanceled {};
    
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()
    
    template<class Archive>
    void save(Archive& ar, const unsigned version) const;
    
    template<class Archive>
    void load(Archive& ar, const unsigned version);
};

template<class Archive>
void FitSetup::save(Archive& ar, const unsigned version) const
{
    std::string model = combined_model_->GetExcessAirModelName();
    ar << model;
    std::string ceqmethod = combined_model_->GetCEqMethodName();
    ar << ceqmethod;

    bool constraints_applied = AreConstraintsApplied();
    ar << constraints_applied;
    
    ar << concentrations_model_
       << parameter_setup_model_
       << gases_in_use_
       << parameter_fit_states_
       << parameter_values_
       << individually_configured_parameters_
       << n_monte_carlos_
       << name_;
}

template<class Archive>
void FitSetup::load(Archive& ar, const unsigned version)
{              
    std::string model;
    ar >> model;
    std::string ceqmethod;
    ar >> ceqmethod;
    //WIP: Öffnen gespeicherter Ergebnisse aus voriger Version möglich machen

    SetModel(QString::fromStdString(model), QString::fromStdString(ceqmethod)); 
    
    if (version >= 2)
    {
        bool apply_constraints;
        ar >> apply_constraints;
        SetApplyConstraints(apply_constraints);
    }
    
    ar >> concentrations_model_
       >> parameter_setup_model_
       >> gases_in_use_
       >> parameter_fit_states_
       >> parameter_values_
       >> individually_configured_parameters_
       >> n_monte_carlos_;
       
    if (version >= 1)
        ar >> name_;
    else
        name_ = DEFAULT_NAME;
    
    concentrations_ = concentrations_model_->GetRunData();
    concentrations_model_->setParent(this);
    parameter_setup_model_->setParent(this);
    ConnectSignalsAndSlots();
}

BOOST_CLASS_VERSION(FitSetup, 2)

#endif // FITSETUP_H
