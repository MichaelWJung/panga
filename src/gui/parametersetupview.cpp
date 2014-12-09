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


#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QStyledItemDelegate>

#include "commons.h"
#include "doubleeditorfactory.h"
#include "headercombobox.h"
#include "parametersetupheaderview.h"
#include "parametersetupmodel.h"

#include "parametersetupview.h"

ParameterSetupView::ParameterSetupView(QWidget* parent) :
    QTableView(parent),
    combo_box_(new HeaderComboBox(this)),
    border_(1),
    active_section_(-1),
    editor_factory_(new DoubleEditorFactory())
{
    combo_box_->hide();
    setHorizontalHeader(new ParameterSetupHeaderView());
    connect(horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(ShowHeaderComboBox(int)));
    dynamic_cast<QStyledItemDelegate*>(itemDelegate())->
            setItemEditorFactory(editor_factory_);

    setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* action_paste = new QAction("&Paste", this);
    action_paste->setShortcut(QKeySequence::Paste);
    action_paste->setShortcutContext(Qt::WidgetShortcut);
    addAction(action_paste);
    connect(action_paste, SIGNAL(triggered()),
            this, SLOT(Paste()));
}

ParameterSetupView::~ParameterSetupView()
{
    delete editor_factory_;
}

void ParameterSetupView::setModel(ParameterSetupModel* model)
{
    if (this->model())
    {
        disconnect(this->model(), 0, combo_box_, 0);
        disconnect(combo_box_, 0, this->model(), 0);
    }
    QTableView::setModel(model);
    combo_box_->SetParameterNames(model->GetParameterNames());
    connect(combo_box_, SIGNAL(activated(QString)),
            this, SLOT(SetActiveHeader(QString)));
    connect(this, SIGNAL(HeaderParameterChanged(int, QString)),
            model, SLOT(SetHeaderParameter(int, QString)));
    connect(model, SIGNAL(ParameterAdded(QString)),
            combo_box_, SLOT(AddParameter(QString)));
    connect(model, SIGNAL(ParameterRemoved(QString)),
            combo_box_, SLOT(RemoveParameter(QString)));
}

void ParameterSetupView::SetActiveHeader(QString parameter_name)
{
    emit HeaderParameterChanged(active_section_, parameter_name);
}

void ParameterSetupView::ShowHeaderComboBox(int section)
{
    active_section_ = section;
    QPoint header_position(horizontalHeader()->geometry().x(),
                           horizontalHeader()->geometry().y());
    QPoint section_position(horizontalHeader()->sectionPosition(section),
                            -border_);
    QRect section_rect(section_position + header_position,
                       QSize(horizontalHeader()->sectionSize(section),
                             horizontalHeader()->height()));
    
    combo_box_->SetCurrentElement(
            model()->headerData(section, Qt::Horizontal).toString());
    combo_box_->setGeometry(section_rect);
    combo_box_->show();
    combo_box_->showPopup();
}

void ParameterSetupView::Paste()
{
    QModelIndexList selection = selectedIndexes();
    if (selection.empty()) return;
    qSort(selection.begin(), selection.end(), ModelIndexLessThan());
    int row = selection.front().row();
    int col = selection.front().column();
    int initial_col = col;
    
    QString clipboard_text = QApplication::clipboard()->text();
    QStringList lines = clipboard_text.split('\n');
    
    for (const QString& line : lines)
    {
        if (!line.size()) continue;
        QStringList items = line.split('\t');
        col = initial_col;
        for (const QString& item : items)
            model()->setData(model()->index(row, col++), item.toDouble());
        ++row;
    }
}
