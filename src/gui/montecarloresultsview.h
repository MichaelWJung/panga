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


#ifndef MONTECARLORESULTSVIEW_H
#define MONTECARLORESULTSVIEW_H

#include <QTableView>

#include <map>

class MonteCarloEditor;
class MonteCarloResultsView : public QTableView
{
    Q_OBJECT
    
public:
    explicit MonteCarloResultsView(QWidget* parent = 0);
    void SetResultsWindow(QWidget* results_window);
    virtual void setModel(QAbstractItemModel* new_model);
    
public slots:
    void CloseAllEditors();
    
protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void updateGeometries();
    
private slots:
    void CallClosePersistentEditor();
    void RemoveEditorFromIndexMap(QObject* editor);
    void SetWindowModified();
    
private:
    std::map<QObject*, QModelIndex> index_map_;
    QWidget* results_window_;
};

#endif // MONTECARLORESULTSVIEW_H
