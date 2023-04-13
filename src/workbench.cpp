/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "workbench.h"

#ifdef WIN32
	#include <windows.h>
	#include <winuser.h>
	#include <shobjidl_core.h>
#endif

#include <QtGlobal>
#include <QtVersionChecks>
#include <Qt>
#include <QSize>
#include <QDateTime>
#include <QKeySequence>
#include <QShowEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPixmap>
#include <QWindow>
#include <QScreen>
#include <QFont>
#include <QStyle>
#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QWhatsThis>
#include <QWidget>
#include <QStatusBar>
#include <QStackedWidget>
#include <QProgressBar>
#include <QLabel>

#include "ui_workbench.h"
#include "searchbar.h"
#include "toolwindow.h"
#include "settings.h"
#include "contentpage.h"

#define NOVA_CONTEXT "nova/workbench"
#define NOVA_CURRENT_VIEW_STYLESHEET "QTabBar::tab { color: palette(text); }"

// Workaround to support Qt5 and Qt6
#if WIN32
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
extern HICON qt_pixmapToWinHICON(const QPixmap& pixmap);
	#else
		#define qt_pixmapToWinHICON(pixmap) pixmap.toWinHICON()
	#endif
#endif

namespace nova {
	Workbench* workbench;
	
	Workbench::Workbench(QWidget* parent):
			QMainWindow(parent), ProgressMonitor(this), Notifier(), TempActionProvider(NOVA_TR("Tab")),
			ui(new Ui::Workbench()), menu_tray(nullptr), tool_bar_actions(ActionProvider(NOVA_TR("Tool bar"))),
			tool_window_actions(NOVA_TR("Tool window")), settings_page_actions(NOVA_TR("Settings")), root_view(nullptr),
			current_page(nullptr), current_view(nullptr), tray_icon(nullptr) {
		workbench = this;
		ui->setupUi(this);
		
		ui->statusBar->addWidget(ui->wdgNotificationBar, 3);
		ui->statusBar->addPermanentWidget(ui->wdgProgress, 1);
		
		UpdateProgressView(false, nullptr);  // Initialize the progress monitor

#ifdef WIN32
		HRESULT hresult = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList4, reinterpret_cast<void**>(&taskbar));
		if (FAILED(hresult)) {
			taskbar = nullptr;
		} else if (FAILED(taskbar->HrInit())) {
			taskbar->Release();
			taskbar = nullptr;
		}
#endif
		
		RegisterActionProvider(this);
		RegisterActionProvider(&tool_bar_actions);
		RegisterActionProvider(&tool_window_actions);
		RegisterActionProvider(&settings_page_actions);
		
		set_welcome_actions(QList<QAction*>());  // Must be called once
		
