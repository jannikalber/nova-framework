/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>

#include <workbench.h>

class Workbench : public nova::Workbench {
	public:
		Workbench() : nova::Workbench() {
			ConstructMenu(Workbench::File);
			ConstructMenu(Workbench::Edit);
			ConstructMenu(Workbench::Help);
			
			get_standard_menu(Workbench::Help)->addAction("Test");
		}
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
