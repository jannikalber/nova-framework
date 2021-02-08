/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtWidgets/QApplication>

#include "workbench.h"

class Workbench : public nova::Workbench {
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
