/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#include <QtGui/QKeySequence>
#include <QtGui/QShowEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAction>
#include <QtWidgets/QWhatsThis>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>

#include "ui_workbench.h"
#include "actionprovider.h"
#include "searchbar.h"

namespace nova {
	Workbench* workbench;
	
	Workbench::Workbench(QWidget* parent)
			: QMainWindow(parent), ProgressMonitor(), ui(new Ui::Workbench()), menu_file(nullptr),
			  menu_edit(nullptr), menu_help(nullptr), providers(QList<ActionProvider*>()),
			  taskbar_button(new QWinTaskbarButton(this)) {
		workbench = this;
		ui->setupUi(this);
		
		ui->statusBar->addPermanentWidget(ui->wdgProgress, 1);
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
	
	QAction* Workbench::ConstructStandardAction(StandardAction standard_action, ActionProvider* provider) {
		auto* action = new QAction(provider);
		
		switch (standard_action) {
			case Exit:
				action->setText(QApplication::translate("nova/action", "&Exit"));
				action->setShortcut(QKeySequence("Ctrl+Q"));
				connect(action, &QAction::triggered, this, &Workbench::close);
				
				break;
			
			case DirectHelp:
				action->setText(QApplication::translate("nova/action", "&Direct Help"));
				action->setShortcut(QKeySequence("F2"));
				connect(action, &QAction::triggered, []() { QWhatsThis::enterWhatsThisMode(); });
				
				break;
			
			case SearchBar:
				action->setText(QApplication::translate("nova/action", "&Search..."));
				action->setShortcut(QKeySequence("F3"));
				connect(action, &QAction::triggered, [this]() {
					class SearchBar bar(this);
					bar.exec();
				});
				
				break;
			
			default:
				return nullptr;
		}
		
		return action;
	}
	
	MenuActionProvider* Workbench::get_standard_menu(Workbench::StandardMenu standard_menu) const {
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
	
	void Workbench::AddStatusBarWidget(QWidget* widget, int stretch) {
		// Guarantee the widget to be inserted in front of the progress indicator
		static int index = 0;
		ui->statusBar->insertPermanentWidget(index++, widget, stretch);
	}
	
	void Workbench::showEvent(QShowEvent* event) {
		if (event->spontaneous()) return;
		taskbar_button->setWindow(windowHandle());
		
		event->accept();
	}
	
	void Workbench::UpdateView(bool is_active, const QString& label_text, int max, int val) {
		if (!is_active) {
			ui->lblProgressDescription->setText(QApplication::translate("nova/progress", "Ready"));
			ui->prbProgress->setVisible(false);
			
			taskbar_button->progress()->setVisible(false);
		} else {
			ui->lblProgressDescription->setText(label_text);
			ui->prbProgress->setVisible(true);
			ui->prbProgress->setMaximum(max);
			ui->prbProgress->setValue(val);
			
			taskbar_button->progress()->setVisible(true);
			taskbar_button->progress()->setMaximum(max);
			taskbar_button->progress()->setValue(val);
		}
	}
}
