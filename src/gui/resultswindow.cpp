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


#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/codecvt_null.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/math/special_functions/nonfinite_num_facets.hpp>

#include <algorithm>
#include <cassert>

#include "commons.h"
#include "fitsetup.h"
#include "fitsetupwidget.h"
#include "histogramdata1d.h"
#include "histogramdata2d.h"
#include "histogramplot.h"
#include "mainwindow.h"
#include "montecarloexportercsv.h"
#include "montecarloplotdelegate.h"
#include "montecarloresultsmodel.h"
#include "montecarlosummaryproxymodel.h"
#include "setupmontecarloplotsdialog.h"
#include "standardfitresultsmodel.h"

#include "resultswindow.h"
#include "ui_resultswindow.h"

ResultsWindow::ResultsWindow() :
    QMainWindow(nullptr),
    ui(new Ui::ResultsWindow),
    results_model_(nullptr),
    monte_carlo_model_(nullptr),
    fit_setup_(new FitSetup(this)),
    n_monte_carlos_(0U)
{
    MainWindow::theMainWindow->AddWindowToOpenWindowsVector(this);
    ui->setupUi(this);

    ui->monte_carlo_plots_view->SetResultsWindow(this);
    ui->monte_carlo_plots_view->setItemDelegate(
            new MonteCarloPlotDelegate(ui->monte_carlo_plots_view));
    mc_summary_proxy_model_ = new MonteCarloSummaryProxyModel(
            ui->monte_carlo_summary_view);

    // ist diese Zeile unnötig weil sie auch in
    // SetupResultsModelsAndViews ist??
    ui->monte_carlo_summary_view->setModel(mc_summary_proxy_model_);

    ui->results_label_recommending_monte_carlos->setVisible(false);
    ui->results_label_recommending_monte_carlos
            ->setStyleSheet("QLabel { color : red; }");

    ui->action_save->setShortcut(QKeySequence::Save);
    ui->action_save_as->setShortcut(QKeySequence::SaveAs);

    connect(ui->bin_number_spin_box, SIGNAL(valueChanged(int)),
            ui->bin_number_slider, SLOT(setValue(int)));
    connect(ui->bin_number_slider, SIGNAL(valueChangedLog(int)),
            ui->bin_number_spin_box, SLOT(setValue(int)));
    connect(ui->bin_number_spin_box, SIGNAL(valueChanged(int)),
            this, SLOT(SetWindowModified()));
    connect(ui->action_choose_monte_carlo_plots, SIGNAL(triggered()),
            this, SLOT(OpenMonteCarloSetupDialog()));
    connect(ui->action_export_monte_carlo_data, SIGNAL(triggered()),
            this, SLOT(ExportMonteCarloData()));
    connect(ui->action_save, SIGNAL(triggered()),
            this, SLOT(Save()));
    connect(ui->action_save_as, SIGNAL(triggered()),
            this, SLOT(SaveAs()));

    setAttribute(Qt::WA_DeleteOnClose);
}

ResultsWindow::~ResultsWindow()
{
    delete ui;
}

void ResultsWindow::closeEvent(QCloseEvent* event)
{
    if (isWindowModified())
    {
        activateWindow();
        raise();
        QMessageBox::StandardButton button = QMessageBox::warning(
                this,
                APPLICATION_NAME,
                "Warning: data have been modified. Do you want to save the "
                    "changes?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Abort,
                QMessageBox::Abort);
        switch (button)
        {
            case QMessageBox::Save:
                if (!Save())
                {
                    event->ignore();
                    return;
                }
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Abort:
            default:
                event->ignore();
                return;
        }
    }
    event->accept();
}

