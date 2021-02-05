/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#include "ui_workbench.h"

namespace nova {
	Workbench* workbench;
	
	Workbench::Workbench(QWidget* parent) : QMainWindow(parent), ui(new Ui::Workbench()) {
		workbench = this;
		ui->setupUi(this);
	}
	
	Workbench::~Workbench() noexcept {
		delete ui;
	}
}
