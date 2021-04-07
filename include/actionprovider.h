/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_ACTIONPROVIDER_H
#define NOVA_FRAMEWORK_ACTIONPROVIDER_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>

#include "nova.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace nova {
	class Workbench;
	
	/**
	 * @brief A class which extends ActionProvider can contain actions.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * These actions can be found using the search bar. An ActionProvider is a separated context
	 * (e.g. a tool window or a menu). Therefore, its actions are categorized in that context.
	 */
	class NOVA_API ActionProvider : private QObject {
		public:
			/**
			 * Initializes the ActionProvider.
			 *
			 * @param title The title of the category the provider's actions belong to
			 * @param window The provider's Workbench (optional, default: nova::workbench)
			 */
			inline explicit ActionProvider(const QString& title, Workbench* window = workbench) : title(title),
			                                                                                      window(window) {}
			
			/**
			 * @brief Constructs an action which is bound to this provider.
			 *
			 * @param text The action's title (it might contain the hotkey character "&")
			 * @return A pointer to the constructed action
			 */
			inline QAction* ConstructAction(const QString& text) {
				auto* action = new QAction(this);
				action->setText(text);
				return action;
			}
			
			/**
			 * @brief Shows an action in the implementation specific way.
			 *
			 * @param action The action to be displayed (consider to construct it with ConstructAction())
			 * @param separate If a separator should be inserted before inserting the action (optional, default: false)
			 * @param is_important An important action may be emphasized (this depends also on the type of provider)
			 * (optional, default: false)
			 *
			 * @sa MenuActionProvider::ShowAction()
			 */
			virtual void ShowAction(QAction* action, bool separate = false, bool is_important = false) = 0;
		
		private:
			const QString title;
			Workbench* window;
	};
	
	/**
	 * @brief Implements an ActionProvider
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * Its actions are bound to a menu.
	 *
	 * @sa Workbench::ConstructMenu()
	 */
	class NOVA_API MenuActionProvider : public ActionProvider, private QMenu {
		public:
			/**
			 * Constructs a new MenuActionProvider
			 *
			 * @param title The menu's title (it might contain the hotkey character "&")
			 * @param parent The menu's parent widget
			 */
			MenuActionProvider(const QString& title, QWidget* parent);
			
			void ShowAction(QAction* action, bool separate = false, bool is_important = false) override;
		
		private:
			friend class Workbench;
	};
}

#endif  // NOVA_FRAMEWORK_ACTIONPROVIDER_H
