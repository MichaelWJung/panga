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


#ifndef HISTOGRAMPLOTZOOMER_H
#define HISTOGRAMPLOTZOOMER_H

#include <qwt_plot_canvas.h>
#include <qwt_plot_picker.h>

class HistogramPlotZoomer : public QwtPlotPicker
{
    Q_OBJECT
    
public:
    explicit HistogramPlotZoomer(QWidget* canvas);
    virtual ~HistogramPlotZoomer();

protected:
    virtual void widgetMouseReleaseEvent(QMouseEvent* me);
    
signals:
    void ZoomOut();
};

#endif // HISTOGRAMPLOTZOOMER_H
