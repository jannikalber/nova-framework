/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_ACTIONPROVIDER_H
#define NOVA_FRAMEWORK_ACTIONPROVIDER_H

#include <Qt>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QToolBar>

#include "nova.h"

namespace nova {
	class Workbench;
	class ActionProvider;
	class MenuActionProvider;
}

namespace nova {
	/**
	 * @brief A group of several actions being shown separated in nova::ActionProvider objects
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * Action providers can also show (i.e "present") its actions. Therefore, it's important to group the
	 * actions. This class represents such a group. Usually, a separator is shown between every group.
	 *
	 * There's also the possibility to add actions to the group if it already exists. This is useful for plugins.
	 * A group can only belong to one provider. The group is automatically deleted when its provider gets deleted.
	 *
	 * @sa nova::ActionProvider::ShowActionGroup() to associate a provider
	 */
	class NOVA_API ActionGroup {
		public:
			/**
			 * @brief Creates an empty group.
			 *
			 * @param id The unique identification number of the group. This number should be positive and can be used to find
			 * the group and extend it using nova::ActionProvider::FindGroup().
			 * (optional, default: a negative number is used to avoid conflicts with custom ids)
			 */
			explicit ActionGroup(int id = --id_counter);  // Positive ids are reserved for users
			
			/**
			 * @brief Creates a group with an action.
			 *
			 * Its id is randomly chosen.
			 *
			 * @param action The action which should be added to the group
			 * @param is_important_action Important actions may be emphasized (e.g. also being added to an additional tool bar).
			 * This behavior is implementation-specific. Important actions should have an icon. (optional, default: not important)
			 *
			 * @sa AddAction()
			 */
			explicit ActionGroup(QAction* action, bool is_important_action = false);
			NOVA_DISABLE_COPY(ActionGroup)
			
			/**
			 * @brief Adds an action to the existing group.
			 *
			 * This method works as well if the group has already been added to a provider.
			 * The action should belong to the group's provider (So, consider to construct it with
			 * nova::ActionProvider::ConstructAction()).
			 *
			 * @param action The action which should be added to the group
			 * @param is_important_action Important actions may be emphasized (e.g. also being added to an additional tool bar).
			 * This behavior is implementation-specific. Important actions should have an icon. (optional, default: not important)
			 */
			void AddAction(QAction* action, bool is_important_action = false);
			
			/**
			 * @brief Adds a menu to the existing group.
			 *
			 * This method works as well if the group has already been added to a provider.
			 * The menu should be created by a menu provider using nova::MenuActionProvider::ConstructSubMenu()
			 *
			 * Menus cannot be important because tool bars don't support adding menus.
			 *
			 * @param menu The menu which should be added to the group
			 *
			 * @sa AddAction()
			 * @sa nova::MenuActionProvider::ConstructSubMenu()
			 */
			void AddMenu(MenuActionProvider* menu);
			
			/**
			 * @brief Returns the identification number of the group.
			 *
			 * This number can be used to find the group and extend it using ActionProvider::FindGroup().
			 */
			inline int get_id() const { return id; }
			
			/**
			 * @brief Returns the group's provider or nullptr if there's none yet.
			 *
			 * @sa nova::ActionProvider::ShowActionGroup() to associate a provider
			 */
			inline ActionProvider* get_provider() const { return provider; }
		
		private:
			friend class ActionProvider;
			
			static int id_counter;
			
			int id;
			int num_shown;
			bool has_important_action;
			bool important_action_shown;
			QList<QAction*> actions;
			QList<bool> important_list;
			
			ActionProvider* provider;
			int my_index;
			int current_index;
			int current_index_important;
			
			void ShowAllRemaining();
	};
	
	/**
	 * @brief A class which derives nova::ActionProvider can contain actions.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * These actions can be found using nova::SearchBar. An action provider is a separated context
	 * (e.g. a tool window or a menu). Therefore, its actions are categorized in this context.
	 *
	 * Providers can also show (i.e "present") its actions (e.g. in a menu or a tool bar).
	 */
	class NOVA_API ActionProvider {
		public:
			/**
			 * @brief Creates an empty action provider.
			 *
			 * @param title The name of the provider (displayed in nova::SearchBar)
			 *
			 * @sa nova::Workbench::RegisterActionProvider() to register the new provider
			 */
			explicit ActionProvider(const QString& title);
			virtual ~ActionProvider() noexcept;
			
			/**
			 * @brief Constructs an action which is bound to this provider.
			 *
			 * The action is deleted when the provider gets deleted.
			 *
			 * @param text The action's title (can contain the hotkey character "&")
			 * @return A pointer to the constructed action
			 */
			QAction* ConstructAction(const QString& text);
			
