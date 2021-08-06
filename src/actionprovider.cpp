/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "actionprovider.h"

#include <QtCore/QList>
#include <QtCore/QSize>
#include <QtWidgets/QAction>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

#include "workbench.h"

namespace nova {
	ActionProvider::ActionProvider(const QString& title, Workbench* window)
			: object(QObject()), title(title) {
		window->providers << this;
	}
	
	QAction* ActionProvider::ConstructAction(const QString& text) {
		auto* action = new QAction(&object);
		action->setText(text);
		return action;
	}
	
	MenuActionProvider::MenuActionProvider(const QString& title, bool needs_tool_bar, Workbench* window)
			: ActionProvider(QString(title).replace('&', " "), window), QMenu(window),
			  tool_bar(needs_tool_bar ? new QToolBar(get_title(), this) : nullptr) {
		setTitle(title);
		if (tool_bar != nullptr) tool_bar->setIconSize(QSize(16, 16));
	}
	
	void MenuActionProvider::ShowAction(QAction* action, bool separate, bool is_important) {
		if (separate) addSeparator();
		addAction(action);
		
		if (is_important && tool_bar != nullptr) {
			if (separate && !tool_bar->actions().isEmpty()) tool_bar->addSeparator();
			tool_bar->addAction(action);
		}
	}
}
