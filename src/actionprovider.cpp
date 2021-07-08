/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "actionprovider.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QWidget>

#include "workbench.h"

namespace nova {
	ActionProvider::ActionProvider(const QString& title, Workbench* window)
			: QObject(window), title(title) {
		window->providers << this;
	}
	
	QAction* ActionProvider::ConstructAction(const QString& text) {
		auto* action = new QAction(this);
		action->setText(text);
		return action;
	}
	
	MenuActionProvider::MenuActionProvider(const QString& title, Workbench* window)
			: ActionProvider(QString(title).replace('&', " "), window), QMenu(window) {
		setTitle(title);
	}
	
	void MenuActionProvider::ShowAction(QAction* action, bool separate, bool is_important) {
		if (separate) addSeparator();
		addAction(action);
		
		// TODO: Implement is_important
	}
}
