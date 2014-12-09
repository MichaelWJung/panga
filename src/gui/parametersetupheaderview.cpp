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


#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionComboBox>

#include "parametersetupheaderview.h"

ParameterSetupHeaderView::ParameterSetupHeaderView(QWidget* parent) :
    QHeaderView(Qt::Horizontal, parent)
{
}

void ParameterSetupHeaderView::paintSection(
        QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    
    QStyleOptionComboBox option;
    option.rect = rect;
    option.rect = style()->subControlRect(
            QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxArrow, this);

	if (isEnabled())
            option.state |= QStyle::State_Enabled
                         |  QStyle::State_Active;
            
    style()->drawPrimitive(
            QStyle::PE_IndicatorArrowDown, &option, painter, this);
}

void ParameterSetupHeaderView::mousePressEvent(QMouseEvent* e)
{
}

void ParameterSetupHeaderView::mouseMoveEvent(QMouseEvent* e)
{
}

void ParameterSetupHeaderView::mouseReleaseEvent(QMouseEvent* e)
{
    int section = logicalIndexAt(e->x());
    if (section != -1)
        emit sectionClicked(section);
}
