/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#include <QtGui/QKeySequence>
#include <QtGui/QShowEvent>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
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
			: QMainWindow(parent), ProgressMonitor(this), Notifier(), ui(new Ui::Workbench()), menu_file(nullptr),
			  menu_edit(nullptr), menu_help(nullptr), menu_tray(nullptr), providers(QList<ActionProvider*>()),
			  taskbar_button(new QWinTaskbarButton(this)), tray_icon(nullptr) {
		workbench = this;
		ui->setupUi(this);
		
		ui->statusBar->addWidget(ui->wdgNotificationBar, 3);
		ui->statusBar->addPermanentWidget(ui->wdgProgress, 1);
		
		connect(ui->lblNotificationLinks, &QLabel::linkActivated, this, &Workbench::notificationLinkActivated);
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
	
	void Workbench::AddStatusBarWidget(QWidget* widget, int stretch) {
		// Guarantee the widget to be inserted in front of the progress indicator
		static int index = 1;
		ui->statusBar->insertPermanentWidget(index++, widget, stretch);
	}
	
	QSystemTrayIcon* Workbench::ConstructSystemTrayIcon() {
		tray_icon = new QSystemTrayIcon(QApplication::windowIcon(), this);
		menu_tray = new MenuActionProvider(QApplication::translate("nova/menu", "Tray Icon"), this);
		
		tray_icon->setContextMenu(menu_tray);
		tray_icon->show();
		
		connect(tray_icon, &QSystemTrayIcon::activated, this, &Workbench::sysTrayActivated);
		connect(tray_icon, &QSystemTrayIcon::messageClicked, this, [this] { sysTrayActivated(); });
		
		return tray_icon;
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
	
	void Workbench::showEvent(QShowEvent* event) {
		if (event->spontaneous()) return;
		taskbar_button->setWindow(windowHandle());
		
		event->accept();
	}
	
	void Workbench::UpdateProgressView(bool is_active, Task* task) {
		if (!is_active) {
			ui->lblProgressDescription->setText(QApplication::translate("nova/progress", "Ready"));
			ui->prbProgress->setVisible(false);
			
			taskbar_button->progress()->setVisible(false);
		} else {
			// pb_maximum is 0 when the task is indeterminate, else 100%->100
			const int pb_maximum = task->is_indeterminate() ? 0 : 100;
			
			ui->lblProgressDescription->setText(task->get_task_name() + "...");
			ui->prbProgress->setVisible(true);
			ui->prbProgress->setMaximum(pb_maximum);
			ui->prbProgress->setValue(task->get_value());
			
			taskbar_button->progress()->setVisible(true);
			taskbar_button->progress()->setMaximum(pb_maximum);
			taskbar_button->progress()->setValue(task->get_value());
		}
	}
	
	void Workbench::UpdateNotificationView(bool is_active, Notification* notification) {
		if (is_active) {
			const QIcon& icon = Notification::ConvertToIcon(notification->get_type());
			ui->lblNotification->setText(notification->get_title() + ": " + notification->get_message());
			ui->lblNotificationIcon->setPixmap(icon.pixmap(16, 16));
			ui->lblNotificationLinks->setText(notification->CreateLinksLabelText());
			
			taskbar_button->setOverlayIcon(icon);
		} else taskbar_button->clearOverlayIcon();
		
		ui->wdgNotificationBar->setVisible(is_active);
	}
	
	void Workbench::ShowNotificationPopup(Notification* notification) {
		if (tray_icon != nullptr) {
			QSystemTrayIcon::MessageIcon icon;
			switch (notification->get_type()) {
				case Notification::Information:
					icon = QSystemTrayIcon::Information;
					break;
				
				case Notification::Warning:
					icon = QSystemTrayIcon::Warning;
					break;
				
				case Notification::Error:
					icon = QSystemTrayIcon::Critical;
			}
			
			tray_icon->showMessage(notification->get_title(), notification->get_message(), icon);
		}
		
		if (notification->is_high_priority()) {
			QApplication::beep();
			QApplication::alert(ui->wdgNotificationBar);
		}
	}
	
	void Workbench::sysTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason != QSystemTrayIcon::Trigger) return;
		// Restore window when minimized
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	}
	
	void Workbench::notificationLinkActivated(const QString& link) {
		ActivateNotificationAction(link);
	}
}
