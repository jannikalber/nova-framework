/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>

#include <workbench.h>
#include <actionprovider.h>

class Workbench : public nova::Workbench {
	public:
		inline Workbench() : nova::Workbench() {
			ConstructMenu(Workbench::File);
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Edit);
			ConstructMenu(Workbench::Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Help);
			
			menu_edit->ShowAction(menu_edit->ConstructAction("Edit Demo &1"));
			menu_edit->ShowAction(menu_edit->ConstructAction("Edit Demo &2"), true);
			menu_help->ShowAction(menu_help->ConstructAction("Help &Demo"));
		}
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
