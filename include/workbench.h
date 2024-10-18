/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include <QtGlobal>
#include <Qt>
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
class QCloseEvent;
class QKeyEvent;
class QAction;

#ifdef WIN32
struct ITaskbarList4;
#endif

namespace Ui { class Workbench; }

namespace nova {
	class ContentPage;
	class ContentView;
	class ContentTabView;
	class ContentSplitView;
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
	class NOVA_API Workbench : public QMainWindow, public ProgressMonitor, public Notifier,
	                           private TempActionProvider {  // TempActionProvider for the navigation actions of the content pages
		Q_OBJECT
		
		public:
			/**
			 * @brief A list of standard menus being available
			 *
			 * @sa ConstructMenu(StandardMenu, bool)
			 */
			enum StandardMenu {  // Array length below must be up-to-date
				//! File (title: "&File")
				Menu_File,
				//! Edit (title: "&Edit")
				Menu_Edit,
				//! View (title: "&View")
				Menu_View,
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
				//! "Close" to close the current content page [Ctrl+W] (title: "&Close")
				Action_Close,
				//! "Close Group" to close the whole group of content pages being active [Ctrl+Alt+W] (title: "Close &Group")
				Action_CloseGroup,
				//! "Close All" to close all content pages [Ctrl+Shift+W] (title: "Close &All")
				Action_CloseAll,
				//! "Close Others" to close all tabs in the current group except the current one (title: "Close &Others")
				Action_CloseOthers,
				//! "Close Tabs to the Left" to close all tabs to the left of the current one (title: "Close Tabs to the &Left")
				Action_CloseTabsLeft,
				//! "Close Tabs to the Right" to close all tabs to the right of the current one (title: "Close Tabs to the &Right")
				Action_CloseTabsRight,
				//! "Exit" to exit the application [Ctrl+Q] (title: "&Exit")
				Action_Exit,
				//! "Settings" to open the (automatically created) settings dialog [Ctrl+Shift+S] (title: "&Settings")
				Action_Settings,
				//! "Split Right" to split the current content page to the right (title: "Split &Right")
				Action_SplitRight,
				//! "Split Down" to split the current content page down (title: "Split &Down")
				Action_SplitDown,
				//! "Restore Default Layout" for resetting all tool bars and tool windows to their default position
				Action_RestoreLayout,
				//! "Direct Help" to enable QWhatsThis [F2] (title: "&Direct Help")
				Action_DirectHelp,
				//! "Search Bar" for browsing the application's actions [F3] | [double Shift] (title: "&Search...")
				Action_SearchBar,
				//! "Switcher" to navigate through the tool windows and content pages [Ctrl+Tab] (title: "&Switcher...")
				Action_Switcher
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
			 * The subclass must have a constructor with nova::Workbench* as parameter. The tool window
			 * can only be registered to one workbench.
			 *
			 * @tparam T The nova::ToolWindow class to be registered
			 * @return The instance of the tool window being created
			 */
			template<class T>
			T* RegisterToolWindow() {
				ToolWindow* tool_window = new T(this);
				
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
			 * The subclass must have a constructor with nova::Workbench* as parameter. The page can
			 * only be registered to one workbench.
			 *
			 * @tparam T The nova::SettingsPage class to be registered
			 * @return The instance of the settings page being created
			 *
			 * @sa OpenSettings()
			 */
			template<class T>
			T* RegisterSettingsPage() {
				SettingsPage* settings_page = new T(this);
				
				settings_page->RecreateActions();
				
				RegisterActionProvider(settings_page);
				settings_pages << settings_page;
				settings_page->ConstructNavigationAction(&settings_page_actions, this);
				
				return static_cast<T*>(settings_page);
			}
			
			/**
			 * @brief Opens the given nova::ContentPage.
			 *
			 * The content page is added to the active content tab view. If there's no
			 * active tab view, a new one is created.
			 *
			 * @param page The page to be opened.
			 *
			 * @sa nova::ContentPage
			 * @sa get_current_page()
			 */
			void OpenContentPage(ContentPage* page);
			
			/**
			 * @brief Moves a content page from one view to another (and activates it).
			 *
			 * @param page The page to be moved
			 * @param target The new target view
			 */
			void MoveContentPage(ContentPage* page, ContentTabView* target);
			
			/**
			 * @brief Tries to close all opened content pages.
			 *
			 * @return true if all pages were closed successfully
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa nova::ContentTabView::Close(int)
			 * @sa nova::ContentTabView::Close(nova::ContentPage*)
			 * @sa nova::ContentTabView::CloseCurrent()
			 * @sa nova::ContentTabView::CloseMultiple()
			 */
			bool CloseAllContentPages();
			
			/**
			 * @brief Lists all pages all content view's in the correct order.
			 *
			 * @return A list of all content pages
			 * @sa nova::ContentView::ListPages()
			 */
			QList<ContentPage*> ListPages() const;
			
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
			 * This method is internally required and should not be called.
			 */
			void RecreateActions(const Properties& creation_parameters = Properties()) override;
			
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
			 * @return The action or nullptr if the action was never constructed
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
			 * @brief Returns the current content page or nullptr if there's none.
			 *
			 * @sa nova::ContentPage
			 * @sa OpenContentPage()
			 */
			inline ContentPage* get_current_page() const { return current_page; }
			
			/**
			 * @brief Returns the current content page's tab view or nullptr if there's none.
			 *
			 * A content tab view can display multiple content pages. When splitting, there are more
			 * than one content tab views.
			 *
			 * @sa nova::ContentTabView
			 */
			inline ContentTabView* get_current_view() const { return current_view; }
			
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
			 * @sa ConstructMenu(StandardMenu, bool)
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
			 * @sa ConstructMenu(QString, bool)
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
			virtual void showEvent(QShowEvent* event) override;
			
			/**
			 * @brief Please do always call this implementation when overriding.
			 *
			 * This method is internally required.
			 */
			virtual void closeEvent(QCloseEvent* event) override;
			
			/**
			 * @brief Please do always call this implementation when overriding.
			 *
			 * This method is internally required.
			 */
			virtual void keyPressEvent(QKeyEvent* event) override;
			
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
			
			/**
			 * @brief Welcome actions are displayed if there's no active content page. They should
			 * give an overview of the application's main functionality.
			 *
			 * Additionally, the application's name (QApplication::applicationDisplayName()) is displayed
			 * at the same place.
			 *
			 * @param actions A list of welcome actions to be displayed
			 */
			void set_welcome_actions(const QList<QAction*>& actions);
		
		private:
			friend class SearchBar;
			friend class SettingsDialog;
			friend class ContentTabView;
			friend class ContentSplitView;
			
			Ui::Workbench* const ui;
			
			MenuActionProvider* standard_menus[5] = {};  // Array length must be up-to-date
			QAction* standard_actions[14] = {};  // Array length must be up-to-date
			MenuActionProvider* menu_tray;
			
			ActionProvider tool_bar_actions;
			ActionProvider tool_window_actions;
			ActionProvider settings_page_actions;
			
			QList<QAction*> welcome_actions;
			
			ContentView* root_view;
			ContentPage* current_page;
			ContentTabView* current_view;
			
			QList<ActionProvider*> providers;
			QList<ToolWindow*> tool_windows;
			QList<SettingsPage*> settings_pages;
			
			QSystemTrayIcon* tray_icon;

#ifdef WIN32
			ITaskbarList4* taskbar;
#endif
			
			void RootSplitMergeHelper(nova::ContentView* new_root);
		
		signals:
			/**
			 * @brief This signal is emitted when the current content page changes.
			 *
			 * The signal could be emitted multiple times for the same page (for example when splitting).
			 *
			 * @param page The new nova::ContentPage which got activated or nullptr
			 * if no page is currently active
			 * @param view The nova::ContentTabView which contains the new page or nullptr
			 * if no page is currently active
			 *
			 * @sa nova::ContentPage::Activate()
			 */
			void currentContentPageChanged(ContentPage* page, ContentTabView* view);
		
		private slots:
			void focusChanged(QWidget*, QWidget* widget);
			void currentContentPageChanged2(ContentPage* page, ContentTabView* view);
			void sysTrayActivated(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::Trigger);
			void notificationLinkActivated(const QString& link);
			void lblEmptyViewLinkActivated(const QString& link);
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
