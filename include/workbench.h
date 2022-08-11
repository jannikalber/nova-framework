/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include <QtGlobal>
#include <QObject>
#include <QList>
#include <QString>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "nova.h"
#include "actionprovider.h"
#include "progress.h"
#include "toolwindow.h"
#include "notification.h"
#include "settings.h"

class QWidget;
class QShowEvent;
class QKeyEvent;
class QAction;

#ifdef WIN32
struct ITaskbarList4;
#endif

namespace Ui { class Workbench; }

namespace nova {
	class MenuActionProvider;
	class SearchBar;
}

namespace nova {
	/**
	 * @brief This class represents the main window of the application.
	 * @headerfile workbench.h <nova/workbench.h>
	 *
	 * The workbench has a prefabricated Ui layout. Its content is a tab widget. You can add nova::ContentPage
	 * objects which can be displayed in this widget. The window also contains areas for nova::ToolWindow objects,
	 * menus and a status bar which can be extended too.
	 *
	 * The workbench is a nova::ProgressMonitor and a nova::Notifier too.
	 *
	 * Only one workbench window should exist per application.
	 *
	 * The translations belong to the context "nova/workbench".
	 *
	 * This class must be derived.
	 *
	 * @sa nova::workbench
	 */
	class NOVA_API Workbench : public QMainWindow, public ProgressMonitor, public Notifier {
		Q_OBJECT
		
		public:
			/**
			 * @brief A list of standard menus being available
			 *
			 * @sa ConstructMenu(StandardMenu)
			 */
			enum StandardMenu {  // Array length below must be up-to-date
				//! File (title: "&File")
				Menu_File,
				//! Edit (title: "&Edit")
				Menu_Edit,
				//! Window (title: "&Window")
				Menu_Window,
				//! Help (title: "&Help")
				Menu_Help
			};
			
			/**
			 * @brief A list of standard actions being fully implemented
			 *
			 * @sa ConstructStandardAction()
			 */
			enum StandardAction {  // Array length below must be up-to-date
				Action_Exit,
				//! "Settings" to open the (automatically available) settings dialog [Ctrl+Shift+S] (title: "&Settings")
				Action_Settings,
				//! "Restore Default Layout" for resetting all tool bars and tool windows to their default position
				Action_RestoreLayout,
				//! "Direct Help" to enable QWhatsThis [F2] (title: "&Direct Help")
				Action_DirectHelp,
				//! "Search Bar" for browsing the application's actions [F3] | [double shift] (title: "&Search...")
				Action_SearchBar
			};
			
			NOVA_DISABLE_COPY(Workbench)
			virtual ~Workbench() noexcept;
			
			/**
			 * @brief Adds a nova::ActionProvider to the workbench's provider list.
			 *
			 * Its actions will be available in nova::SearchBar once registered.
			 *
			 * Usually, if you add content using one of the register methods, their providers
			 * get automatically registered and calling this method is not required.
			 *
			 * @param provider The nova::ActionProvider to be registered
			 *
			 * @sa UnregisterActionProvider()
			 */
			inline void RegisterActionProvider(ActionProvider* provider) {
				providers << provider;
			}
			
			/**
			 * @brief Unregisters a nova::ActionProvider.
			 *
			 * Its actions won't be available anymore in nova::SearchBar once
			 * unregistered.
			 *
			 * @param provider The nova::ActionProvider to be unregistered
			 *
			 * @sa RegisterActionProvider()
			 */
			inline void UnregisterActionProvider(ActionProvider* provider) {
				providers.removeAll(provider);
			}
			
			/**
			 * @brief Adds a nova::ToolWindow class to the workbench.
			 *
			 * The tool window will be shown and its actions can be found
			 * using nova::SearchBar once registered.
			 *
			 * The subclass must have a constructor with QWidget* as parameter (the tool window's parent window)
			 *
			 * @tparam T The class to be registered
			 * @return The instance of the tool window being created
			 */
			template<class T>
			T* RegisterToolWindow() {
				ToolWindow* tool_window = new T(static_cast<QWidget*>(this));
				
				RegisterActionProvider(tool_window);
				tool_windows << tool_window;
				tool_window->ConstructNavigationAction(&tool_window_actions);
				addDockWidget(tool_window->default_layout, tool_window);
				
				return static_cast<T*>(tool_window);
			}
			
			/**
			 * @brief Adds a nova::SettingsPage class to the workbench.
			 *
			 * The settings page will be shown in nova::SettingsDialog and its actions can be found
			 * using nova::SearchBar once registered.
			 *
			 * The subclass must have a constructor with QObject* as parameter (the page's parent)
			 *
			 * @tparam T The class to be registered
			 * @return The instance of the settings page being created
			 *
			 * @sa OpenSettings()
			 */
			template<class T>
			T* RegisterSettingsPage() {
				SettingsPage* settings_page = new T(static_cast<QObject*>(this));
				
				Properties parameters;
				parameters["workbench"] = reinterpret_cast<quintptr>(this);
				settings_page->RecreateActions(parameters);  // Associated workbench as parameter
				
				RegisterActionProvider(settings_page);
				settings_pages << settings_page;
				settings_page->ConstructNavigationAction(&settings_page_actions, this);
				
				return static_cast<T*>(settings_page);
			}
			
			/**
			 * @brief Starts nova::SettingsDialog and opens a specific page.
			 *
			 * @param page The page which is opened first (optional, default: the first one)
			 * @param widget The widget to be focused (optional, default: none)
			 *
			 * @sa nova::SettingsDialog
			 */
			void OpenSettings(SettingsPage* page = nullptr, QWidget* widget = nullptr);
			
