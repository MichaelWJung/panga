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


#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/codecvt_null.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/math/special_functions/nonfinite_num_facets.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>

#include "core/fitting/fitresults.h"
#include "core/fitting/fitresultsprocessor.h"
#include "core/fitting/noblefitfunction.h"
#include "core/models/combinedmodelfactory.h"
#include "core/models/modelfactory.h"

#include "commons.h"
#include "concentrationsmodel.h"
#include "fitsetup.h"
#include "histogramdata1d.h"
#include "histogramdata2d.h"
#include "montecarloresultsmodel.h"
#include "montecarloplotdelegate.h"
#include "parametersetupmodel.h"
#include "resultsmodel.h"
#include "resultswindow.h"
#include "standardfitresultsmodel.h"
#include "version.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow* MainWindow::theMainWindow = nullptr;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fit_setup_(new FitSetup()),
    fit_button_(new QPushButton("&Standard Fit")),
    ensemble_fit_button_(new QPushButton("&Ensemble Fit")),
    chi2explorer_button_(new QPushButton("χ² &Explorer"))
{
    theMainWindow = this;
    
    ui->setupUi(this);
    ui->fit_setup_widget->SetFitSetup(fit_setup_);
    fit_setup_->setParent(ui->fit_setup_widget);
    
    ui->fit_setup_widget->AddButton(fit_button_);
    ui->fit_setup_widget->AddButton(ensemble_fit_button_);
    ui->fit_setup_widget->AddButton(chi2explorer_button_);
    
    ui->action_open_results->setShortcut(QKeySequence::Open);
    ui->action_quit->setShortcut(QKeySequence::Quit);
    ui->action_load_samples_from_file->setShortcut(QKeySequence("Ctrl+L"));
    
    setWindowTitle(APPLICATION_NAME);

    ConnectSignalsAndSlots();
    ConnectButtonSignalsAndSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    for (auto& it : open_windows_)
        if(!it->close())
        {
            event->ignore();
            return;
        }
    event->accept();
}

void MainWindow::ConnectSignalsAndSlots()
{
    connect(ui->action_load_samples_from_file, SIGNAL(triggered()),
            ui->fit_setup_widget, SLOT(LoadSamplesFromFile()));
    connect(ui->action_load_samples_from_clipboard, SIGNAL(triggered()),
            ui->fit_setup_widget, SLOT(LoadSamplesFromClipboard()));
    connect(ui->action_open_results, SIGNAL(triggered()),
            this, SLOT(OpenResults()));
    connect(ui->action_about, SIGNAL(triggered()),
            this, SLOT(ShowProgramVersion()));
    connect(ui->action_quit, SIGNAL(triggered()),
            this, SLOT(close()));
}

void MainWindow::ConnectButtonSignalsAndSlots()
{
    connect(fit_button_, SIGNAL(clicked(bool)),
            fit_setup_, SLOT(Fit()));
    connect(ensemble_fit_button_, SIGNAL(clicked(bool)),
            fit_setup_, SLOT(EnsembleFit()));
    connect(chi2explorer_button_, SIGNAL(clicked(bool)),
            fit_setup_, SLOT(OpenChi2Explorer()));
}

void MainWindow::AddWindowToOpenWindowsVector(QWidget* window)
{
    open_windows_.push_back(window);
    connect(window, SIGNAL(destroyed(QObject*)),
            this, SLOT(RemoveWindowFromOpenWindowsVector(QObject*)));
}

void MainWindow::RemoveWindowFromOpenWindowsVector(QObject* window)
{
    QWidget* widget = qobject_cast<QWidget*>(window);
    if (!widget) return;
    auto it = std::find(open_windows_.begin(),
                        open_windows_.end(),
                        widget);
    if (it != open_windows_.end())
        open_windows_.erase(it);
}

void MainWindow::ShowProgramVersion()
{
    QMessageBox::information(
        this, APPLICATION_NAME, 
        QString(APPLICATION_NAME) +
        " Version: " +
        VERSION_STRING +
        "\n\nCopyright © 2013 Michael Jung\n\n"
        "The Qt libraries are "
        "Copyright © 2013 Digia Plc and/or its subsidiary(-ies). "
        "Qt is published under the GNU Lesser General Public License, "
        "Version 2.1 (a copy of the license is available at "
        "http://www.gnu.org/licenses/old-licenses/lgpl-2.1)\n\n"
        "The Qwt Widget Library is Copyright © 1997 Josef Wilgen, "
        "Copyright © 2002 Uwe Rathmann.\n\n"
        "This product includes software developed by the University of Chicago,"
        " as Operator of Argonne National Laboratory.");
}

