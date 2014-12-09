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


#ifndef MONTECARLOPLOTDELEGATE_H
#define MONTECARLOPLOTDELEGATE_H

#include <QAbstractItemDelegate>

#include <boost/variant.hpp>

#include "histogramdata.h"

class MonteCarloEditor;
class QPainter;
class HistogramPlot1D;
class HistogramPlot2D;

class MonteCarloPlotDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    
public:
    explicit MonteCarloPlotDelegate(QObject* parent = 0);
    virtual ~MonteCarloPlotDelegate();
    
    virtual void paint(QPainter* painter,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option,
                           const QModelIndex& index) const;
                           
    virtual QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
    
private slots:
    void CommitAndCloseEditor();
 
private:
    class PaintDataVisitor : public boost::static_visitor<void>
    {
    public:
        PaintDataVisitor(const MonteCarloPlotDelegate& delegate, QPainter* painter,
                         const QStyleOptionViewItem& option);
        void operator()(SharedHistogramData1D data) const;
        void operator()(SharedHistogramData2D data) const;
    private:
        const MonteCarloPlotDelegate& delegate_;
        QPainter* painter_;
        const QStyleOptionViewItem& option_;
    };
    
    class CreateEditorVisitor : public boost::static_visitor<MonteCarloEditor*>
    {
    public:
        CreateEditorVisitor(QWidget* parent) : parent_(parent) {}
        MonteCarloEditor* operator()(SharedHistogramData1D data) const;
        MonteCarloEditor* operator()(SharedHistogramData2D data) const;
    private:
        QWidget* parent_;
    };
    
    HistogramPlot1D* plot1d_;
    HistogramPlot2D* plot2d_;
};

#endif // MONTECARLOPLOTDELEGATE_H