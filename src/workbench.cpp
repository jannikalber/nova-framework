/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#include <QtWidgets/QApplication>

#include "ui_workbench.h"

namespace nova {
	Workbench* workbench;
	
	Workbench::Workbench(QWidget* parent) : QMainWindow(parent), ui(new Ui::Workbench()),
	                                        menu_file(nullptr), menu_edit(nullptr), menu_help(nullptr) {
		workbench = this;
		ui->setupUi(this);
	}
	
	Workbench::~Workbench() noexcept {
		delete ui;
	}
	
	QMenu* Workbench::ConstructMenu(const QString& title) {
		auto* menu = new QMenu(this);
		menu->setTitle(title);
		menuBar()->addMenu(menu);
		
		return menu;
	}
	
	QMenu* Workbench::ConstructMenu(Workbench::StandardMenu standard_menu) {
		switch (standard_menu) {
			case File:
				menu_file = ConstructMenu(QApplication::translate("nova/menu", "&File"));
				return menu_file;
			
			case Edit:
				menu_edit = ConstructMenu(QApplication::translate("nova/menu", "&Edit"));
				return menu_edit;
			
			case Help:
				menu_help = ConstructMenu(QApplication::translate("nova/menu", "&Help"));
				return menu_help;
			
			default:
				return nullptr;
		}
	}
	
	QMenu* Workbench::get_standard_menu(Workbench::StandardMenu standard_menu) {
		switch (standard_menu) {
			case File:
				return menu_file;
				
			case Edit:
				return menu_edit;
				
			case Help:
				return menu_help;
			
			default:
				return nullptr;
		}
	}
}
