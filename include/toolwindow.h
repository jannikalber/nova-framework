/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_TOOLWINDOW_H
#define NOVA_FRAMEWORK_TOOLWINDOW_H

#include <QtCore/QtGlobal>
#include <QtCore/Qt>
#include <QtWidgets/QDockWidget>

#include "nova.h"
#include "actionprovider.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QWidget;
class QMainWindow;
class QToolBar;
QT_END_NAMESPACE

namespace nova {
	/**
	 * @brief A tool window is a small window which sits in the workbench's border (QDockWidget).
	 * @headerfile toolwindow.h <nova/toolwindow.h>
	 *
	 * Tool windows take a special role in this library: Often, plugins extend an application by providing their
	 * tool windows.
	 *
	 * Tool windows can have a tool bar (that isn't possible in Qt) and therefore can contain actions. So, a tool window
	 * is an ActionProvider.
	 *
	 * There are two types of tool windows: the vertical (left and right areas) and the horizontal (top and bottom areas)
	 * ones.
	 *
	 * This class should be derived.
	 */
	class NOVA_API ToolWindow : public QDockWidget, public ActionProvider {
		public:
			/**
			 * @brief Adds the tool window to the workbench (see parameter window below).
			 *
			 * Use set_content_widget() for changing the window's content.
			 *
			 * @param title The tool window's title
			 * @param orientation if the tool window is a vertical or a horizontal one.
			 * @param needs_tool_bar if a tool bar should also be created and shown (optional, default: false).
			 * @param default_layout The tool window's initial position or Qt::NoDockWidgetArea if it should be hidden
			 * by default. (optional, hidden)
			 * @param window The tool window's parent workbench and widget (optional, default: nova::workbench)
			 *
			 * @sa set_content_widget()
			 */
			explicit ToolWindow(const QString& title, Qt::Orientation orientation, bool needs_tool_bar = false,
			                    Qt::DockWidgetArea default_layout = Qt::NoDockWidgetArea,
			                    Workbench* window = workbench);
			
			/**
			 * Destructs the tool window.
			 */
			virtual ~ToolWindow() noexcept;
			
			/**
			 * @brief Resets the tool window to its default position or hides it if it's hidden by default.
			 */
			void ResetLayout();
			
			/**
			 * @brief Returns the tool window's content widget.
			 */
			QWidget* get_content_widget() const;
		
		protected:
			/**
			 * @brief Manipulates the tool window's content widget.
			 *
			 * Call this method in the constructor of your window to add functionality.
			 */
			void set_content_widget(QWidget* widget);
			
			/**
			 * @brief Adds actions to the tool bar. (Reimplements ActionProvider::ShowAction())
			 *
			 * This only works for important actions and if a tool bar exists.
			 */
			void ShowAction(QAction* action, bool separate = false, bool is_important = true) override;
		
		private:
			QMainWindow* nested_main_window;
			QToolBar* tool_bar;
			
			Qt::DockWidgetArea default_layout;
			const bool default_hidden;
	};
}

#endif  // NOVA_FRAMEWORK_TOOLWINDOW_H
