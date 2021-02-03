/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

namespace nova {
	Workbench* workbench;
	
	Workbench::Workbench(QWidget* parent) : QMainWindow(parent) {
		workbench = this;
	}
}