			/**
			 * @brief Adds an action which wasn't constructed using ConstructAction() to the provider's list.
			 *
			 * The action's parent will be manipulated.
			 *
			 * @param action The action to be migrated
			 * @sa ConstructAction()
			 */
			inline void MigrateAction(QAction* action) { action->setParent(&object); }
			
			/**
			 * @brief Returns a list containing all actions which belong to this provider.
			 */
			inline QList<QAction*> ListActions() const {
				return object.findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
			}
			
			/**
			 * @brief Returns a list containing all nova::ActionGroup objects which belong to this provider.
			 */
			inline QList<ActionGroup*> ListGroups() const { return groups; }
			
			/**
			 * @brief Returns the nova::ActionGroup with the given identification number or nullptr if it cannot
			 * be found or if it doesn't belong to this group.
			 */
			ActionGroup* FindGroup(int id) const;
			
			/**
			 * @brief Shows an action in the implementation-specific way.
			 *
			 * Warning: If the action is not associated with this action provider (i.e. it wasn't constructed with
             * ConstructAction() or MigrateAction()), it's just shown, but doesn't really belong to the provider. So, it can't be found
			 * using nova::SearchBar.
			 *
			 * @param action The action to be shown (consider to create it with ConstructAction())
			 * @param is_important_action Important actions could be emphasized in the implementation (e.g. also being shown in an
			 * additional tool bar). Important actions should have an icon.
			 *
			 * @return The nova::ActionGroup being created. A single action also belongs to a group.
			 *
			 * @sa ShowActionGroup()
			 */
			inline ActionGroup* ShowAction(QAction* action, bool is_important_action = false) {
				return ShowActionGroup(new ActionGroup(action, is_important_action));
			}
			
			/**
			 * @brief Shows a menu in the implementation-specific way.
			 *
			 * This method works similar to the method ShowAction(). Since the menus being created by the workbench
			 * are automatically shown, this method is usually used to show sub menus (nova::MenuActionProvider::ConstructSubMenu()).
			 *
			 * Menus cannot be important because tool bars don't support adding menus.
			 *
			 * @param menu The menu to be shown
			 *
			 * @return The nova::ActionGroup being created. A single menu also belongs to a group.
			 *
			 * @sa ShowAction()
			 * @sa nova::ActionGroup::AddMenu()
			 * @sa nova::MenuActionProvider::ConstructSubMenu()
			 */
			ActionGroup* ShowMenu(MenuActionProvider* menu);
			
			/**
			 * @brief Shows the group in the implementation-specific way.
			 *
			 * The group will be automatically separated from other groups. You can still add actions to the group
			 * using nova::ActionGroup::AddAction().
			 *
			 * @param group The group to be shown
			 *
			 * @return The group itself because of compatibility reasons
			 *
			 * @sa ShowAction()
			 * @sa nova::ActionGroup::AddAction()
			 */
			ActionGroup* ShowActionGroup(ActionGroup* group);
			
			/**
			 * @brief Changes the title of the provider.
			 *
			 * If your implementation of the action provider needs an update too, simply reimplement this method.
			 * If you don't want to allow title changes, reimplement this method with an empty stub.
			 *
			 * @param title The new title to be set
			 */
			inline virtual void set_title(const QString& title) { this->title = title; }
			
			/**
			 * @brief Returns the provider's title.
			 */
			inline QString get_title() const { return title; }
		
		protected:
			/**
			 * @brief This virtual method is called if a new action should be shown. Your implementation should display it.
			 *
			 * The method's default implementation does nothing.
			 *
			 * If you don't allow important actions, you can ignore the parameters for important actions.
			 *
			 * @param action The action to be displayed
			 * @param index The position at which the action should be displayed. Separators also count. This value is calculated automatically.
			 * @param is_important_action An important action should be emphasized (e.g. also being added to an additional tool bar).
			 * @param important_actions_index The position in the additional container if the action is important. Separators also count.
			 * If the action is not important, the parameter is -1.
			 */
			inline virtual void DisplayAction(QAction* action, int index, bool is_important_action, int important_actions_index) {}
			
			/**
			 * @brief The library automatically calculates the position of necessary separators.
			 * Your implementation should show such a separator.
			 *
			 * The method's default implementation does nothing.
			 *
			 * If you don't allow important actions, you can ignore the parameters for important actions.
			 *
			 * This method is also used to display separators in an additional container for important actions. Usually, the method
			 * is only called once to show the separators for both containers. But there are cases, where only one separator is requested
			 * be shown. So, you have to work with the bool attributes below.
			 *
			 * @param show_regular Indicates if a separator for the regular container is requested
			 * @param index_regular The position at which the separator should be displayed. Other separators also count.
			 * @param show_important_actions Indicates if a separator for the container of important actions is requested
			 * @param index_important_actions The position at which the separator should be displayed in the additional container
			 *
			 * @sa DisplayAction()
			 */
			inline virtual void DisplaySeparators(bool show_regular, int index_regular,
			                                      bool show_important_actions, int index_important_actions) {}
		
