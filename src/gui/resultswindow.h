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


#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QMainWindow>

#include <memory>

class FitSetup;
class MainWindow;
class MonteCarloResultsModel;
class StandardFitResultsModel;
class Filter1DPlotProxyModel;
class MonteCarloSummaryProxyModel;

namespace Ui {
    class ResultsWindow;
}

class ResultsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResultsWindow();
    ~ResultsWindow();
    virtual void closeEvent(QCloseEvent* event);

    void SetupResultsModelsAndViews(StandardFitResultsModel* results_model,
                                    MonteCarloResultsModel* monte_carlo_model,
                                    const FitSetup* fit_setup);
    void SetFileName(QString file_name, bool is_binary);

    FitSetup* GetCopyOfFitSetup() const;

signals:
    void FitSetupTakeOverRequested();

private slots:
    void OpenMonteCarloSetupDialog();
    void ExportMonteCarloData();
    bool Save();
    bool SaveAs();
    void RequestFitSetupTakeOver();
    void SetWindowModified();
    void UpdateWindowTitle();

private:
    void InitializeFitSetupWidget();
    void FlushWindowFilePath();

    Ui::ResultsWindow* ui;

    MonteCarloSummaryProxyModel* mc_summary_proxy_model_;

    StandardFitResultsModel* results_model_;
    MonteCarloResultsModel* monte_carlo_model_;
    FitSetup* fit_setup_;

    unsigned n_monte_carlos_;

    bool is_binary_;
};

#endif // RESULTSWINDOW_H