void MainWindow::OpenResults()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            "Open Results",
            QString(),
            APPLICATION_NAME + " files (*." + BINARY_FILE_EXTENSION +
                                      " *." + PORTABLE_FILE_EXTENSION + ");;"
            "All Files (*)");
    if (filename.isEmpty())
        return;
    
    StandardFitResultsModel* results_model(nullptr);
    MonteCarloResultsModel* monte_carlo_model(nullptr);
    FitSetup* fit_setup(nullptr);
    
    bool file_is_binary;
    bool opened_successfully = false;
    std::map<QString, std::string> error_messages;
    {
        GlobalWaitCursor wait_cursor;
        try
        {
            std::ifstream ifs(filename.toStdString(), std::ios_base::binary);
            boost::archive::binary_iarchive ia(ifs);
            
            ia >> results_model
               >> monte_carlo_model
               >> fit_setup;
            
            file_is_binary = true;
            opened_successfully = true;
        }
        catch (std::exception& e)
        {
            error_messages["Binary archive"] = e.what();
        }
        catch (...)
        {
        }
        
        // Zum Laden alter unkomprimierter Dateien. Diese können jetzt nicht
        // mehr erstellt werden.
        if (!opened_successfully)
            try
            {
                // Die geänderten Locales sind notwendig um Inf und NaN
                // zuverlässig und platformunabhängig zu speichern.
                std::locale default_locale(
                        std::locale::classic(),
                        new boost::archive::codecvt_null<char>);
                std::locale in_locale(
                        default_locale,
                        new boost::math::nonfinite_num_get<char>);
                std::ifstream ifs(filename.toStdString());
                ifs.imbue(in_locale);
                boost::archive::text_iarchive ia(ifs,
                                                 boost::archive::no_codecvt);
                
                ia >> results_model
                   >> monte_carlo_model
                   >> fit_setup;
                
                file_is_binary = false;
                opened_successfully = true;
            }
            catch (std::exception& e)
            {
                error_messages["Uncompressed portable archive"] = e.what();
            }
            catch (...)
            {
            }

        if (!opened_successfully)
            try
            {
                // Die geänderten Locales sind notwendig um Inf und NaN
                // zuverlässig und platformunabhängig zu speichern.
                std::locale default_locale(
                        std::locale::classic(),
                        new boost::archive::codecvt_null<char>);
                std::locale in_locale(
                        default_locale,
                        new boost::math::nonfinite_num_get<char>);
                std::ifstream ifs(filename.toStdString(),
                                  std::ios_base::binary);
                boost::iostreams::filtering_istream f;
                f.push(boost::iostreams::zlib_decompressor());
                f.push(ifs);
                f.imbue(in_locale);
                boost::archive::text_iarchive ia(f, boost::archive::no_codecvt);
                
                ia >> results_model
                   >> monte_carlo_model
                   >> fit_setup;
                
                file_is_binary = false;
                opened_successfully = true;
            }
            catch (std::exception& e)
            {
                error_messages["Compressed portable archive"] = e.what();
            }
            catch (...)
            {
            }
        
    }
            
    if (!opened_successfully)
    {
        QString errors;
        if (error_messages.empty())
            errors = "unknown error.";
        for (const auto& error : error_messages)
            errors.push_back("\n" + error.first + ": " +
                             QString::fromStdString(error.second));
        QMessageBox::critical(this, APPLICATION_NAME,
                              "Error opening file:" + errors);
        delete results_model;
        delete monte_carlo_model;
        delete fit_setup;
        return;
    }
    
    ResultsWindow* results_window = new ResultsWindow();
    results_window->SetupResultsModelsAndViews(results_model,
                                               monte_carlo_model,
                                               fit_setup);
    results_window->SetFileName(filename, file_is_binary);
    results_window->show();
        
    delete fit_setup;
}

void MainWindow::TakeOverFitSetup()
{
    ResultsWindow* results_window = qobject_cast<ResultsWindow*>(sender());
    fit_setup_ = results_window->GetCopyOfFitSetup();
    ui->fit_setup_widget->SetFitSetup(fit_setup_);
    fit_setup_->setParent(ui->fit_setup_widget);
    ConnectButtonSignalsAndSlots();
    activateWindow();
    raise();
}
