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


#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>

#include "ensemblefitconfigurationdialog.h"

EnsembleFitConfigurationDialog::EnsembleFitConfigurationDialog(
        std::map<std::string, bool>& parameter_configuration,
        QWidget* parent,
        Qt::WindowFlags f) :
    QDialog(parent, f),
    parameter_configuration_(parameter_configuration)
{
    QVBoxLayout* layout = new QVBoxLayout();
    QLabel* message = new QLabel("Please select the parameters to fit as an "
                                 "ensemble:");
    layout->addWidget(message);
    for (const auto& parameter : parameter_configuration_)
    {
        QCheckBox* checkbox =
                new QCheckBox(QString::fromStdString(parameter.first));
        checkbox->setChecked(parameter.second);
        connect(checkbox, SIGNAL(clicked(bool)),
                this, SLOT(ChangeFitState(bool)));
        layout->addWidget(checkbox);
    }

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    setLayout(layout);
}

void EnsembleFitConfigurationDialog::ChangeFitState(bool enabled)
{
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (!checkbox) return;
    parameter_configuration_.at(checkbox->text().toStdString()) = enabled;
}
