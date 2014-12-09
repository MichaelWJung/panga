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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <list>
#include <map>
#include <memory>

#include "core/fitting/fitconfiguration.h"
#include "core/models/combinedmodel.h"
#include "core/misc/rundata.h"
#include "core/misc/typedefs.h"

#include "parametersetupmodel.h"

#include "guiresultsprocessor.h"

namespace Ui {
    class MainWindow;
}

class FitSetup;
class ResultsWindow;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    static MainWindow* theMainWindow;
    
    void AddWindowToOpenWindowsVector(QWidget* window);
    
public slots:
    void TakeOverFitSetup();
    
protected:
    void closeEvent(QCloseEvent* event);
    
private:
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    
    void ConnectSignalsAndSlots();
    void ConnectButtonSignalsAndSlots();
    
private slots:
    void ShowProgramVersion();
    void OpenResults();
    void RemoveWindowFromOpenWindowsVector(QObject* window);
    
private:
    Ui::MainWindow* ui;
    FitSetup* fit_setup_;
    QPushButton* fit_button_;
    QPushButton* ensemble_fit_button_;
    QPushButton* chi2explorer_button_;
    std::list<QWidget*> open_windows_;
};

#endif // MAINWINDOW_H