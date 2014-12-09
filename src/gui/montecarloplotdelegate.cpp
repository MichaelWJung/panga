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


#include <QMetaType>
#include <QPainter>

#include <cassert>

#include "histogramplot1d.h"
#include "histogramplot2d.h"
#include "montecarloeditor.h"

#include "montecarloplotdelegate.h"

MonteCarloPlotDelegate::MonteCarloPlotDelegate(QObject* parent) :
    QAbstractItemDelegate(parent),
    plot1d_(new HistogramPlot1D(true)),
    plot2d_(new HistogramPlot2D(false, true))
{
}

MonteCarloPlotDelegate::~MonteCarloPlotDelegate()
{
    delete plot1d_;
    delete plot2d_;    
}

MonteCarloPlotDelegate::PaintDataVisitor::PaintDataVisitor(
        const MonteCarloPlotDelegate& delegate, QPainter* painter,
        const QStyleOptionViewItem& option) :
    delegate_(delegate),
    painter_(painter),
    option_(option)
{
}

void MonteCarloPlotDelegate::PaintDataVisitor::operator()(
        SharedHistogramData1D data) const
{
    delegate_.plot1d_->SetData(data);
    delegate_.plot1d_->Paint(painter_, option_);
}

void MonteCarloPlotDelegate::PaintDataVisitor::operator()(
        SharedHistogramData2D data) const
{
    delegate_.plot2d_->SetData(data);
    delegate_.plot2d_->Paint(painter_, option_);
}

MonteCarloEditor* MonteCarloPlotDelegate::CreateEditorVisitor::operator()(
        SharedHistogramData1D data) const
{
    return new MonteCarloEditor(false, parent_);
}

MonteCarloEditor* MonteCarloPlotDelegate::CreateEditorVisitor::operator()(
        SharedHistogramData2D data) const
{
    return new MonteCarloEditor(true, parent_);
}


void MonteCarloPlotDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    if (index.data().canConvert<HistogramData>())
    {
        HistogramData data = qvariant_cast<HistogramData>(index.data());
        boost::apply_visitor(PaintDataVisitor(*this, painter, option), data);
    }
}

QSize MonteCarloPlotDelegate::sizeHint(
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    return plot1d_->sizeHint();
}

QWidget* MonteCarloPlotDelegate::createEditor(
        QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const
{
    if (index.data().canConvert<HistogramData>())
    {
        HistogramData data = qvariant_cast<HistogramData>(index.data());
        MonteCarloEditor* editor = boost::apply_visitor(
                CreateEditorVisitor(parent), data);
        connect(editor, SIGNAL(EditingFinished()),
                this, SLOT(CommitAndCloseEditor()));
        return editor;
    }
    else
        return nullptr;
}

void MonteCarloPlotDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.data().canConvert<HistogramData>())
    {
        HistogramData data = qvariant_cast<HistogramData>(index.data());
        MonteCarloEditor* mc_editor = qobject_cast<MonteCarloEditor*>(editor);
        if (!mc_editor->AreDataAlreadySet())
        {
            mc_editor->SetData(data);
            QStringList titles = index.data(Qt::UserRole).toStringList();
            assert(titles.size() == 2 || titles.size() == 3);
            mc_editor->SetSampleName(titles.takeFirst());
            mc_editor->SetAxisTitles(titles);
            mc_editor->SetSampleIndex(index);
        }
    }
}

void MonteCarloPlotDelegate::CommitAndCloseEditor()
{
    MonteCarloEditor* editor = qobject_cast<MonteCarloEditor*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
