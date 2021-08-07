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
#include <QtWidgets/QToolBar>

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
	 *
	 * This class should be derived.
	 */
	class NOVA_API ActionProvider {
		public:
			/**
			 * Initializes the ActionProvider.
			 *
			 * Note that you have to associate the provider to your workbench.
			 *
			 * This class should be derived.
			 *
			 * @param title The title of the category the provider's actions belong to
			 * @sa Workbench::RegisterActionProvider()
			 */
			explicit ActionProvider(const QString& title);
			virtual ~ActionProvider() noexcept = default;
			
			/**
			 * @brief Constructs an action which is bound to this provider.
			 *
			 * The action is deleted when the provider will be deleted.
			 *
			 * @param text The action's title (it might contain the hotkey character "&")
			 * @return A pointer to the constructed action
			 */
			QAction* ConstructAction(const QString& text);
			
			/**
			 * @brief Shows an action in the implementation-specific way.
			 *
			 * Warning: If the action is not associated with this action provider (i.e. it wasn't constructed with
			 * ConstructAction()), it 's just shown, bt doesn't really belong to the provider. So, it can't be found
			 * using the SearchBar.
			 *
			 * @param action The action to be displayed (consider to construct it with ConstructAction())
			 * @param separate If a separator should be inserted in front of the action
			 * @param is_important An important action may be emphasized (this depends also on the type of provider).
			 * Important actions should have an icon.
			 *
			 * @sa MenuActionProvider::ShowAction()
			 */
			virtual void ShowAction(QAction* action, bool separate, bool is_important) = 0;
			
			/**
			 * @brief Returns the provider's title.
			 */
			inline QString get_title() { return title; }
			
			/**
			 * @brief Returns a list containing all actions which belong to this provider.
			 */
			inline QList<QAction*> ListActions() {
				return object.findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
			}
		
		private:
			const QString title;
			QObject object;
	};
	
	/**
	 * @brief A minimal extended standard action provider which is designed to contain changeable actions.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * There are two extensions:
	 * - All actions can be cleared
	 * - There is a pure virtual method in which the provider's actions are recreated
	 *
	 * For example, this make sense if you want a recent files list to be found using the SearchBar.
	 *
	 * @sa ActionProvider
	 */
	class NOVA_API TempActionProvider : public ActionProvider {
		public:
			/**
			 * Initializes the TempActionProvider.
			 *
			 * @param title The provider's title
			 * @sa ActionProvider::ActionProvider()
			 */
			inline explicit TempActionProvider(const QString& title) : ActionProvider(title) {}
			virtual ~TempActionProvider() noexcept = default;
			
			/**
			 * @brief Recreates the provider's action considering the actual state of your application.
			 *
			 * This method is pure virtual and must be implemented
			 *
			 * @param creation_parameter This can be any parameter needed to recreate (optional, default: nullptr)
			 */
			virtual void RecreateActions(void* creation_parameter = nullptr) = 0;
		
		protected:
			/**
			 * @brief Clears and deletes all actions associated with this provider.
			 *
			 * Note: All actions are also destructed.
			 */
			void ClearActions();
			
			/**
			 * Reimplements ActionProvider::ShowAction()
			 *
			 * ShowAction() doesn't make sense for TempActionProviders.
			 */
			inline void ShowAction(QAction* action, bool separate, bool is_important) override {}
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
			 * Note: The menu isn't shown in the menu bar, for that use Workbench::ConstructMenu().
			 * Don't forget to register the action provider.
			 *
			 * @param parent The QObject's parent
			 * @param title The menu's title (it might contain the hotkey character "&")
			 * @param needs_tool_bar if a tool bar should also be created (optional, default: false)
			 * @sa Workbench::RegisterActionProvider()
			 */
			MenuActionProvider(QWidget* parent, const QString& title, bool needs_tool_bar = false);
			
			/**
			 * Reimplements ActionProvider::ShowAction()
			 *
			 * Important actions are shown in the tool bar.
			 */
			void ShowAction(QAction* action, bool separate = false, bool is_important = false) override;
			
			/**
			 * @brief Returns a pointer to the menu's tool bar or nullptr if it's never created.
			 */
			inline QToolBar* get_tool_bar() const { return tool_bar; }
		
		private:
			friend class Workbench;
			
			QToolBar* tool_bar;
	};
}

#endif  // NOVA_FRAMEWORK_ACTIONPROVIDER_H