		private:
			friend class ActionGroup;
			
			QString title;
			QObject object;  // For the actions to be deleted
			
			QList<ActionGroup*> groups;
			int max_index;
			int max_index_important;
	};
	
	/**
	 * @brief nova::Properties is used by some functions to provide extra information.
	 *
	 * The typedef is translated to QMap<QString, QVariant>.
	 */
	typedef QMap<QString, QVariant> Properties;
	
	/**
	 * @brief A minimal extended nova::ActionProvider which is designed to contain changeable actions.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * There are two extensions:
	 * <ol>
	 *  <li>All actions can be cleared.</li>
	 *  <li>There is a virtual method which allows you to recreate all actions.</li>
	 * </ol>
	 *
	 * For example, this make sense if you want a recent files list to be found using nova::SearchBar.
	 *
	 * @sa nova::ActionProvider
	 */
	class NOVA_API TempActionProvider : public ActionProvider {
		public:
			/**
			 * @brief Creates a new nova::TempActionProvider.
			 *
			 * @param title The provider's title
			 * @sa nova::ActionProvider::ActionProvider()
			 */
			inline explicit TempActionProvider(const QString& title):
					ActionProvider(title) {}
			virtual ~TempActionProvider() noexcept = default;
			
			/**
			 * @brief This method allows you to recreate all of your actions.
			 *
			 * In this method, you could, for example, check if there's a new recent file. Then, you
			 * could create a new action for it.
			 *
			 * Feel free to call your method as often as required.
			 *
			 * The default implementation does nothing.
			 *
			 * @param creation_parameters The map can contain some custom parameter (optional, default: empty map)
			 */
			virtual void RecreateActions(const Properties& creation_parameters = Properties()) {}
		
		protected:
			/**
			 * @brief Clears and deletes all actions associated with this provider.
			 *
			 * This means that all actions are destructed too.
			 */
			void ClearActions();
	};
	
	/**
	 * @brief Implements a QMenu which is an action provider.
	 * @headerfile actionprovider.h <nova/actionprovider.h>
	 *
	 * @sa Workbench::ConstructMenu()
	 */
	class NOVA_API MenuActionProvider : public ActionProvider, private QMenu {
		public:
			/**
			 * @brief Creates a new nova::MenuActionProvider.
			 *
			 * Note: The menu isn't shown in the menu bar or somewhere else. If you want to create a menu for
			 * the menu bar, use nova::Workbench::ConstructMenu().
			 *
			 * @param parent The QObject's parent
			 * @param title The menu's title (can contain the hotkey character "&")
			 * @param needs_tool_bar if a tool bar should also be created (optional, default: false)
			 *
			 * @sa nova::Workbench::RegisterActionProvider() to register the new provider
			 */
			MenuActionProvider(QWidget* parent, const QString& title, bool needs_tool_bar = false);
			NOVA_DISABLE_COPY(MenuActionProvider)
			
			/**
			 * @brief Creates a new sub menu for this menu.
			 *
			 * A new nova::ActionProvider is created. Its name contains the hierarchical path including the parent
			 * menu's names.
			 *
			 * The new provider is automatically registered but not shown.
			 *
			 * @param title The sub menu's title
			 * @param window The workbench which is used for registration (optional, default: nova::workbench)
			 *
			 * @return A pointer to the new menu
			 *
			 * @sa nova::ActionProvider::ShowMenu() to show the sub menu
			 */
			MenuActionProvider* ConstructSubMenu(const QString& title, Workbench* window = workbench);
			
			/**
			 * @brief Returns a pointer to the menu's tool bar or nullptr if it's never created.
			 */
			inline QToolBar* get_tool_bar() const { return tool_bar; }
			
			/**
			 * @brief Menus don't allow changeable titles.
			 *
			 * This method is internally required and should not be called.
			 */
			inline void set_title(const QString&) override {}
		
		protected:
			/**
			 * @brief Important actions are shown in the tool bar.
			 *
			 * This method is internally required and should not be called.
			 */
			void DisplayAction(QAction* action, int index, bool is_important, int important_actions_index) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplaySeparators(bool show_regular, int index_regular,
			                       bool show_important_actions, int index_important_actions) override;
		
		private:
			friend class Workbench;
			friend class ActionGroup;
			
			QToolBar* const tool_bar;
			
			void ConstructNavigationAction(ActionProvider* provider);
	};
}

#endif  // NOVA_FRAMEWORK_ACTIONPROVIDER_H
