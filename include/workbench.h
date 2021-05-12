/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include <QtCore/QtGlobal>
#include <QtCore/QList>
#include <QtWidgets/QMainWindow>

#include "nova.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QWidget;
class QAction;

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
	 * @sa nova::workbench
	 */
	class NOVA_API Workbench : public QMainWindow {
		public:
			/**
			 * @brief A list of menus needed in nearly every application:
			 *
			 * - File (title: "&File")
			 * - Edit (title: "&Edit")
			 * - Help (title: "&Help")
			 *
			 * The translations belong to the context "nova/menu".
			 *
			 * @sa ConstructMenu(StandardMenu)
			 */
			enum StandardMenu {
				File, Edit, Help
			};
			
			/**
			 * @brief A list of standard actions which are handled by Nova:
			 *
			 * - Exit [Ctrl+Q] (title: "&Exit")
			 * - Direct Help, QWhatsThis [F2] (title: "&Direct Help")
			 * - Search bar for browsing the application's actions
			 *
			 * The translations belong to the context "nova/action".
			 *
			 * @sa ConstructStandardAction()
			 */
			enum StandardAction {
				Exit, DirectHelp, SearchBar
			};
			
			/**
			 * Constructs a new workbench. The constructor should only be called once in one application.
			 * Calling this constructor updates automatically the reference of nova::workbench.
			 *
			 * @param parent: The parent window which blocks its input until the workbench window is closed (optional, default: none)
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
			 * @return A pointer to the created menu
			 */
			MenuActionProvider* ConstructMenu(const QString& title);
			/**
			 * @brief Constructs one of the standard menus and displays it in the menu bar.
			 *
			 * This function overloads ConstructMenu()
			 *
			 * @param standard_menu specifies which menu should be created.
			 * @return A pointer to the created menu or nullptr if standard_menu is invalid
			 *
			 * @sa ConstructMenu()
			 * @sa get_standard_menu()
			 */
			MenuActionProvider* ConstructMenu(StandardMenu standard_menu);
			
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
			 * Returns the given standard menu which was created using ConstructMenu(StandardMenu).
			 *
			 * @return The menu or nullptr if the menu is never constructed
			 */
			MenuActionProvider* get_standard_menu(StandardMenu standard_menu) const;
		
		private:
			friend class ActionProvider;
			friend class SearchBar;
			
			Ui::Workbench* ui;
			
			MenuActionProvider* menu_file;
			MenuActionProvider* menu_edit;
			MenuActionProvider* menu_help;
			
			QList<ActionProvider*> providers;
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
