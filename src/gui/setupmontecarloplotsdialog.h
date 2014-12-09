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


#ifndef SETUPMONTECARLOPLOTSDIALOG_H
#define SETUPMONTECARLOPLOTSDIALOG_H

#include <QDialog>

#include "columntypelistmodel.h"

namespace Ui {
    class SetupMonteCarloPlotsDialog;
}

class MonteCarloResultsModel;

class SetupMonteCarloPlotsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupMonteCarloPlotsDialog(
            const MonteCarloResultsModel& results_model,
            QWidget* parent = 0,
            Qt::WindowFlags f = 0);
    virtual ~SetupMonteCarloPlotsDialog();

public slots:
    void accept();

signals:
    void accepted(const QList<ColumnTypeVariant> column_types);

private:
    Ui::SetupMonteCarloPlotsDialog* ui;
};

#endif // SETUPMONTECARLOPLOTSDIALOG_H
