/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "actionprovider.h"

#include <QList>
#include <QSize>
#include <QWidget>

#include "workbench.h"

namespace nova {
	int ActionGroup::id_counter = 0;
	
	ActionGroup::ActionGroup(int id):
			id(id), num_shown(0), has_important_action(false), important_action_shown(false),
			my_index(-1), current_index(-1), current_index_important(-1), provider(nullptr) {}
	
	ActionGroup::ActionGroup(QAction* action, bool is_important_action):
			ActionGroup() {
		actions << action;
		important_list << is_important_action;
		if (is_important_action) has_important_action = true;
	}
	
	void ActionGroup::AddAction(QAction* action, bool is_important_action) {
		actions << action;
		important_list << is_important_action;
		if (is_important_action && !has_important_action) has_important_action = true;
		
		if (provider != nullptr) ShowAllRemaining();
	}
	
	void ActionGroup::AddMenu(MenuActionProvider* menu) {
		AddAction(menu->menuAction());
	}
	
	void ActionGroup::ShowAllRemaining() {
		if (actions.isEmpty()) return;
		
		int counter = 0;
		int counter_important = 0;
		
		// Add separators
		int separator = -1;  // Avoid calling the virtual method too often
		int separator_important = -1;
		if ((num_shown == 0) && (current_index != 0)) {
			separator = current_index;
			++current_index;
			++provider->max_index;
		}
		
		if (has_important_action && !important_action_shown && (current_index_important != 0)) {
			separator_important = current_index_important;
			++current_index_important;
			++provider->max_index_important;
		}
		
		if ((separator != -1) || (separator_important != -1)) {
			provider->DisplaySeparators((separator != -1), separator,
			                            (separator_important != -1), separator_important);
		}
		
		for (int i = num_shown ; i < actions.count() ; ++i) {
			QAction* action = actions[i];
			const bool is_important_action = important_list[i];
			
			provider->DisplayAction(action, current_index, is_important_action,
			                        (is_important_action ? current_index_important : -1));
			++current_index;
			++counter;
			
			if (is_important_action) {
				if (!important_action_shown) important_action_shown = true;
				
				++current_index_important;
				++counter_important;
			}
		}
		
		num_shown += counter;
		provider->max_index += counter;
		provider->max_index_important += counter_important;
		
		// Update other indexes
		for (int i = my_index + 1 ; i < provider->groups.count() ; ++i) {
			provider->groups[i]->current_index += counter;
			provider->groups[i]->current_index_important += counter_important;
		}
	}
	
	ActionProvider::ActionProvider(const QString& title):
			title(title), max_index(0), max_index_important(0) {}
	
	ActionProvider::~ActionProvider() noexcept {
		// Delete all assigned groups
		for (ActionGroup* i : groups) {
			delete i;
		}
	}
	
	QAction* ActionProvider::ConstructAction(const QString& text) {
		auto* action = new QAction(&object);
		action->setText(text);
		return action;
	}
	
	MenuActionProvider* ActionProvider::ConstructMenu(QWidget* parent, const QString& title) {
		auto* provider = new MenuActionProvider(parent, (get_title() + " > " + title));
		provider->setTitle(title);
		return provider;
	}
	
	ActionGroup* ActionProvider::FindGroup(int id) const {
		for (ActionGroup* i : groups) {
			if (i->get_id() == id) return i;
		}
		
		return nullptr;
	}
	
	ActionGroup* ActionProvider::ShowMenu(MenuActionProvider* menu) {
		auto* group = new ActionGroup();
		group->AddMenu(menu);
		return ShowActionGroup(group);
	}
	
	ActionGroup* ActionProvider::ShowActionGroup(ActionGroup* group) {
		if (group->provider != nullptr) return nullptr;
		
		group->provider = this;
		group->my_index = groups.count();
		group->current_index = max_index;
		group->current_index_important = max_index_important;
		groups << group;
		
		group->ShowAllRemaining();
		return group;
	}
	
	void TempActionProvider::ClearActions() {
		for (const QAction* i : ListActions()) {
			delete i;
		}
	}
	
	MenuActionProvider::MenuActionProvider(QWidget* parent, const QString& title, bool needs_tool_bar):
			ActionProvider(QString(title).replace('&', "")), QMenu(parent),
			tool_bar(needs_tool_bar ? new QToolBar(get_title(), parent) : nullptr) {
		setTitle(title);
		if (tool_bar != nullptr) {
			tool_bar->setIconSize(QSize(16, 16));
		}
	}
	
	void MenuActionProvider::DisplayAction(QAction* action, int index, bool is_important, int important_actions_index) {
		if (index >= actions().count()) addAction(action);
		else insertAction(actions()[index], action);
		
		if (is_important && (tool_bar != nullptr)) {
			if (important_actions_index >= tool_bar->actions().count()) tool_bar->addAction(action);
			else tool_bar->insertAction(tool_bar->actions()[important_actions_index], action);
		}
	}
	
	void MenuActionProvider::DisplaySeparators(bool show_regular, int index_regular,
	                                           bool show_important_actions, int index_important_actions) {
		if (show_regular) {
			if (index_regular >= actions().count()) addSeparator();
			else insertSeparator(actions()[index_regular]);
		}
		
		if ((tool_bar != nullptr) && show_important_actions) {
			if (index_important_actions >= tool_bar->actions().count()) tool_bar->addSeparator();
			else tool_bar->insertSeparator(tool_bar->actions()[index_important_actions]);
		}
	}
	
	void MenuActionProvider::ConstructNavigationAction(ActionProvider* provider) {
		if (tool_bar != nullptr) {
			QAction* action = provider->ConstructAction(get_title());
			action->setCheckable(true);
			
			connect(tool_bar, &QToolBar::visibilityChanged, action, [action](bool is_visible) {
				// Don't emit toggled() to avoid undefined behavior
				const bool old_state = action->blockSignals(true);
				action->setChecked(is_visible);
				action->blockSignals(old_state);
			});
			connect(action, &QAction::toggled, tool_bar, &QToolBar::setVisible);
		}
	}
}
