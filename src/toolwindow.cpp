/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "toolwindow.h"

#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QMainWindow>

#include "workbench.h"

namespace nova {
	ToolWindow::ToolWindow(QWidget* parent, const QString& title, Qt::Orientation orientation, bool needs_tool_bar,
	                       Qt::DockWidgetArea default_layout) :
			QDockWidget(title, parent), ActionProvider(title),
			nested_main_window(new QMainWindow()),
			tool_bar(needs_tool_bar ? new QToolBar(nested_main_window) : nullptr), default_layout(default_layout),
			default_hidden(default_layout == Qt::NoDockWidgetArea) {
		setAllowedAreas(orientation == Qt::Vertical ? Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
		                                            : Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		nested_main_window->setContextMenuPolicy(Qt::PreventContextMenu);
		setWidget(nested_main_window);
		
		if (tool_bar != nullptr) {
			// Tool bar
			tool_bar->setMovable(false);
			tool_bar->setIconSize(QSize(16, 16));
			
			nested_main_window->addToolBar(orientation == Qt::Vertical ? Qt::TopToolBarArea : Qt::LeftToolBarArea,
			                               tool_bar);
		}
		
		// this->default_layout is modified, therefore using this->... for consistency
		if (default_hidden || !isAreaAllowed(this->default_layout)) {
			// Illegal default layout or not displayed at beginning
			if (default_hidden) hide();
			this->default_layout = (orientation == Qt::Vertical ? Qt::LeftDockWidgetArea : Qt::BottomDockWidgetArea);
		}
	}
	
	ToolWindow::~ToolWindow() noexcept {
		delete nested_main_window;
	}
	
	QWidget* ToolWindow::get_content_widget() const {
		return nested_main_window->centralWidget();
	}
	
	void ToolWindow::set_content_widget(QWidget* widget) {
		nested_main_window->setCentralWidget(widget);
	}
	
	void ToolWindow::ShowAction(QAction* action, bool separate, bool is_important) {
		if (is_important && tool_bar != nullptr) {
			if (!tool_bar->actions().isEmpty() && separate) tool_bar->addSeparator();
			tool_bar->addAction(action);
		}
	}
}