			/**
			 * @brief Returns the given standard menu which was created using ConstructMenu(StandardMenu).
			 *
			 * @return The menu or nullptr if the menu is never constructed
			 * @sa ConstructMenu()
			 */
			inline MenuActionProvider* get_standard_menu(StandardMenu standard_menu) const { return standard_menus[standard_menu]; }
			
			/**
			 * @brief Returns the given standard action which was created using ConstructStandardAction().
			 *
			 * @return The action or nullptr if the action is never constructed
			 * @sa ConstructStandardAction()
			 */
			inline QAction* get_standard_action(StandardAction standard_action) const { return standard_actions[standard_action]; }
			
			/**
			 * @brief Returns a list of all action providers being associated with this workbench.
			 *
			 * @sa nova::ActionProvider
			 * @sa RegisterActionProvider()
			 */
			inline QList<ActionProvider*> get_action_providers() const { return providers; }
			
			/**
			 * @brief Returns a list of all tool windows being associated with this workbench.
			 *
			 * @sa nova::ToolWindow
			 * @sa RegisterToolWindow()
			 */
			inline QList<ToolWindow*> get_tool_windows() const { return tool_windows; }
			
			/**
			 * @brief Returns a list of all settings pages being associated with this workbench.
			 *
			 * @sa nova::SettingsPage
			 * @sa RegisterSettingsPage()
			 */
			inline QList<SettingsPage*> get_settings_pages() const { return settings_pages; }
			
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
		
		protected:
			/**
			 * @brief Creates a new nova::Workbench.
			 *
			 * The constructor should only be called once per application.
			 * Calling this constructor automatically updates the reference of nova::workbench.
			 *
			 * @param parent The parent window (optional, default: none)
			 *
			 * @sa nova::workbench
			 */
			explicit Workbench(QWidget* parent = nullptr);
			
			/**
			 * @brief Creates a new menu and displays it in the menu bar.
			 *
			 * The order of the menu entries is analog to the calls' order of this method.
			 *
			 * @param title The menu's title shown in the menu bar (can contain the hotkey character "&")
			 * @param needs_tool_bar if a tool bar should be created and shown too
			 *
			 * @return A pointer to the created menu
			 */
			MenuActionProvider* ConstructMenu(const QString& title, bool needs_tool_bar = false);
			
			/**
			 * @brief Creates one of the standard menus and displays it in the menu bar.
			 *
			 * @param standard_menu specifies which menu should be created
			 * @param needs_tool_bar if a tool bar should be created and shown too
			 *
			 * @return A pointer to the created menu or nullptr if the parameter standard_menu is invalid.
			 *
			 * @sa StandardMenu to see a list of all available menus
			 * @sa ConstructMenu()
			 * @sa get_standard_menu()
			 */
			MenuActionProvider* ConstructMenu(StandardMenu standard_menu, bool needs_tool_bar = false);
			
			/**
			 * @brief Creates one of the standard actions.
			 *
			 * Standard actions are fully implemented and ready to use.
			 *
			 * @param standard_action specifies which action should be created
			 * @param provider is the nova::ActionProvider the action gets associated with
			 *
			 * @return A pointer to the created action or nullptr if the parameter standard_action is invalid.
			 *
			 * @sa StandardAction to see a list of all available actions
			 */
			QAction* ConstructStandardAction(StandardAction standard_action, ActionProvider* provider);
			
			/**
			 * @brief Inserts a widget into the application's status bar.
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
			 * The tray icon has a context menu which is a normal nova::MenuActionProvider with the title "Tray Icon".
			 *
			 * Clicking the icon will activate the window. In addition, notifications will be shown by the tray icon.
			 *
			 * @return A pointer to the created tray icon
			 *
			 * @sa get_system_tray_icon()
			 * @sa get_system_tray_menu()
			 */
			QSystemTrayIcon* ConstructSystemTrayIcon();
			
			/**
			 * @brief Resets all tool windows and tool bars to their default position.
			 *
			 * Tool bars and tool windows save their default position after creation.
			 *
			 * The workbench's geometry is restored too.
			 */
			void RestoreLayout();
			
			/**
			 * @brief Please do always call this implementation when overriding.
			 *
			 * This method is internally required.
			 */
			void showEvent(QShowEvent* event) override;
			
			/**
			 * @brief Please do always call this implementation when overriding.
			 *
			 * This method is internally required.
			 */
			void keyPressEvent(QKeyEvent* event) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void UpdateProgressView(bool is_active, const Task* task) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void UpdateNotificationView(bool is_active, const Notification* notification) override;
			
			/**
			 * @brief A popup is only shown if the workbench has a tray icon.
			 *
			 * This method is internally required and should not be called.
			 */
			void ShowNotificationPopup(const Notification* notification) override;
		
		private:
			friend class SearchBar;
			friend class SettingsDialog;
			
			Ui::Workbench* const ui;
			
			MenuActionProvider* standard_menus[4] = {};  // Array length must be up-to-date
			QAction* standard_actions[5] = {};  // Array length must be up-to-date
			MenuActionProvider* menu_tray;
			
			ActionProvider tool_bar_actions;
			ActionProvider tool_window_actions;
			ActionProvider settings_page_actions;
			
			QList<ActionProvider*> providers;
			QList<ToolWindow*> tool_windows;
			QList<SettingsPage*> settings_pages;
			
			QSystemTrayIcon* tray_icon;

#ifdef WIN32
			ITaskbarList4* taskbar;
#endif
			
		private slots:
			void sysTrayActivated(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::Trigger);
			void notificationLinkActivated(const QString& link);
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
