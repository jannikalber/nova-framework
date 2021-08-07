/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSystemTrayIcon>

#include "nova.h"
#include "progress.h"
#include "toolwindow.h"
#include "notification.h"
#include "settings.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QString;
class QWidget;
class QShowEvent;
class QAction;
class QWinTaskbarButton;
class QWinTaskbarProgress;

namespace Ui { class Workbench; }
QT_END_NAMESPACE

namespace nova {
	class ActionProvider;
	class MenuActionProvider;
	class SearchBar;
	
	/**
	 * @brief This class represents the main window of the application.
	 * @headerfile workbench.h <nova/workbench.h>
	 *
	 * The workbench has a prefabricated Ui layout. Its content is a tab widget. One can add content types
	 * which can be displayed in this widget. The window also contains areas for tool windows, menus and
	 * a status bar which can be extended too.
	 *
	 * Only one workbench window should be constructed in one application.
	 *
	 * This class should be derived.
	 *
	 * @sa nova::workbench
	 */
	class NOVA_API Workbench : public QMainWindow, public ProgressMonitor, public Notifier {
		Q_OBJECT
		
		public:
			/**
			 * @brief A list of menus needed in nearly every application:
			 *
			 * - Menu_File (title: "&File")
			 * - Menu_Edit (title: "&Edit")
			 * - Menu_Window (title: "&Window")
			 * - Menu_Help (title: "&Help")
			 *
			 * The translations belong to the context "nova/menu".
			 *
			 * @sa ConstructMenu(StandardMenu)
			 */
			enum StandardMenu {
				Menu_File, Menu_Edit, Menu_Window, Menu_Help
			};
			
			/**
			 * @brief A list of standard actions which are handled by Nova:
			 *
			 * - Action_Exit to exit the application [Ctrl+Q] (title: "&Exit")
			 * - Action_Settings to open the (automatically available) settings dialog [Ctrl+Shift+S] (title: "&Settings")
			 * - Action_ResetLayout for resetting all tool bars and tool windows to their default position
			 * (title: "Restore &Default Layout")
			 * - Action_DirectHelp to enable QWhatsThis [F2] (title: "&Direct Help")
			 * - Action_SearchBar for browsing the application's actions [F3] (title: "&Search...")
			 *
			 * The translations belong to the context "nova/action".
			 *
			 * @sa ConstructStandardAction()
			 */
			enum StandardAction {
				Action_Exit, Action_Settings, Action_ResetLayout, Action_DirectHelp, Action_SearchBar
			};
			
			/**
			 * Constructs a new workbench. The constructor should only be called once in one application.
			 * Calling this constructor updates automatically the reference of nova::workbench.
			 *
			 * A ProgressMonitor is inserted in the workbench's status bar.
			 *
			 * @param parent The parent window which blocks its input until the workbench window is closed (optional, default: none)
			 * @sa nova::workbench
			 */
			explicit Workbench(QWidget* parent = nullptr);
			Workbench(const Workbench&) = delete;
			Workbench(Workbench&&) = delete;
			
			/**
			 * Destructs the workbench and also its QObject children.
			 */
			virtual ~Workbench() noexcept;
			
			/**
			 * @brief Constructs a new menu and displays it in the menu bar.
			 *
			 * The order of the menu entries is analog to the calls' order of this method.
			 *
			 * @param title The menu's title shown in the menu bar (it might contain the hotkey character "&")
			 * @param needs_tool_bar if a tool bar should be created and shown too
			 *
			 * @return A pointer to the created menu
			 */
			MenuActionProvider* ConstructMenu(const QString& title, bool needs_tool_bar = false);
			
			/**
			 * @brief Constructs one of the standard menus and displays it in the menu bar.
			 *
			 * This function overloads ConstructMenu()
			 *
			 * @param standard_menu specifies which menu should be created
			 * @param needs_tool_bar if a tool bar should be created and shown too
			 *
			 * @return A pointer to the created menu or nullptr if standard_menu is invalid
			 *
			 * @sa ConstructMenu()
			 * @sa get_standard_menu()
			 */
			MenuActionProvider* ConstructMenu(StandardMenu standard_menu, bool needs_tool_bar = false);
			
			/**
			 * @brief Constructs one of the standard actions and handles its functionality.
			 *
			 * @param standard_action specifies which action should be created.
			 * @param provider is the ActionProvider the action gets associated with.
			 * @return A pointer to the created action or nullptr if standard_action is invalid
			 *
			 * @sa StandardAction to see a list of available actions
			 */
			QAction* ConstructStandardAction(StandardAction standard_action, ActionProvider* provider);
			
			/**
			 * @brief Inserts a widget into the program's status bar.
			 *
			 * The widget is always inserted in front of the progress monitor.
			 *
			 * @param widget The widget to be inserted
			 * @param stretch The stretch factor (optional, default: 1)
			 *
			 * @sa QStatusBar::addPermanentWidget()
			 */
			void AddStatusBarWidget(QWidget* widget, int stretch = 1);
			
			/**
			 * @brief Creates and shows an icon in the system's tray menu.
			 *
			 * The tray icon has a context menu which is a normal action provider with the title "Tray Icon"
			 * (translation context: "nova/menu").
			 *
			 * Clicking the icon will restore the window.
			 *
			 * Further notifications are shown by the tray icon.
			 *
			 * @return A pointer to the created tray icon
			 */
			QSystemTrayIcon* ConstructSystemTrayIcon();
			
