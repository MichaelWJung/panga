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


#include <QModelIndex>
#include <QMouseEvent>
#include <QScrollBar>

#include <algorithm>
#include <cassert>

#include "montecarloeditor.h"

#include "montecarloresultsview.h"

MonteCarloResultsView::MonteCarloResultsView(QWidget* parent) :
    QTableView(parent)
{
}

void MonteCarloResultsView::SetResultsWindow(QWidget* results_window)
{
    results_window_ = results_window;
}

void MonteCarloResultsView::setModel(QAbstractItemModel* new_model)
{
    if (model())
        disconnect(model(), SIGNAL(AnyDataChanged()),
                   this, SLOT(SetWindowModified()));
    QTableView::setModel(new_model);
    connect(model(), SIGNAL(AnyDataChanged()),
            this, SLOT(SetWindowModified()));
}

void MonteCarloResultsView::CloseAllEditors()
{
    for (const auto& it : index_map_)
        closePersistentEditor(it.second);
}

void MonteCarloResultsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) return;

    auto it = std::find_if(index_map_.cbegin(),
                           index_map_.cend(),
                           [&](const std::pair<QObject*, QModelIndex>& p)
                           {
                               return p.second == index;
                           });
    if (it == index_map_.cend())
    {
        openPersistentEditor(index);
        MonteCarloEditor* editor =
                qobject_cast<MonteCarloEditor*>(indexWidget(index));
        index_map_[editor] = index;
        connect(editor, SIGNAL(EditingFinished()),
                this, SLOT(CallClosePersistentEditor()));
        connect(editor, SIGNAL(destroyed(QObject*)),
                this, SLOT(RemoveEditorFromIndexMap(QObject*)));
    }
    else
    {
        MonteCarloEditor* editor = qobject_cast<MonteCarloEditor*>(it->first);
        assert(editor);
        if (editor->isMinimized())
        {
#ifdef __gnu_linux__
            // Irgendein Bug unter Linux sorgt dafür, dass die Fenster nicht
            // direkt aufgehen, daher dieser Workaround.
            editor->showNormal();
            editor->showMinimized();
#endif
            editor->showNormal();
        }
        editor->activateWindow();
        editor->raise();
    }
}

void MonteCarloResultsView::updateGeometries()
{
    QTableView::updateGeometries();
    horizontalScrollBar()->setSingleStep(20);
    verticalScrollBar()  ->setSingleStep(20);
}

void MonteCarloResultsView::CallClosePersistentEditor()
{
    closePersistentEditor(index_map_.at(sender()));
}

void MonteCarloResultsView::RemoveEditorFromIndexMap(QObject* editor)
{
    index_map_.erase(editor);
}

void MonteCarloResultsView::SetWindowModified()
{
    if (results_window_)
        results_window_->setWindowModified(true);
}
