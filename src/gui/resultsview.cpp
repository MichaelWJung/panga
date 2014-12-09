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


#include <QApplication>
#include <QClipboard>
#include <QMenu>

#include <algorithm>
#include <iostream>
#include <set>

#include "commons.h"

#include "resultsview.h"

ResultsView::ResultsView(QWidget* parent) :
    QTableView(parent)
{
    setContextMenuPolicy(Qt::ActionsContextMenu);
    
    QAction* action_copy = new QAction("&Copy", this);
    action_copy->setShortcut(QKeySequence::Copy);
    addAction(action_copy);
    
    QAction* action_copy_with_headers = new QAction("Copy with &headers", this);
    action_copy_with_headers->setShortcut(Qt::Key_H | Qt::CTRL);
    addAction(action_copy_with_headers);
    
    connect(action_copy, SIGNAL(triggered()),
            this, SLOT(Copy()));
    connect(action_copy_with_headers, SIGNAL(triggered(bool)),
            this, SLOT(CopyWithHeaders()));
}

void ResultsView::Copy(bool include_headers)
{
    if (!selectionModel()->hasSelection())
        return;
    
    std::set<int> cols;
    std::set<int> rows;
   
    // Irgendwie macht das MonteCarloSummaryProxyModel hier Ärger und gibt
    // ungültige Indizes zurück wenn man selectedIndexes() verwendet.
    // Daher so, das scheint zu funktionieren.
    for (int i = 0; i < model()->columnCount(); ++i)
        if (selectionModel()->columnIntersectsSelection(i, QModelIndex()))
            cols.insert(i);
    for (int i = 0; i < model()->rowCount(); ++i)
        if (selectionModel()->rowIntersectsSelection(i, QModelIndex()))
            rows.insert(i);
    
    if (cols.empty() || rows.empty())
        return;
    
    int last_col = *--cols.end();
    int last_row = *--rows.end();
        
    QString table;
    
    if (include_headers)
    {
        table.append('\t');
        for (int col : cols)
        {
            table.append(model()->headerData(col, Qt::Horizontal).toString());
            table.append(col == last_col ? '\n' : '\t');
        }
    }    
    for (int row : rows)
    {
        if (include_headers)
            table.append(model()->headerData(row, Qt::Vertical).
                    toString() + '\t');
        for (int col : cols)
        {
            table.append(model()->data(model()->index(row, col)).toString());
            if (col != last_col)
                table.append('\t');
        }
        if (row != last_row)
            table.append('\n');
    }
    
    QApplication::clipboard()->setText(table);
}

void ResultsView::CopyWithHeaders()
{
    Copy(true);
}
