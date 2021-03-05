/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include <QtCore/QtGlobal>
#include <QtWidgets/QMainWindow>

#include "nova.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QWidget;

namespace Ui { class Workbench; }
QT_END_NAMESPACE

namespace nova {
	class MenuActionProvider;
	
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
			 * - File (title: ```&File```)
			 * - Edit (title: ```&Edit```)
			 * - Help (title: ```&Help```)
			 *
			 * Translate the titles using this context: ```nova/menu```.
			 *
			 * @sa ConstructMenu(StandardMenu)
			 */
			enum StandardMenu {
				File, Edit, Help
			};
			
			/**
			 * Constructs a new workbench. The constructor should only be called once in one application.
			 * Calling this constructor updates automatically the reference of nova::workbench.
			 *
			 * @param parent: The parent window which blocks its input until the workbench window is closed
			 * @sa nova::workbench
			 */
			explicit Workbench(QWidget* parent = nullptr);
			Workbench(const Workbench&) = delete;
			Workbench(Workbench&&) = delete;
			
			virtual ~Workbench() noexcept;
			
			/**
			 * @brief Constructs a new menu and displays it in the menu bar.
			 *
			 * The order of the menu entries is analog to the calls' order of this method.
			 *
			 * @param title The menu's title shown in the menu bar (it might contain the hotkey character ```&```)
			 * @return A pointer to the created menu
			 */
			MenuActionProvider* ConstructMenu(const QString& title);
			/**
			 * @brief Constructs one of the standard menus and displays it in the menu bar.
			 *
			 * This function overloads ConstructMenu()
			 *
			 * @return A pointer to the created menu
			 *
			 * @sa ConstructMenu()
			 * @sa get_standard_menu()
			 */
			MenuActionProvider* ConstructMenu(StandardMenu standard_menu);
			
			/**
			 * Returns the given standard menu which was created using ConstructMenu(StandardMenu).
			 *
			 * @return The menu or ```nullptr``` if the menu is never constructed
			 */
			MenuActionProvider* get_standard_menu(StandardMenu standard_menu);
		
		private:
			Ui::Workbench* ui;
			
			MenuActionProvider* menu_file;
			MenuActionProvider* menu_edit;
			MenuActionProvider* menu_help;
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
