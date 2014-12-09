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

#include "concentrationsheaderview.h"
#include "concentrationsmodel.h"

#include "concentrationsview.h"

ConcentrationsView::ConcentrationsView(QWidget* parent) : QTableView(parent)
{
    setHorizontalHeader(new ConcentrationsHeaderView());
}

void ConcentrationsView::setModel(ConcentrationsModel* new_model)
{
    ConcentrationsHeaderView* header =
        qobject_cast<ConcentrationsHeaderView*>(horizontalHeader());
    assert(header);
    if (model())
    {
        disconnect(header, 0, model(), 0);
        disconnect(model(), 0, header, 0);
    }
    QTableView::setModel(new_model);
    connect(header, SIGNAL(CheckStateChanged(Qt::CheckState)),
            new_model, SLOT(ChangeSampleState(Qt::CheckState)));
    connect(new_model, SIGNAL(SampleStateChanged(Qt::CheckState)),
            header, SLOT(SetCheckState(Qt::CheckState)));
    new_model->EmitSampleState();
}
