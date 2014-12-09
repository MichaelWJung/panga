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


#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionViewItemV4>

#include "concentrationsheaderview.h"

ConcentrationsHeaderView::ConcentrationsHeaderView(QWidget* parent) :
    QHeaderView(Qt::Horizontal, parent),
    check_state_(Qt::Checked),
    mouse_over_checkbox_(false),
    left_mouse_button_pressed_over_checkbox_(false),
    checkbox_rect_(),
    immutable_(false)
{
}

void ConcentrationsHeaderView::MakeImmutable()
{
    immutable_ = true;
    update();
}

void ConcentrationsHeaderView::SetCheckState(Qt::CheckState check_state)
{
    bool update_needed = check_state != check_state_;
    check_state_ = check_state;
    if (update_needed)
        updateSection(0);
}

void ConcentrationsHeaderView::paintSection(QPainter* painter,
                                            const QRect& rect,
                                            int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    
    if (logicalIndex == 0)
    {
        QStyleOptionViewItemV4 option;
        option.features |= QStyleOptionViewItemV4::HasCheckIndicator;
        
        if (isEnabled() && !immutable_)
            option.state |= QStyle::State_Enabled |  QStyle::State_Active;
        else
            option.state |= QStyle::State_ReadOnly;
        
        switch (check_state_)
        {
            case Qt::Checked:
                option.state |= QStyle::State_On;
                break;
            case Qt::Unchecked:
                option.state |= QStyle::State_Off;
                break;
            case Qt::PartiallyChecked:
                option.state |= QStyle::State_NoChange;
        }
        
        if (mouse_over_checkbox_)
            option.state |= QStyle::State_MouseOver;
        
        option.rect = rect;
        option.rect = style()->subElementRect(
                QStyle::SE_ItemViewItemCheckIndicator, &option);
        const_cast<QRect&>(checkbox_rect_) = option.rect;
        //! \todo Dieses const_cast überdenken!!
        
        option.rect = rect;
        style()->drawControl(QStyle::CE_ItemViewItem, &option, painter, this);
    }
}

void ConcentrationsHeaderView::leaveEvent(QEvent* e )
{
    QWidget::leaveEvent(e);
    if (mouse_over_checkbox_)
    {
        mouse_over_checkbox_ = false;
        updateSection(0);
    }
}

void ConcentrationsHeaderView::mouseMoveEvent(QMouseEvent* e)
{
    QHeaderView::mouseMoveEvent(e);
    bool mouse_over_checkbox;
    if (checkbox_rect_.contains(e->pos()))
        mouse_over_checkbox = true;
    else
        mouse_over_checkbox = false;
    if (mouse_over_checkbox != mouse_over_checkbox_)
    {
        mouse_over_checkbox_ = mouse_over_checkbox;
        updateSection(0);
    }
}

void ConcentrationsHeaderView::mousePressEvent(QMouseEvent* e)
{
    QHeaderView::mousePressEvent(e);
    if (!immutable_ &&
        e->button() == Qt::LeftButton &&
        isEnabled() &&
        checkbox_rect_.contains(e->pos()))
        left_mouse_button_pressed_over_checkbox_ = true;
}

void ConcentrationsHeaderView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QHeaderView::mouseDoubleClickEvent(e);
    if (!immutable_ &&
        e->button() == Qt::LeftButton &&
        isEnabled() &&
        checkbox_rect_.contains(e->pos()))
        left_mouse_button_pressed_over_checkbox_ = true;
}

void ConcentrationsHeaderView::mouseReleaseEvent(QMouseEvent* e)
{
    QHeaderView::mouseReleaseEvent(e);
    if (!immutable_ && e->button() == Qt::LeftButton)
    {
        if (left_mouse_button_pressed_over_checkbox_ &&
            isEnabled() &&
            checkbox_rect_.contains(e->pos()))
        {
            switch (check_state_)
            {
                case Qt::Checked:
                    check_state_ = Qt::Unchecked;
                    break;
                case Qt::Unchecked:
                case Qt::PartiallyChecked:
                    check_state_ = Qt::Checked;
                    break;
            }
            
            updateSection(0);
            emit CheckStateChanged(check_state_);
        }
        left_mouse_button_pressed_over_checkbox_ = false;
    }
}
