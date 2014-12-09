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


#include <QFileDialog>
#include <QImageWriter>

#include <qwt_plot_renderer.h>

#include <cassert>

#include "commons.h"
#include "histogramplot1d.h"
#include "histogramplot2d.h"

#include "montecarloeditor.h"
#include "ui_montecarloeditor.h"

MonteCarloEditor::MonteCarloEditor(bool two_dimensional, QWidget* parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::MonteCarloEditor),
    sample_index_(),
    data_set_(false)
{
    ui->setupUi(this);
    delete ui->plot;
    if (two_dimensional)
        ui->plot = new HistogramPlot2D(true, false, this);
    else
        ui->plot = new HistogramPlot1D(false, this);
    ui->plot->SetInformationLabel(ui->information_label);
    
    ui->plot->setObjectName("plot");
    ui->plot_layout->addWidget(ui->plot);
    
    ui->bin_number_spin_box->setMinimum(1);
    ui->bin_number_slider  ->setMinimum(1);
    
    setAutoFillBackground(true);
    
    connect(ui->bin_number_slider, SIGNAL(valueChangedLog(int)),
            ui->bin_number_spin_box, SLOT(setValue(int)));
    connect(ui->bin_number_spin_box, SIGNAL(valueChanged(int)),
            ui->bin_number_slider, SLOT(setValue(int)));
    connect(ui->save_plot_button, SIGNAL(clicked(bool)),
            this, SLOT(SavePlot()));
}

MonteCarloEditor::~MonteCarloEditor()
{
    delete ui;
}

void MonteCarloEditor::SetData(HistogramData data)
{
    if (data_set_) return;
    
    data_ = HistogramDataBase::GetBase(data);
    ui->plot->SetData(data);
    const unsigned n_monte_carlos = data_->size;
    
    {
        SignalBlocker b1(ui->bin_number_spin_box);
        SignalBlocker b2(ui->bin_number_slider);
        const unsigned max_bin_number =
                HistogramPlot::CalcMaxBinNumber(n_monte_carlos);
        ui->bin_number_spin_box->setMaximum(max_bin_number);
        ui->bin_number_slider  ->setMaximum(max_bin_number);
    }

    connect(data_.get(), SIGNAL(HistogramChanged()),
            this, SLOT(UpdateButtons()));
    connect(data_.get(), SIGNAL(SelectionChanged()),
            this, SLOT(UpdateButtons()));
    connect(data_.get(), SIGNAL(BinNumberChanged()),
            this, SLOT(UpdateBinNumber()));
    connect(ui->override_check_box, SIGNAL(toggled(bool)),
            data_.get(), SLOT(UsePlotSpecificBinNumber(bool)));
    connect(ui->bin_number_spin_box, SIGNAL(valueChanged(int)),
            data_.get(), SLOT(SetBinNumber(int)));
    connect(ui->clear_button, SIGNAL(clicked(bool)),
            data_.get(), SLOT(ClearSelection()));
    connect(ui->mask_button, SIGNAL(clicked(bool)),
            data_.get(), SLOT(MaskWithSelection()));
    connect(ui->unmask_button, SIGNAL(clicked(bool)),
            data_.get(), SLOT(ResetMask()));
    connect(ui->invert_mask_button, SIGNAL(clicked(bool)),
            data_.get(), SLOT(InvertMask()));
    connect(ui->invert_selection_button, SIGNAL(clicked(bool)),
            data_.get(), SLOT(SwitchSelectionInverted()));
    
    UpdateBinNumber();
    UpdateButtons();
    
    data_set_ = true;
}
void MonteCarloEditor::SetSampleName(QString title)
{
    ui->plot->setTitle(title);
    UpdateWindowTitle();
}

void MonteCarloEditor::SetAxisTitles(QStringList axis_titles)
{
    assert(axis_titles.size() == 1 || axis_titles.size() == 2);
    if (axis_titles.size() == 2)
        plot_name_for_window_title_ = axis_titles[1] + " vs " +
                                      axis_titles[0];
    else
        plot_name_for_window_title_ = axis_titles[0];
    if (axis_titles.size() == 1)
        axis_titles.push_back("Counts");
    ui->plot->SetAxisTitles(axis_titles);
    UpdateWindowTitle();
}

void MonteCarloEditor::SetSampleIndex(QModelIndex sample_index)
{
    sample_index_ = sample_index;
}

void MonteCarloEditor::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    emit EditingFinished();
}

bool MonteCarloEditor::AreDataAlreadySet() const
{
    return data_set_;
}

void MonteCarloEditor::UpdateBinNumber()
{
    const unsigned n_bins = data_->GetCurrentBinNumber();
    SignalBlocker b1(ui->override_check_box);
    SignalBlocker b2(ui->bin_number_spin_box);
    SignalBlocker b3(ui->bin_number_slider);
    
    ui->override_check_box->setChecked(data_->IsPlotSpecificBinNumberUsed());
    ui->bin_number_spin_box->setValue(n_bins);
    ui->bin_number_slider  ->setValue(n_bins);
}

void MonteCarloEditor::UpdateButtons()
{
    ui->           clear_button->setEnabled(ui->plot->SelectionActive());
    ui->            mask_button->setEnabled(ui->plot->SelectionActive());
    ui->invert_selection_button->setEnabled(ui->plot->SelectionActive());
    ui->          unmask_button->setEnabled(data_->GetMask().active());
    ui->     invert_mask_button->setEnabled(data_->GetMask().active());
}

void MonteCarloEditor::SavePlot()
{
    const QList<QByteArray> imageFormats =
        QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter("Images (");
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    QString fileName(ui->plot->title().text() + ".pdf");
    fileName = QFileDialog::getSaveFileName(
        this, tr("Save Plot..."), fileName, filter.join(";;"));

    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;

        // flags to make the document look like the widget
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);
        //renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);

        renderer.renderDocument(ui->plot, fileName, QSizeF(240, 180), 120);
    }
}

void MonteCarloEditor::UpdateWindowTitle()
{
    const QString sample_name = ui->plot->title().text();
    setWindowTitle(plot_name_for_window_title_ + " - " +
                   sample_name + " - " +
                   APPLICATION_NAME);
}