void ResultsWindow::SetupResultsModelsAndViews(
        StandardFitResultsModel* results_model,
        MonteCarloResultsModel* monte_carlo_model,
        const FitSetup* fit_setup)
{
    results_model_ = results_model;
    monte_carlo_model_ = monte_carlo_model;
    *fit_setup_ = *fit_setup;
    InitializeFitSetupWidget();

    results_model_->setParent(this);
    monte_carlo_model_->setParent(this);
    results_model_->AddUnusedConcentrations(
            *fit_setup_->GetConcentrationsModel()->GetRunData());
    monte_carlo_model_->SetOriginalResults(results_model_->GetResultsVector());
    ui->results_view->setModel(results_model_);
    ui->monte_carlo_plots_view->setModel(monte_carlo_model_);
    n_monte_carlos_ = monte_carlo_model_->NumberOfMonteCarlos();
    if (n_monte_carlos_)
    {
        unsigned n_bins;
        {
            SignalBlocker blocker1(ui->bin_number_spin_box);
            SignalBlocker blocker2(ui->bin_number_slider);
            const unsigned max_bin_number =
                    HistogramPlot::CalcMaxBinNumber(n_monte_carlos_);
            ui->bin_number_spin_box->setMinimum(1);
            ui->bin_number_spin_box->setMaximum(max_bin_number);
            ui->bin_number_slider->setMinimum(1);
            ui->bin_number_slider->setMaximum(max_bin_number);
            n_bins = monte_carlo_model_->NumberOfBins() ?
                     monte_carlo_model_->NumberOfBins() :
                     std::sqrt(n_monte_carlos_);
            ui->bin_number_spin_box->setValue(n_bins);
            ui->bin_number_slider->setValue(n_bins);
        }
        SignalBlocker blocker(monte_carlo_model_);
        monte_carlo_model_->SetNumberOfBins(n_bins);
        connect(ui->bin_number_spin_box, SIGNAL(valueChanged(int)),
                monte_carlo_model_, SLOT(SetNumberOfBins(int)));
        connect(monte_carlo_model_, SIGNAL(layoutChanged()),
                ui->monte_carlo_plots_view, SLOT(resizeColumnsToContents()));
        connect(mc_summary_proxy_model_, SIGNAL(layoutChanged()),
                ui->monte_carlo_summary_view, SLOT(resizeColumnsToContents()));
    }
    ui->tab_widget->setTabEnabled(1, n_monte_carlos_);
    ui->tab_widget->setTabEnabled(2, n_monte_carlos_);
    ui->action_choose_monte_carlo_plots->setEnabled(n_monte_carlos_);
    ui->action_export_monte_carlo_data ->setEnabled(n_monte_carlos_);

    mc_summary_proxy_model_->SetSourceModel(monte_carlo_model_);

    ui->results_view->resizeColumnsToContents();
    ui->results_view->resizeRowsToContents();
    ui->monte_carlo_plots_view->resizeColumnsToContents();
    ui->monte_carlo_plots_view->resizeRowsToContents();
    ui->monte_carlo_summary_view->resizeColumnsToContents();
    ui->monte_carlo_summary_view->resizeRowsToContents();

    ui->results_label_recommending_monte_carlos->setVisible(
            results_model_->DoesAnySampleNeedMonteCarlo());;

    UpdateWindowTitle();
}

void ResultsWindow::SetFileName(QString file_name, bool is_binary)
{
    setWindowFilePath(file_name);
    is_binary_ = is_binary;
    UpdateWindowTitle();
}

FitSetup* ResultsWindow::GetCopyOfFitSetup() const
{
    return new FitSetup(*fit_setup_);
}

void ResultsWindow::InitializeFitSetupWidget()
{
    ui->fit_setup_widget->SetFitSetup(fit_setup_);
    ui->fit_setup_widget->MakeImmutable();

    QPushButton* button = new QPushButton("&Load into Main Window");
    ui->fit_setup_widget->AddButton(button);
    connect(button, SIGNAL(clicked(bool)),
            this, SLOT(RequestFitSetupTakeOver()));
    connect(this, SIGNAL(FitSetupTakeOverRequested()),
            MainWindow::theMainWindow, SLOT(TakeOverFitSetup()));
    connect(ui->fit_setup_widget, SIGNAL(NameChanged(QString)),
            this, SLOT(SetWindowModified()));
    connect(ui->fit_setup_widget, SIGNAL(NameChanged(QString)),
            this, SLOT(UpdateWindowTitle()));

    UpdateWindowTitle();
}

void ResultsWindow::OpenMonteCarloSetupDialog()
{
    ui->monte_carlo_plots_view->CloseAllEditors();
    SetupMonteCarloPlotsDialog* setup_dialog(
            new SetupMonteCarloPlotsDialog(*monte_carlo_model_,
                                           this));
    connect(setup_dialog, SIGNAL(accepted(QList<ColumnTypeVariant>)),
            monte_carlo_model_, SLOT(SetColumnTypes(QList<ColumnTypeVariant>)));
    setup_dialog->setWindowModality(Qt::WindowModal);
    setup_dialog->show();
}

