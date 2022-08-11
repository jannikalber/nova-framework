/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "toolwindow.h"

#include <QList>
#include <QSize>
#include <QString>
#include <QLayout>
#include <QAction>
#include <QToolBar>
#include <QMainWindow>

namespace nova {
	ToolWindow::ToolWindow(QWidget* parent, const QString& title, Qt::Orientation orientation, bool needs_tool_bar,
	                       Qt::DockWidgetArea default_layout):
			QDockWidget(title, parent), ActionProvider(title),
			nested_main_window(new QMainWindow()),
			tool_bar(needs_tool_bar ? new QToolBar(nested_main_window) : nullptr), default_layout(default_layout),
			default_hidden(default_layout == Qt::NoDockWidgetArea), orientation(orientation), initial_size(0) {
		setObjectName("tw" + title);  // For QMainWindow::saveState()
		setAllowedAreas(orientation == Qt::Vertical ? Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
		                                            : Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		
		nested_main_window->setParent(this);  // Setting the parent using the constructor causes a separate window
		nested_main_window->setContextMenuPolicy(Qt::PreventContextMenu);
		setWidget(nested_main_window);
		
		if (tool_bar != nullptr) {
			// Tool bar
			tool_bar->setMovable(false);
			tool_bar->setIconSize(QSize(16, 16));
			
			nested_main_window->addToolBar((orientation == Qt::Vertical ? Qt::TopToolBarArea : Qt::LeftToolBarArea),
			                               tool_bar);
		}
		
		// this->default_layout is modified, therefore using this->... for consistency
		if (default_hidden || !isAreaAllowed(this->default_layout)) {
			// Illegal default layout or not displayed at beginning
			if (default_hidden) hide();
			this->default_layout = (orientation == Qt::Vertical ? Qt::LeftDockWidgetArea : Qt::BottomDockWidgetArea);
		}
	}
	
	QWidget* ToolWindow::get_content_widget() const {
		return nested_main_window->centralWidget();
	}
	
	void ToolWindow::set_content_widget(QWidget* content_widget) {
		nested_main_window->setCentralWidget(content_widget);
		initial_size = (orientation == Qt::Vertical) ? sizeHint().width() : sizeHint().height();
	}
	
	void ToolWindow::DisplayAction(QAction* action, int index, bool, int) {
		if (tool_bar != nullptr) {
			if (index >= tool_bar->actions().count()) tool_bar->addAction(action);
			else tool_bar->insertAction(tool_bar->actions()[index], action);
		}
	}
	
	void ToolWindow::DisplaySeparators(bool show_regular, int index_regular, bool, int) {
		if ((tool_bar != nullptr) && show_regular) {
			if (index_regular >= tool_bar->actions().count()) tool_bar->addSeparator();
			else tool_bar->insertSeparator(tool_bar->actions()[index_regular]);
		}
	}
	
	void ToolWindow::ConstructNavigationAction(ActionProvider* parent) {
		QAction* action = parent->ConstructAction(get_title());
		action->setCheckable(true);
		
		connect(this, &ToolWindow::visibilityChanged, action, [this, action](bool is_visible) {
			// Don't emit toggled() to avoid undefined behavior
			const bool old_state = action->blockSignals(true);
			action->setChecked(is_visible);
			action->blockSignals(old_state);
			
			if (is_visible) nested_main_window->centralWidget()->setFocus();
		});
		connect(action, &QAction::toggled, this, &ToolWindow::setVisible);
	}
}
