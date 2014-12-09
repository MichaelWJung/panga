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


#include <QLabel>
#include <QPainter>
#include <QStyleOptionViewItem>

#include <cassert>

#include "histogramplot.h"

HistogramPlot::HistogramPlot(bool in_table, QWidget* parent) :
    QwtPlot(parent),
    in_table_(in_table),
    information_label_(nullptr)
{
    if (in_table_)
    {
        setAxisMaxMajor(QwtPlot::xBottom, 3);
        setAxisMaxMajor(QwtPlot::yLeft, 4);
    }
}

void HistogramPlot::Paint(QPainter* painter, const QStyleOptionViewItem& option)
{
    UpdatePlot();
    resize(option.rect.size());
    updateAxes();
    replot();
    painter->save();
    painter->translate(option.rect.topLeft());
    render(painter);
    painter->restore();
}

void HistogramPlot::SetData(HistogramData data)
{
    if (data_)
        disconnect(data_.get(), 0, this, 0);
    data_ = HistogramDataBase::GetBase(data);
    
    connect(data_.get(), SIGNAL(HistogramChanged()),
            this, SLOT(UpdatePlot()));
    connect(data_.get(), SIGNAL(SelectionChanged()),
            this, SLOT(UpdatePlot()));
}

void HistogramPlot::SetInformationLabel(QLabel* label)
{
    information_label_ = label;
}

void HistogramPlot::UpdatePlot()
{
    if (information_label_ && data_)
    {
        QString text;
        text += "Number of samples:\n"
              + QString::number(data_->GetMask().NumberOfUnmaskedElements())
              + " ("
              + QString::number(data_->GetMask().
                                    NumberOfUnmaskedElementsPerCent(),
                                'f', 2)
              + "%)\n"
              + "Converged: "
              + QString::number(data_->GetMask().PerCentConverged(), 'f', 2)
              + "%";
        information_label_->setText(text);
    }
}

void HistogramPlot::SetAxisTitles(QStringList axis_titles)
{
    assert(axis_titles.size() == 2);
    setAxisTitle(QwtPlot::xBottom, axis_titles.takeFirst());
    setAxisTitle(QwtPlot::yLeft  , axis_titles.takeFirst());
}

bool HistogramPlot::SelectionActive()
{
    return false;
}

QSize HistogramPlot::sizeHint() const
{
    if (in_table_)
        return QSize(300, 250);
    return QwtPlot::sizeHint();
}

unsigned HistogramPlot::CalcMaxBinNumber(unsigned int n_monte_carlos)
{
    // Zahl der Monte Carlos hoch 0.6 sollte verhindern dass es zu Speicher-
    // problemen kommt.
    return std::pow(n_monte_carlos, .6);
}