void ResultsWindow::ExportMonteCarloData()
{
    try
    {
        MonteCarloExporterCsv exporter;
        monte_carlo_model_->ExportMonteCarloData(
                exporter,
                results_model_->GetAvailableColumnTypes());
    }
    catch (MonteCarloExporterCsv::Canceled)
    {
    }
}

bool ResultsWindow::Save()
{
    if (windowFilePath().isEmpty())
        return SaveAs();

    bool saved = false;
    try
    {
        GlobalWaitCursor wait_cursor;
        if (is_binary_)
        {
            std::ofstream ofs(windowFilePath().toStdString(),
                              std::ios_base::binary);
            ofs.exceptions(std::iostream::badbit  |
                           std::iostream::failbit);
            boost::archive::binary_oarchive oa(ofs);
            oa << results_model_
               << monte_carlo_model_
               << fit_setup_;
        }
        else
        {
            // Die geänderten Locales sind notwendig um Inf und NaN
            // zuverlässig und platformunabhängig zu speichern.
            std::locale default_locale(
                    std::locale::classic(),
                    new boost::archive::codecvt_null<char>);
            std::locale out_locale(
                    default_locale,
                    new boost::math::nonfinite_num_put<char>);
            std::ofstream ofs(windowFilePath().toStdString(),
                              std::ios_base::binary);
            ofs.exceptions(std::iostream::badbit  |
                           std::iostream::failbit);
            boost::iostreams::filtering_ostream f;
#ifndef __APPLE__
            // Das Komprimieren führt unter OSX oft zu fehlerhaften Dateien.
            // Daher wird es nur auf den anderen Systemen verwendet.
            // Die Portabilität wird dadurch nicht eingeschränkt, da jedes System
            // komprimierte und unkomprimierte Dateien einlesen kann.
            f.push(boost::iostreams::zlib_compressor());
#endif
            f.push(ofs);
            f.imbue(out_locale);
            boost::archive::text_oarchive oa(f, boost::archive::no_codecvt);
            oa << results_model_
               << monte_carlo_model_
               << fit_setup_;
        }
        saved = true;
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this,
                              APPLICATION_NAME,
                              QString("Error: ") + e.what());
    }
    catch (...)
    {
        QMessageBox::critical(this,
                              APPLICATION_NAME,
                              QString("Error: unknown error"));
    }
    setWindowModified(!saved);
    return saved;
}

bool ResultsWindow::SaveAs()
{
    QString filter_binary(APPLICATION_NAME + " binary file (*." +
                          BINARY_FILE_EXTENSION + ")");
    QString filter_text(APPLICATION_NAME + " portable file (*." +
                        PORTABLE_FILE_EXTENSION + ")");
    QString selected_filter;
    QString filename = QFileDialog::getSaveFileName(
            this,
            "Save results as",
            QString(),
            filter_binary + ";;" + filter_text,
            &selected_filter
            );

    if (filename.isEmpty()) return false;

    if (selected_filter != filter_binary && selected_filter != filter_text)
    {
        QMessageBox::critical(this, APPLICATION_NAME,
                              "Error: Please select a file type.");
        return false;
    }

    SetFileName(filename, selected_filter == filter_binary);
    bool saved = Save();
    FlushWindowFilePath();
    return saved;
}

void ResultsWindow::RequestFitSetupTakeOver()
{
    emit FitSetupTakeOverRequested();
}

void ResultsWindow::SetWindowModified()
{
    setWindowModified(true);
}

void ResultsWindow::UpdateWindowTitle()
{
    auto parameter_names = fit_setup_->GetFittedParametersInOrder();

    QString joined_names;
    std::for_each(parameter_names.begin(),
                  parameter_names.end(),
                  [&](const std::string& str)
                  {
                      joined_names += QString::fromStdString(str) + ",";
                  });
    joined_names.remove(joined_names.size() - 1, 1);

    QString fit_name = fit_setup_->GetName();
    QString file_name;
    if (windowFilePath().isEmpty())
        file_name = "";
    else
    {
        QFileInfo fi(windowFilePath());
        file_name = " (" + fi.fileName() + ")";
    }

    setWindowTitle(fit_name + file_name + "[*] - " +
                   fit_setup_->GetModelName() + " model " +
                   "(" + joined_names + ") - Fit Results - " +
                   APPLICATION_NAME);
}

void ResultsWindow::FlushWindowFilePath()
{
    QString file_path_cache = windowFilePath();
    setWindowFilePath("");
    setWindowFilePath(file_path_cache);
}
