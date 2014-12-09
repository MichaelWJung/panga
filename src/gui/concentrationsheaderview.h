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


#ifndef CONCENTRATIONSHEADERVIEW_H
#define CONCENTRATIONSHEADERVIEW_H

#include <QHeaderView>

class ConcentrationsHeaderView : public QHeaderView
{
    Q_OBJECT
    
public:
    explicit ConcentrationsHeaderView(QWidget* parent = nullptr);
    void MakeImmutable();
    
public slots:
    void SetCheckState(Qt::CheckState check_state);
    
signals:
    void CheckStateChanged(Qt::CheckState check_state);
    
protected:
    virtual void paintSection(QPainter* painter,
                              const QRect& rect,
                              int logicalIndex) const;
                              
    virtual void leaveEvent(QEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    
private:
    Qt::CheckState check_state_;
    bool mouse_over_checkbox_;
    bool left_mouse_button_pressed_over_checkbox_;
    QRect checkbox_rect_;
    bool immutable_;
};

#endif // CONCENTRATIONSHEADERVIEW_H
