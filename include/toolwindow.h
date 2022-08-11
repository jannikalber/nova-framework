/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_TOOLWINDOW_H
#define NOVA_FRAMEWORK_TOOLWINDOW_H

#include <Qt>
#include <QDockWidget>

#include "nova.h"
#include "actionprovider.h"

class QWidget;
class QAction;
class QMainWindow;
class QToolBar;

namespace nova { class Workbench; }

namespace nova {
	/**
	 * @brief A nova::ToolWindow is a small window which sits in the workbench's docking area (QDockWidget).
	 * @headerfile toolwindow.h <nova/toolwindow.h>
	 *
	 * Tool windows can have a tool bar with actions. Therefore, a tool window is a nova::ActionProvider.
	 *
	 * There are two types of tool windows: the vertical (left and right areas) and the horizontal (top and bottom areas)
	 * ones.
	 *
	 * Your subclass must have a constructor with QWidget* as parameter (the parent window).
	 * Call nova::Workbench::RegisterToolWindow<YourSubclass>() to register the tool window class.
	 *
	 * This class must be derived.
	 */
	class NOVA_API ToolWindow : public QDockWidget, public ActionProvider {
		public:
			NOVA_DISABLE_COPY(ToolWindow)
			virtual ~ToolWindow() noexcept = default;
			
			/**
			 * @brief Returns the tool window's content widget.
			 */
			QWidget* get_content_widget() const;
			
			/**
			 * @brief Returns the tool window's orientation.
			 */
			inline Qt::Orientation get_orientation() const { return orientation; }
			
			/**
			 * @brief Tool windows don't allow changeable titles.
			 *
			 * This method is internally required and should not be called.
			 */
			inline void set_title(const QString&) override {}
		
		protected:
			/**
			 * @brief Creates a new nova::ToolWindow.
			 *
			 * Use set_content_widget() to change the window's content.
			 *
			 * @param parent The parent window
			 * @param title The tool window's title
			 * @param orientation if the tool window is a vertical or a horizontal one
			 * @param needs_tool_bar if a tool bar should also be created and shown (optional, default: false)
			 * @param default_layout The tool window's initial position or Qt::NoDockWidgetArea if it should be hidden
			 * by default. (optional, default: hidden)
			 *
			 * @sa set_content_widget()
			 * @sa nova::Workbench::RegisterToolWindow()
			 */
			ToolWindow(QWidget* parent, const QString& title, Qt::Orientation orientation,
			           bool needs_tool_bar = false, Qt::DockWidgetArea default_layout = Qt::NoDockWidgetArea);
			
			/**
			 * @brief Sets the tool window's content widget.
			 *
			 * Call this method in the constructor of your window to add functionality.
			 * The method takes ownership of the pointer and deletes it at appropriate time.
			 *
			 * @param content_widget The widget to be set
			 */
			void set_content_widget(QWidget* content_widget);
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplayAction(QAction* action, int index, bool, int) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplaySeparators(bool show_regular, int index_regular, bool, int) override;
		
		private:
			friend class Workbench;
			
			QMainWindow* const nested_main_window;
			QToolBar* const tool_bar;
			
			Qt::DockWidgetArea default_layout;
			const bool default_hidden;
			const Qt::Orientation orientation;
			// For Workbench::RestoreLayout(); it holds the width (vertical ones) or height (horizontal ones) of the tool window
			int initial_size;
			
			void ConstructNavigationAction(ActionProvider* provider);
	};
}

#endif  // NOVA_FRAMEWORK_TOOLWINDOW_H
