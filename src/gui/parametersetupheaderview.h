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


#ifndef PARAMETERSETUPHEADERVIEW_H
#define PARAMETERSETUPHEADERVIEW_H

#include <QHeaderView>

class ParameterSetupHeaderView : public QHeaderView
{
public:
    explicit ParameterSetupHeaderView(QWidget* parent = 0);
    
protected:
    virtual void paintSection(
            QPainter* painter, const QRect& rect, int logicalIndex) const;
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
};

#endif // PARAMETERSETUPHEADERVIEW_H