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


#include <cassert>

#include "setupmontecarloplotsdialog.h"
#include "ui_setupmontecarloplotsdialog.h"

SetupMonteCarloPlotsDialog::SetupMonteCarloPlotsDialog(
        const MonteCarloResultsModel& results_model,
        QWidget* parent,
        Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::SetupMonteCarloPlotsDialog)
{
    ui->setupUi(this);

    ui->all_items_list_view->setModel(new ColumnTypeListModel(
            results_model.GetAvailableColumnTypes(),
            results_model,
            false,
            true,
            ui->all_items_list_view));
    ui->selected_items_list_view->setModel(new ColumnTypeListModel(
            results_model.GetColumnTypes(),
            results_model,
            true,
            false,
            ui->all_items_list_view));

    setAttribute(Qt::WA_DeleteOnClose);
}

SetupMonteCarloPlotsDialog::~SetupMonteCarloPlotsDialog()
{
    delete ui;
}

void SetupMonteCarloPlotsDialog::accept()
{
    const ColumnTypeListModel* model = qobject_cast<const ColumnTypeListModel*>(
        ui->selected_items_list_view->model());
    emit accepted(model->GetColumnTypes());
    QDialog::accept();
}