		connect(ui->lblNotificationLinks, &QLabel::linkActivated, this, &Workbench::notificationLinkActivated);
		connect(ui->lblEmptyView, &QLabel::linkActivated, this, &Workbench::lblEmptyViewLinkActivated);
		connect(this, &Workbench::currentContentPageChanged, this, &Workbench::currentContentPageChanged2);
	}
	
	Workbench::~Workbench() noexcept {
		delete ui;

#ifdef WIN32
		if (taskbar != nullptr) taskbar->Release();
#endif
	}
	
	void Workbench::OpenContentPage(ContentPage* page) {
		if (root_view == nullptr) {
			root_view = new ContentTabView(this);
			static_cast<ContentTabView*>(root_view)->setStyleSheet(NOVA_CURRENT_VIEW_STYLESHEET);
			
			ui->stwCentralWidget->addWidget(*root_view);
			ui->stwCentralWidget->setCurrentIndex(1);
		}
		
		root_view->Open(page);
	}
	
	void Workbench::OpenSettings(SettingsPage* page, QWidget* widget) {
		SettingsDialog dialog(this);
		if (page != nullptr) dialog.OpenSettingsPage(page);
		if (widget != nullptr) widget->setFocus();
		dialog.exec();
	}
	
	MenuActionProvider* Workbench::ConstructMenu(const QString& title, bool needs_tool_bar) {
		auto* menu = new MenuActionProvider(this, title, needs_tool_bar);
		RegisterActionProvider(menu);
		menuBar()->addMenu(menu);
		
		QToolBar* tool_bar = menu->get_tool_bar();
		if (tool_bar != nullptr) {
			tool_bar->setObjectName("tb" + title);  // For QMainWindow::saveState()
			
			menu->ConstructNavigationAction(&tool_bar_actions);
			addToolBar(tool_bar);
		}
		
		return menu;
	}
	
	MenuActionProvider* Workbench::ConstructMenu(Workbench::StandardMenu standard_menu, bool needs_tool_bar) {
		switch (standard_menu) {
			case Menu_File:
				standard_menus[standard_menu] = ConstructMenu(NOVA_TR("&File"), needs_tool_bar);
				return standard_menus[standard_menu];
			
			case Menu_Edit:
				standard_menus[standard_menu] = ConstructMenu(NOVA_TR("&Edit"), needs_tool_bar);
				return standard_menus[standard_menu];
			
			case Menu_Window:
				standard_menus[standard_menu] = ConstructMenu(NOVA_TR("&Window"), needs_tool_bar);
				return standard_menus[standard_menu];
			
			case Menu_Help:
				standard_menus[standard_menu] = ConstructMenu(NOVA_TR("&Help"), needs_tool_bar);
				return standard_menus[standard_menu];
			
			default:
				return nullptr;
		}
	}
	
	QAction* Workbench::ConstructStandardAction(StandardAction standard_action, ActionProvider* provider) {
		QAction* action;
		
		switch (standard_action) {
			case Action_Close:
				action = provider->ConstructAction(NOVA_TR("&Close"));
				action->setEnabled(false);
				action->setShortcut(QKeySequence("Ctrl+W"));
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) { action->setEnabled(view != nullptr); });
				connect(action, &QAction::triggered, [this]() { if (current_view != nullptr) current_view->CloseCurrent(); });
				
				break;
			
			case Action_CloseGroup:
				action = provider->ConstructAction(NOVA_TR("Close &Group"));
				action->setEnabled(false);
				action->setShortcut(QKeySequence("Ctrl+Alt+W"));
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) { action->setEnabled(view != nullptr); });
				connect(action, &QAction::triggered, [this]() { if (current_view != nullptr) current_view->Close(); });
				
				break;
			
			case Action_CloseAll:
				action = provider->ConstructAction(NOVA_TR("Close &All"));
				action->setEnabled(false);
				action->setShortcut(QKeySequence("Ctrl+Shift+W"));
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) { action->setEnabled(view != nullptr); });
				connect(action, &QAction::triggered, [this]() { if (root_view != nullptr) root_view->Close(); });
				
				break;
			
			case Action_CloseOthers:
				action = provider->ConstructAction(NOVA_TR("Close &Others"));
				action->setEnabled(false);
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) {
					action->setEnabled((view != nullptr) && (view->count() > 1));
				});
				connect(action, &QAction::triggered, [this]() { if (current_view != nullptr) current_view->CloseMultiple(true, false, true); });
				
				break;
			
			case Action_CloseTabsLeft:
				action = provider->ConstructAction(NOVA_TR("Close Tabs to the &Left"));
				action->setEnabled(false);
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) {
					action->setEnabled((view != nullptr) && (view->currentIndex() > 0));
				});
				connect(action, &QAction::triggered, [this]() { if (current_view != nullptr) current_view->CloseMultiple(true, false, false); });
				
				break;
			
			case Action_CloseTabsRight:
				action = provider->ConstructAction(NOVA_TR("Close Tabs to the &Right"));
				action->setEnabled(false);
				connect(this, &Workbench::currentContentPageChanged, [action](ContentPage*, ContentTabView* view) {
					action->setEnabled((view != nullptr) && (view->currentIndex() < (view->count() - 1)));
				});
				connect(action, &QAction::triggered, [this]() {
					if (current_view != nullptr) current_view->CloseMultiple(false, false, true);
				});
				
				break;
			
			case Action_Exit:
				action = provider->ConstructAction(NOVA_TR("&Exit"));
				action->setShortcut(QKeySequence("Ctrl+Q"));
				connect(action, &QAction::triggered, this, &Workbench::close);
				
				break;
			
			case Action_Settings:
				action = provider->ConstructAction(NOVA_TR("&Settings"));
				action->setShortcut(QKeySequence("Ctrl+Shift+S"));
				connect(action, &QAction::triggered, [this]() { OpenSettings(nullptr); });
				
				break;
			
			case Action_RestoreLayout:
				action = provider->ConstructAction(NOVA_TR("Restore &Default Layout"));
				connect(action, &QAction::triggered, [this]() { RestoreLayout(); });
				
				break;
			
			case Action_DirectHelp:
				action = provider->ConstructAction(NOVA_TR("&Direct Help"));
				action->setShortcut(QKeySequence("F2"));
				connect(action, &QAction::triggered, []() { QWhatsThis::enterWhatsThisMode(); });
				
				break;
			
			case Action_SearchBar:
				action = provider->ConstructAction(NOVA_TR("&Search..."));
				action->setShortcut(QKeySequence("F3"));
				connect(action, &QAction::triggered, [this]() {
					SearchBar bar(this);
					bar.exec();
				});
				
				break;
			
			default:
				return nullptr;
		}
		
		standard_actions[standard_action] = action;
		return action;
	}
	
	void Workbench::AddStatusBarWidget(QWidget* widget, int stretch) {
		// Guarantee the widget to be inserted in front of the progress indicator
		static int index = 1;
		ui->statusBar->insertPermanentWidget(index++, widget, stretch);
	}
	
	QSystemTrayIcon* Workbench::ConstructSystemTrayIcon() {
		tray_icon = new QSystemTrayIcon(QApplication::windowIcon(), this);
		menu_tray = new MenuActionProvider(this, NOVA_TR("Tray Icon"), false);
		RegisterActionProvider(menu_tray);
		
		tray_icon->setContextMenu(menu_tray);
		tray_icon->show();
		
		connect(tray_icon, &QSystemTrayIcon::activated, this, &Workbench::sysTrayActivated);
		connect(tray_icon, &QSystemTrayIcon::messageClicked, this, [this] { sysTrayActivated(); });
		
		return tray_icon;
	}
	
	void Workbench::RestoreLayout() {
		for (QToolBar* i : findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly)) {
			// Remove and add the tool bars to reset their layouts
			removeToolBar(i);
			
			addToolBar(i);
			i->show();  // Because removeToolBar() hides it
		}
		
		for (ToolWindow* i : tool_windows) {
			i->hide();
			i->setFloating(false);
			removeDockWidget(i);
			addDockWidget(i->default_layout, i);
			
			// Resize the tool window (Note: the orientation of resizeDocks() specifies if the width or the height should be taken, so it's just swapped)
			resizeDocks({i}, {i->initial_size}, (i->get_orientation() == Qt::Vertical) ? Qt::Horizontal : Qt::Vertical);
			
			if (!i->default_hidden) i->show();
		}
		
		// Reset the geometry
		setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(920, 640), screen()->availableGeometry()));
	}
	
	void Workbench::showEvent(QShowEvent* event) {
		QMainWindow::showEvent(event);
		
		if (event->spontaneous()) {
			event->accept();
			return;
		}
		
		// Showing the window might change some settings (e.g. geometry)
		Properties parameters;
		parameters["workbench"] = reinterpret_cast<quintptr>(this);
		for (SettingsPage* i : settings_pages) {
			i->RecreateActions(parameters);
		}
		
		event->accept();
	}
	
	void Workbench::keyPressEvent(QKeyEvent* event) {
		QMainWindow::keyPressEvent(event);
		
		// Double shift shortcut for the search bar action
		if ((standard_actions[Action_SearchBar] != nullptr) && (event->key() == Qt::Key_Shift)) {
			static qint64 remaining_time = 0;
			
			if (QDateTime::currentMSecsSinceEpoch() > remaining_time) {
				// First click
				remaining_time = QDateTime::currentMSecsSinceEpoch() + 500;
			} else {
				// Second click
				standard_actions[Action_SearchBar]->trigger();
				remaining_time = 0;
			}
		}
	}
	
	void Workbench::UpdateProgressView(bool is_active, const Task* task) {
		if (!is_active) {
			ui->lblProgressDescription->setText(NOVA_TR("Ready"));
			ui->prbProgress->setVisible(false);

#ifdef WIN32
			if ((taskbar != nullptr) && (windowHandle() != nullptr)) {
				HWND native_window = reinterpret_cast<HWND>(windowHandle()->winId());
				taskbar->SetProgressState(native_window, TBPF_NOPROGRESS);
			}
#endif
		} else {
			// pb_maximum is 0 when the task is indeterminate, else 100%->100
			const int pb_maximum = task->is_indeterminate() ? 0 : 100;
			
			ui->lblProgressDescription->setText(task->get_task_name() + "...");
			ui->prbProgress->setVisible(true);
			ui->prbProgress->setMaximum(pb_maximum);
			ui->prbProgress->setValue(task->get_value());

#ifdef WIN32
			if ((taskbar != nullptr) && (windowHandle() != nullptr)) {
				HWND native_window = reinterpret_cast<HWND>(windowHandle()->winId());
				taskbar->SetProgressState(native_window, (task->is_indeterminate() ? TBPF_INDETERMINATE : TBPF_NORMAL));
				if (!task->is_indeterminate()) taskbar->SetProgressValue(native_window, task->get_value(), 100);
			}
#endif
		}
	}
	
	void Workbench::UpdateNotificationView(bool is_active, const Notification* notification) {
		QIcon icon;
		if (is_active) {
			icon = Notification::ConvertToIcon(notification->get_type());
			ui->lblNotification->setText(notification->get_title() + ": " + notification->get_message());
			ui->lblNotificationIcon->setPixmap(icon.pixmap(16, 16));
			ui->lblNotificationLinks->setText(notification->CreateLinksLabelText());
		}

#ifdef WIN32
		if ((taskbar != nullptr) && (windowHandle() != nullptr)) {
			HWND native_window = reinterpret_cast<HWND>(windowHandle()->winId());
			
			HICON native_icon = nullptr;
			if (is_active && !icon.isNull()) {
				native_icon = qt_pixmapToWinHICON(icon.pixmap(GetSystemMetrics(SM_CXSMICON)));
			}
			
			taskbar->SetOverlayIcon(native_window, native_icon, nullptr);
			DestroyIcon(native_icon);
		}
#endif
		
		ui->wdgNotificationBar->setVisible(is_active);
	}
	
	void Workbench::ShowNotificationPopup(const Notification* notification) {
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
					break;
			}
			
			tray_icon->showMessage(notification->get_title(), notification->get_message(), icon);
		}
		
		if (notification->is_high_priority()) {
			QApplication::beep();
			QApplication::alert(ui->wdgNotificationBar);
		}
	}
	
	void Workbench::set_welcome_actions(const QList<QAction*>& welcome_actions) {
		this->welcome_actions = welcome_actions;
		
		// Display all available actions again
		QString markdown = ("## " + QApplication::applicationDisplayName() + "\n\n");
		
		for (int i = 0 ; i < welcome_actions.count() ; ++i) {
			QAction* action = welcome_actions[i];
			markdown += ("[" + action->toolTip()
			             + (action->shortcut().isEmpty() ? "" : (" [" + action->shortcut().toString() + "]"))
			             + "](" + QString::number(i) + ")\n\n");
		}
		
		markdown += NOVA_TR("(Drag tabs here to open)");
		ui->lblEmptyView->setText(markdown);
	}
	
	void Workbench::RecreateActions(const Properties& creation_parameters) {
		ClearActions();
		
		// Create a navigation page for every tab
		if (root_view != nullptr) {
			for (ContentPage* i : root_view->ListPages()) {
				QAction* action = ConstructAction(i->get_title());
				action->setIcon(i->get_icon());
				
				connect(action, &QAction::triggered, [i]() { i->Activate(); });
			}
		}
	}
	
	void Workbench::currentContentPageChanged2(ContentPage* page, ContentTabView* view) {
		// Since there's only one ContentPage provider at time, it's always at the first position in the list.
		static bool has_old = false;
		if (has_old) providers.removeAt(0);
		
		if (view == nullptr) {
			ui->stwCentralWidget->setCurrentIndex(0);
			ui->stwCentralWidget->removeWidget(*root_view);
			
			root_view = nullptr;  // The view deletes itself
			has_old = false;
		} else {
			providers.insert(0, page);
			has_old = true;
		}
		
		current_page = page;
		current_view = view;
		
		RecreateActions();
	}
	
	void Workbench::sysTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason != QSystemTrayIcon::Trigger) return;
		// Restore window when minimized
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		raise();
		activateWindow();
	}
	
	void Workbench::notificationLinkActivated(const QString& link) {
		ActivateNotificationAction(link);
	}
	
	void Workbench::lblEmptyViewLinkActivated(const QString& link) {
		welcome_actions[link.toInt()]->trigger();
	}
}