			/**
			 * @brief Adds an ActionProvider to the workbench's provider list.
			 *
			 * Its actions can be found using the SearchBar once their provider is registered.
			 *
			 * @param provider The ActionProvider to be registered
			 */
			inline void RegisterActionProvider(ActionProvider* provider) {
				providers << provider;
			}
			
			/**
			 * @brief Adds a ToolWindow class to the workbench.
			 *
			 * The ToolWindow is shown and available once registered.
			 *
			 * The subclass must have a constructor with QWidget* as parameter (the tool window's parent window)
			 *
			 * @tparam T The class to be registered
			 */
			template<class T>
			void RegisterToolWindow() {
				ToolWindow* tool_window = new T(static_cast<QWidget*>(this));
				
				RegisterActionProvider(tool_window);
				tool_windows << tool_window;
				
				addDockWidget(tool_window->default_layout, tool_window);
			}
			
			/**
			 * @brief Adds a SettingsPage class to the workbench.
			 *
			 * The SettingsPage will be shown in the SettingsDialog and can be found using the SearchBar.
			 *
			 * The subclass must have a constructor with QObject* as parameter (the page's parent)
			 *
			 * @tparam T The class to be registered
			 * @sa OpenSettings()
			 */
			template<class T>
			void RegisterSettingsPage() {
				SettingsPage* settings_page = new T(static_cast<QObject*>(this));
				settings_page->RecreateActions(this);  // Associated workbench as parameter
				
				RegisterActionProvider(settings_page);
				settings_pages << settings_page;
			}
			
			/**
			 * @brief Starts a SettingsDialog with the workbench's SettingsPage objects being registered.
			 *
			 * @param page Which page should be opened first (optional, default: the first one)
			 * @sa SettingsDialog
			 */
			void OpenSettings(SettingsPage* page = nullptr);
			
			/**
			 * @brief Returns the given standard menu which was created using ConstructMenu(StandardMenu).
			 *
			 * @return The menu or nullptr if the menu is never constructed
			 * @sa ConstructMenu()
			 */
			MenuActionProvider* get_standard_menu(StandardMenu standard_menu) const;
			
			/**
			 * @brief Returns a pointer to the window's taskbar button.
			 */
			inline QWinTaskbarButton* get_taskbar_button() const { return taskbar_button; }
			
			/**
			 * @brief Returns a pointer to the workbench's icon in the system's tray menu.
			 *
			 * @return The icon or nullptr if the icon is never constructed
			 *
			 * @sa ConstructSystemTrayIcon()
			 */
			inline QSystemTrayIcon* get_system_tray_icon() const { return tray_icon; }
			
			/**
			 * @brief Returns a pointer to the tray icon's context menu.
			 *
			 * @return The menu or nullptr if the icon is never constructed
			 *
			 * @sa ConstructSystemTrayIcon()
			 * @sa get_system_tray_menu()
			 */
			inline MenuActionProvider* get_system_tray_menu() const { return menu_tray; }
			
			/**
			 * @brief Returns a list of all action providers being associated with this workbench.
			 *
			 * @sa ActionProvider
			 * @sa RegisterActionProvider()
			 */
			inline QList<ActionProvider*> get_action_providers() const { return providers; }
			
			/**
			 * @brief Returns a list of all tool windows being associated with this workbench.
			 *
			 * @sa ToolWindow
			 * @sa RegisterToolWindow()
			 */
			inline QList<ToolWindow*> get_tool_windows() const { return tool_windows; };
			
			/**
			 * @brief Returns a list of all settings pages being associated with this workbench.
			 *
			 * @sa SettingsPage
			 * @sa RegisterSettingsPage()
			 */
			inline QList<SettingsPage*> get_settings_pages() const { return settings_pages; };
		
		protected:
			/**
			 * @brief Resets all tool windows and tool bars to their default position.
			 *
			 * This method can be overridden to add extra functionality.
			 *
			 * @sa ToolWindow::ResetLayout()
			 */
			virtual void ResetLayout();
			
			/**
			 * Reimplements QWidget::showEvent()
			 *
			 * Please do always call this implementation when overriding.
			 */
			void showEvent(QShowEvent* event) override;
			
			/**
			 * Reimplements ProgressMonitor::UpdateProgressView()
			 */
			void UpdateProgressView(bool is_active, Task* task) override;
			
			/**
			 * Reimplements Notifier::UpdateNotificationView()
			 */
			void UpdateNotificationView(bool is_active, Notification* notification) override;
			
			/**
			 * Reimplements Notifier::ShowNotificationPopup()
			 *
			 * A popup is only shown if the workbench has a tray icon.
			 */
			void ShowNotificationPopup(Notification* notification) override;
		
		private:
			friend class SearchBar;
			friend class SettingsDialog;
			
			Ui::Workbench* ui;
			
			MenuActionProvider* menu_file;
			MenuActionProvider* menu_edit;
			MenuActionProvider* menu_window;
			MenuActionProvider* menu_help;
			
			MenuActionProvider* menu_tray;
			
			QList<ActionProvider*> providers;
			QList<ToolWindow*> tool_windows;
			QList<SettingsPage*> settings_pages;
			
			QWinTaskbarButton* taskbar_button;
			QSystemTrayIcon* tray_icon;
		
		private slots:
			void sysTrayActivated(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::Trigger);
			void notificationLinkActivated(const QString& link);
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
