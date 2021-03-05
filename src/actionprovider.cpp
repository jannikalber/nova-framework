/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "actionprovider.h"

#include <QtWidgets/QWidget>

namespace nova {
	MenuActionProvider::MenuActionProvider(const QString& title, QWidget* parent)
			: ActionProvider(QString(title).replace('&', " ")), QMenu(parent) {
		setTitle(title);
	}
	
	void MenuActionProvider::ShowAction(QAction* action, bool separate, bool is_important) {
		if (separate) addSeparator();
		addAction(action);
		
		// TODO: Implement is_important
	}
}
