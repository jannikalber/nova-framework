/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_ACTIONPROVIDER_H
#define NOVA_FRAMEWORK_ACTIONPROVIDER_H

#include <QtCore/Qt>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtWidgets/QMenu>

#include "nova.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

namespace nova {
	class Workbench;
	class SearchBar;
	
	/**
	 * @brief A class which extends ActionProvider can contain actions.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * These actions can be found using the SearchBar. An ActionProvider is a separated context
	 * (e.g. a tool window or a menu). Therefore, its actions are categorized in that context.
	 */
	class NOVA_API ActionProvider : public QObject {
		public:
			/**
			 * Initializes the ActionProvider.
			 *
			 * @param title The title of the category the provider's actions belong to
			 * @param window The provider's Workbench (optional, default: nova::workbench)
			 */
			explicit ActionProvider(const QString& title, Workbench* window = workbench);
			
			/**
			 * @brief Constructs an action which is bound to this provider.
			 *
			 * @param text The action's title (it might contain the hotkey character "&")
			 * @return A pointer to the constructed action
			 */
			QAction* ConstructAction(const QString& text);
			
			/**
			 * @brief Shows an action in the implementation specific way.
			 *
			 * Warning: If the action is not associated with this action provider (i.e. it wasn't constructed with
			 * ConstructAction()), it won't/shouldn't be shown.
			 *
			 * @param action The action to be displayed (consider to construct it with ConstructAction())
			 * @param separate If a separator should be inserted before inserting the action (optional, default: false)
			 * @param is_important An important action may be emphasized (this depends also on the type of provider)
			 * (optional, default: false)
			 *
			 * @sa MenuActionProvider::ShowAction()
			 */
			virtual void ShowAction(QAction* action, bool separate = false, bool is_important = false) = 0;
			
			/**
			 * @brief Returns the provider's title.
			 */
			inline QString get_title() { return title; }
			
			/**
			 * @brief Returns a list containing all actions which belong to this provider.
			 */
			inline QList<QAction*> ListActions() {
				return findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
			}
		
		private:
			const QString title;
			Workbench* window;
	};
	
	/**
	 * @brief Implements an ActionProvider
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * Its actions are bound to a global menu. This is in nearly every case a menu of the menu bar.
	 *
	 * @sa Workbench::ConstructMenu()
	 */
	class NOVA_API MenuActionProvider : public ActionProvider, private QMenu {
		public:
			/**
			 * Constructs a new MenuActionProvider
			 *
			 * Hint: the menu isn't shown in the menu bar, for that use Workbench::ConstructMenu().
			 *
			 * @param title The menu's title (it might contain the hotkey character "&")
			 * @param window The menu's parent workbench and widget
			 */
			MenuActionProvider(const QString& title, Workbench* window);
			
			void ShowAction(QAction* action, bool separate = false, bool is_important = false) override;
		
		private:
			friend class Workbench;
	};
}

#endif  // NOVA_FRAMEWORK_ACTIONPROVIDER_H
