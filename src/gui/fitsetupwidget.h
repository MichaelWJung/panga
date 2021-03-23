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


#ifndef FITSETUPWIDGET_H
#define FITSETUPWIDGET_H

#include <QWidget>

#include <map>

#include "core/misc/gas.h"

namespace Ui {
    class FitSetupWidget;
}

class FitSetup;
class QCheckBox;
class QDoubleValidator;
class QLineEdit;
class QPushButton;

class FitSetupWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit FitSetupWidget(QWidget* parent = nullptr);
    ~FitSetupWidget();
    void SetFitSetup(FitSetup* fit_setup);
    void AddButton(QPushButton* button);
    void MakeImmutable();
    
public slots:
    void LoadSamplesFromFile();
    void LoadSamplesFromClipboard();
    
signals:
    void NameChanged(QString name);
    
protected:
    virtual bool eventFilter(QObject* o, QEvent* e);
    
private:
    void LinkModelsAndViews();
    void InitializeModelBox();
    void InitializeSolubilityBox();
    void SetupNobleGasCheckBoxes();
    
    //! Entfernt und zerstört alle Widgets des Layouts sowie das Layout selbst.
    void ClearAndDeleteLayout(QLayout* layout);
    
    //! Entfernt die Widgets eines Layouts, löscht sie jedoch nicht.
    /*!
     * Die Widgets werden beim Entfernen versteckt.
     * \warning Wenn nirgendwo sonst Pointer auf diese Widgets existieren könnte
     * es zu einem Memory leak kommen.
     */
    void ClearLayout(QLayout* layout);
    void InitializeParameterInitialsAndValues();
    void UpdateGasesInUse();

private slots:
    void ChangeGasInUse();
    void ChangeParameterValue();
    void ChangeParameterIndividuallyConfigured();
    void EmitNameChanged(QString name);
    void SetActiveModel();
    void UpdateParameters();
    void UpdateInitialsAndValues();
    void UpdateParameterFitStatus();
    void UpdateConstrainedFitCheckbox();

private:
    Ui::FitSetupWidget* ui;
    QDoubleValidator* double_validator_;
    FitSetup* fit_setup_;
    
    //! Enthält Zeiger auf die QLineEdit-Widgets.
    /*!
     * Diese sind hier nach Parameternamen geordnet.
     * Die Widgets befinden sich immer entweder in ui->initials_group_box oder
     * in ui->values_group_box. Sie werden von diesen auch wieder gelöscht.
     */
    std::map<std::string, QLineEdit*> parameter_line_edits_;
    
    //! Enthält Zeiger auf die Checkbox-Widgets.
    /*!
     * Diese geben an, ob Parameter-Start- oder -Fix-Werte individuell, für jede
     * Probe, festgelegt werden, oder gemeinsam (in den QLineEdit-Widgets).
     */
    std::map<std::string, QCheckBox*> parameter_from_table_checkboxes_;
    std::map<GasType, QCheckBox*> gas_checkbox_lookup_;
};

#endif // FITSETUPWIDGET_H
