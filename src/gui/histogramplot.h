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


#ifndef HISTOGRAMPLOT_H
#define HISTOGRAMPLOT_H

#include <qwt_plot.h>

#include "histogramdata.h"

class QLabel;
class QPainter;
class QStyleOptionViewItem;

class HistogramPlot : public QwtPlot
{
    Q_OBJECT

public:
    explicit HistogramPlot(bool in_table = false, QWidget* parent = 0);

    void Paint(QPainter* painter, const QStyleOptionViewItem& option);
    
    virtual void SetData(HistogramData data);
    virtual void SetInformationLabel(QLabel* label);
    virtual void SetAxisTitles(QStringList axis_titles);
    virtual bool SelectionActive();
    virtual QSize sizeHint() const;
    
    //! Berechnet die für die Plots maximal mögliche Binzahl.
    static unsigned CalcMaxBinNumber(unsigned n_monte_carlos);

public slots:
    virtual void UpdatePlot();
    
protected:
    const bool in_table_;
    QLabel* information_label_;

private:
    SharedHistogramDataBase data_;
};
#endif // HISTOGRAMPLOT_H
