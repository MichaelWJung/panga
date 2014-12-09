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


#ifndef MONTECARLOEDITOR_H
#define MONTECARLOEDITOR_H

#include <QModelIndex>
#include <QWidget>

#include <histogramdata.h>

namespace Ui {
    class MonteCarloEditor;
}

class MonteCarloEditor : public QWidget
{
    Q_OBJECT
    
public:    
    explicit MonteCarloEditor(bool two_dimensional, QWidget* parent = 0);
    virtual ~MonteCarloEditor();
    
    void SetData(HistogramData data);
    void SetSampleName(QString title);
    void SetAxisTitles(QStringList axis_titles);
    void SetSampleIndex(QModelIndex sample_index);
    void closeEvent(QCloseEvent* event);
    bool AreDataAlreadySet() const;
    
signals:
    void EditingFinished();
    
private slots:
    void UpdateButtons();
    void UpdateBinNumber();
    void SavePlot();
    
private:
    void UpdateWindowTitle();
    
    boost::shared_ptr<HistogramDataBase> data_;
    Ui::MonteCarloEditor* ui;
    
    QModelIndex sample_index_;
    QString plot_name_for_window_title_;
    
    bool data_set_;
};

#endif // MONTECARLOEDITOR_H
