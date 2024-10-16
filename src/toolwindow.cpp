/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "toolwindow.h"

#include <QList>
#include <QSize>
#include <QPoint>
#include <QString>
#include <QLayout>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMainWindow>

#include "workbench.h"

#define NOVA_CONTEXT "nova/workbench"

namespace nova {
	ToolWindow::ToolWindow(const QString& title, Qt::Orientation orientation, Workbench* workbench, bool needs_tool_bar,
	                       Qt::DockWidgetArea default_layout):
			QDockWidget(title, workbench), ActionProvider(title), nested_main_window(new QMainWindow()),
			tool_bar(needs_tool_bar ? new QToolBar(nested_main_window) : nullptr), menu(new QMenu(this)), default_layout(default_layout),
			default_hidden(default_layout == Qt::NoDockWidgetArea), orientation(orientation), initial_size(0) {
		setObjectName("tw" + title);  // For QMainWindow::saveState()
		setAllowedAreas(orientation == Qt::Vertical ? Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea
		                                            : Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		setContextMenuPolicy(Qt::CustomContextMenu);
		
		auto* hide_action = new QAction(NOVA_TR("&Hide Tool Window"), this);
		menu->addAction(hide_action);
		connect(hide_action, &QAction::triggered, this, &ToolWindow::hide);
		
		auto* detach_action = new QAction(NOVA_TR("&Detach Tool Window"), this);
		menu->addAction(detach_action);
		connect(detach_action, &QAction::triggered, [this]() { setFloating(true); });
		connect(this, &ToolWindow::dockLocationChanged, [this, detach_action]() { detach_action->setEnabled(!isFloating()); });
		
		connect(this, &ToolWindow::customContextMenuRequested, this, &ToolWindow::customContextMenuRequested2);
		
		nested_main_window->setParent(this);  // Setting the parent using the constructor causes a separate window
		nested_main_window->setContextMenuPolicy(Qt::PreventContextMenu);
		setWidget(nested_main_window);
		
		if (tool_bar != nullptr) {
			// Tool bar
			tool_bar->setMovable(false);
			tool_bar->setIconSize(QSize(16, 16));
			
			nested_main_window->addToolBar((orientation == Qt::Vertical ? Qt::TopToolBarArea : Qt::LeftToolBarArea), tool_bar);
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
	
	void ToolWindow::DisplayAction(QAction* action, int index,
	                               bool is_important_action, int important_actions_index) {
		if (menu->actions().count() == 2) {
			menu->addSeparator();
		}
		
		index += 3;
		if (index >= menu->actions().count()) menu->addAction(action);
		else menu->insertAction(actions()[index], action);
		
		action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		addAction(action);
		
		if ((tool_bar != nullptr) && is_important_action) {
			if (important_actions_index >= tool_bar->actions().count()) tool_bar->addAction(action);
			else tool_bar->insertAction(tool_bar->actions()[important_actions_index], action);
		}
	}
	
	void ToolWindow::DisplaySeparators(bool show_regular, int index_regular,
	                                   bool show_important_actions, int index_important_actions) {
		if (show_regular) {
			index_regular += 3;
			
			if (index_regular >= menu->actions().count()) menu->addSeparator();
			else menu->insertSeparator(actions()[index_regular]);
		}
		
		if ((tool_bar != nullptr) && show_important_actions) {
			if (index_important_actions >= tool_bar->actions().count()) tool_bar->addSeparator();
			else tool_bar->insertSeparator(tool_bar->actions()[index_important_actions]);
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
	
	void ToolWindow::customContextMenuRequested2(const QPoint& pos) {
		get_content_widget()->setFocus();
		menu->exec(mapToGlobal(pos));
	}
}
