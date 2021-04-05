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
			ConstructMenu(Workbench::File)->ShowAction(ConstructStandardAction(Workbench::Exit));
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Edit);
			ConstructMenu(Workbench::Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Help);
			
			menu_edit->ShowAction(menu_edit->ConstructAction("&Edit Demo"));
			menu_help->ShowAction(ConstructStandardAction(Workbench::SearchBar));
			menu_help->ShowAction(ConstructStandardAction(Workbench::DirectHelp), true);
			menu_help->ShowAction(menu_help->ConstructAction("&Help Demo"));
		}
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
