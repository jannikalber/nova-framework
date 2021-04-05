/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#include <QtGui/QKeySequence>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAction>
#include <QtWidgets/QWhatsThis>

#include "ui_workbench.h"
#include "actionprovider.h"

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
	
	MenuActionProvider* Workbench::ConstructMenu(const QString& title) {
		auto* menu = new MenuActionProvider(title, this);
		menuBar()->addMenu(menu);
		
		return menu;
	}
	
	MenuActionProvider* Workbench::ConstructMenu(Workbench::StandardMenu standard_menu) {
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
	
	QAction* Workbench::ConstructStandardAction(StandardAction standard_action) {
		auto* action = new QAction(this);
		
		switch (standard_action) {
			case Exit:
				action->setText(QApplication::translate("nova/action", "&Exit"));
				action->setShortcut(QKeySequence("Ctrl+Q"));
				connect(action, &QAction::triggered, this, &QMainWindow::close);
				
				break;
			
			case DirectHelp:
				action->setText(QApplication::translate("nova/action", "&Direct Help"));
				action->setShortcut(QKeySequence("F2"));
				connect(action, &QAction::triggered, []() { QWhatsThis::enterWhatsThisMode(); });
				
				break;
			
			case SearchBar:
				// TODO: Implement search bar
				break;
			
			default:
				return nullptr;
		}
		
		return action;
	}
	
	MenuActionProvider* Workbench::get_standard_menu(Workbench::StandardMenu standard_menu) {
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
