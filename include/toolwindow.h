/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_TOOLWINDOW_H
#define NOVA_FRAMEWORK_TOOLWINDOW_H

#include <Qt>
#include <QObject>
#include <QDockWidget>

#include "nova.h"
#include "actionprovider.h"

class QWidget;
class QAction;
class QMenu;
class QMainWindow;
class QToolBar;
class QPoint;

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
	 *
	 * The translations belong to the context "nova/workbench".
	 */
	class NOVA_API ToolWindow : public QDockWidget, public ActionProvider {
		Q_OBJECT
		
		public:
			NOVA_DISABLE_COPY(ToolWindow)
			inline virtual ~ToolWindow() noexcept = default;
			
			/**
			 * @brief Returns the tool window's content widget.
			 */
			QWidget* get_content_widget() const;
			
			/**
			 * @brief Returns a pointer to the tool window's tool bar or nullptr if it's never created.
			 */
			inline QToolBar* get_tool_bar() const { return tool_bar; }
			
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
			 * @param title The tool window's title
			 * @param orientation if the tool window is a vertical or a horizontal one
			 * @param window The associated workbench (subclass constructors should forward this pointer)
			 * @param needs_tool_bar if a tool bar should also be created and shown (optional, default: false)
			 * @param default_layout The tool window's initial position or Qt::NoDockWidgetArea if it should be hidden
			 * by default. (optional, default: hidden)
			 *
			 * @sa set_content_widget()
			 * @sa nova::Workbench::RegisterToolWindow()
			 */
			ToolWindow(const QString& title, Qt::Orientation orientation, Workbench* window,
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
			void DisplayAction(QAction* action, int index,
			                    bool is_important_action, int important_actions_index) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplaySeparators(bool show_regular, int index_regular,
			                       bool show_important_actions, int index_important_actions) override;
		
		private:
			friend class Workbench;
			
			QMainWindow* const nested_main_window;
			QToolBar* const tool_bar;
			QMenu* const menu;
			
			Qt::DockWidgetArea default_layout;
			const bool default_hidden;
			const Qt::Orientation orientation;
			// For Workbench::RestoreLayout(); it holds the width (vertical ones) or height (horizontal ones) of the tool window
			int initial_size;
			
			// Checkable action to enable or disable the tool window
			void ConstructNavigationAction(ActionProvider* provider);
			
		private slots:
			void customContextMenuRequested2(const QPoint& pos);
	};
}

#endif  // NOVA_FRAMEWORK_TOOLWINDOW_H
