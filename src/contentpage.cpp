/*
 * Copyright (c) 2022 by Jannik Alber.
 * All rights reserved.
 */

#include "contentpage.h"

#include <Qt>
#include <QObject>
#include <QPoint>
#include <QSize>
#include <QList>
#include <QStringList>
#include <QStyle>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolBar>
#include <QTabBar>
#include <QApplication>
#include <QScreen>

#include "workbench.h"
#include "quickdialog.h"

#define NOVA_CONTEXT "nova/workbench"

namespace nova {
	ContentPage::ContentPage(const QString& title, const QIcon& icon, Workbench* window, bool needs_tool_bar):
			QWidget(window), ActionProvider(title), current_view(nullptr), nested_main_window(new QMainWindow()),
			icon(icon), tool_bar(needs_tool_bar ? new QToolBar(nested_main_window) : nullptr) {
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		
		nested_main_window->setParent(this);  // Setting the parent using the constructor causes a separate window
		nested_main_window->setContextMenuPolicy(Qt::PreventContextMenu);
		layout->addWidget(nested_main_window);
		
		if (tool_bar != nullptr) {
			// Tool bar
			tool_bar->setMovable(false);
			tool_bar->setIconSize(QSize(16, 16));
			
			nested_main_window->addToolBar(Qt::TopToolBarArea, tool_bar);
		}
	}
	
	void ContentPage::Activate() {
		// All extra functionality must be added to ContentTabView::Activate() for compatibility reasons
		if (current_view != nullptr) current_view->Activate(this);
	}
	
	bool ContentPage::IsActive() const {
		if (current_view != nullptr) return (current_view->workbench_window->get_current_page() == this);
		else return false;
	}
	
	bool ContentPage::Close() {
		if (current_view != nullptr) return current_view->Close(this);
		else return false;
	}
	
	QWidget* ContentPage::get_content_widget() const {
		return nested_main_window->centralWidget();
	}
	
	void ContentPage::set_title(const QString& title) {
		ActionProvider::set_title(title);
		UpdateTabText();
	}
	
	void ContentPage::set_prefix(const QString& prefix) {
		this->prefix = prefix;
		UpdateTabText();
	}
	
	void ContentPage::set_suffix(const QString& suffix) {
		this->suffix = suffix;
		UpdateTabText();
	}
	
	void ContentPage::set_content_widget(QWidget* content_widget) {
		nested_main_window->setCentralWidget(content_widget);
	}
	
	void ContentPage::DisplayAction(QAction* action, int index,
	                                bool is_important_action, int important_actions_index) {
		addAction(action);
		
		menu_actions.insert(index, action);
		
		if ((tool_bar != nullptr) && is_important_action) {
			if (important_actions_index >= tool_bar->actions().count()) tool_bar->addAction(action);
			else tool_bar->insertAction(tool_bar->actions()[important_actions_index], action);
		}
	}
	
	void ContentPage::DisplaySeparators(bool show_regular, int index_regular,
	                                    bool show_important_actions, int index_important_actions) {
		if (show_regular) {
			auto* separator = new QAction(this);
			separator->setSeparator(true);
			menu_actions.insert(index_regular, separator);
		}
		
		if ((tool_bar != nullptr) && show_important_actions) {
			if (index_important_actions >= tool_bar->actions().count()) tool_bar->addSeparator();
			else tool_bar->insertSeparator(tool_bar->actions()[index_important_actions]);
		}
	}
	
	void ContentPage::UpdateTabText() {
		if (current_view != nullptr) {
			current_view->setTabText(current_view->ListPages().indexOf(this), (prefix + get_title() + suffix));
			current_view->workbench_window->RecreateActions();
		}
	}
	
	ContentTabView::ContentTabView(ContentSplitView* parent_view, Workbench* window):
			ContentView(parent_view, window), QTabWidget(window) {
		setObjectName("nova-content-tab-view");  // Used in the workbench's style sheet
		
		setAcceptDrops(true);
		setMovable(true);
		setTabsClosable(true);
		setDocumentMode(true);
		tabBar()->setAcceptDrops(true);
		tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
		tabBar()->setChangeCurrentOnDrag(true);
		tabBar()->setElideMode(Qt::ElideRight);
		
		auto* tool_bar = new QToolBar(this);
		tool_bar->setIconSize(QSize(10, 10));
		setCornerWidget(tool_bar);
		
		auto* tab_list_action = new QAction(NOVA_TR("&Tabs"), this);
		tab_list_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
		tool_bar->addAction(tab_list_action);
		connect(tab_list_action, &QAction::triggered, this, &ContentTabView::actTabListTriggered);
		
		auto* detach_action = new QAction(NOVA_TR("&Detach Group"), this);
		detach_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
		tool_bar->addAction(detach_action);
		
		auto* close_action = new QAction(NOVA_TR("&Close Group"), this);
		close_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
		tool_bar->addAction(close_action);
		connect(close_action, &QAction::triggered, [this]() { Close(); });
		
		connect(tabBar(), &QTabBar::tabBarClicked, this, &ContentTabView::tabBarClicked);
		connect(tabBar(), &QTabBar::customContextMenuRequested, this,
		        &ContentTabView::tabBarCustomContextMenuRequested);
		connect(this, &ContentTabView::currentChanged, [this](int index) {
			/*
			 * Also executed when closing and adding new tabs, in this case the list is not synchronized with the tabs,
			 * so the activation has to be emitted manually later on.
			 */
			if (content_pages.count() == count()) {
				Activate(index);
			}
		});
		connect(this, &ContentTabView::tabCloseRequested, [this](int index) { Close(index); });
		connect(tabBar(), &QTabBar::tabMoved, [this](int from, int to) {
			content_pages.move(from, to);
			Activate(to);
		});
	}
	
	void ContentTabView::Activate(int index) {
		if ((index >= 0) && (index < count())) {
			// setCurrentIndex() will call this method and emit the signal later on (the first call is like a preparation)
			if (index == currentIndex()) {
				content_pages[index]->get_content_widget()->setFocus();
				emit workbench_window->currentContentPageChanged(content_pages[index], this);
			} else setCurrentIndex(index);
		}
	}
	
	bool ContentTabView::Close(int index) {
		if ((index < 0) || (index >= count())) return false;
		
		Activate(index);
		if (!content_pages[index]->CanClose()) return false;
		
		removeTab(index);
		delete content_pages[index];
		content_pages.removeAt(index);
		
		if (count() == 0) {
			// Trigger merge
			if (parent_view != nullptr) {
				parent_view->Merge(this);
			} else {
				workbench_window->RootSplitMergeHelper(nullptr);
			}
			
			deleteLater();
		} else {
			Activate(currentIndex());
		}
		
		return true;
	}
	
	bool ContentTabView::CloseMultiple(bool left, bool current, bool right) {
		const int left_count = currentIndex();
		const int right_count = (count() - left_count - 1);
		int start_index_right = 0;
		
		if (left) {
			for (int i = 0; i < left_count; ++i) {
				if (!Close(0)) return false;
			}
		} else start_index_right += left_count;
		
		if (current) {
			if (!Close(0)) return false;
		} else ++start_index_right;
		
		if (right) {
			for (int i = start_index_right; i < (start_index_right + right_count); ++i) {
				if (!Close(start_index_right)) return false;
			}
		}
		
		return true;
	}
	
	void ContentTabView::Split(int index, Qt::Orientation orientation) {
		if (count() <= 1) return;
		
		// Sizes must be queried before changing the parent
		const QList<int>& sizes = ((parent_view == nullptr) ? QList<int>() : parent_view->sizes());
		
		auto* new_tab_view = new ContentTabView(nullptr, workbench_window);
		auto* split_view = new ContentSplitView(this, new_tab_view, orientation, parent_view, workbench_window);
		new_tab_view->parent_view = split_view;
		
		if (parent_view == nullptr) {
			workbench_window->RootSplitMergeHelper(split_view);
		} else {
			parent_view->Replace(this, split_view, sizes);
		}
		
		parent_view = split_view;
		
		// Qt changes the view's visibility, so it has to be shown again
		setVisible(true);
		workbench_window->MoveContentPage(content_pages[index], new_tab_view);
	}
	
	void ContentTabView::Open(ContentPage* page) {
		const int new_index = addTab(page, page->get_icon(), (page->get_title() + page->get_suffix()));
		tabBar()->setTabToolTip(new_index, page->get_title());
		content_pages.insert(new_index, page);
		page->current_view = this;
		
		Activate(new_index);
	}
	
	void ContentTabView::tabBarClicked() {
		if (workbench_window->get_current_view() != this) {
			Activate(currentIndex());
		}
	}
	
	void ContentTabView::tabBarCustomContextMenuRequested(const QPoint& pos) {
		const int index = tabBar()->tabAt(pos);
		if (index != -1) {
			QMenu menu(tabBar());
			
			if (workbench_window->get_standard_action(Workbench::Action_Close) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_Close));
			}
			if (workbench_window->get_standard_action(Workbench::Action_CloseOthers) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseOthers));
			}
			if (workbench_window->get_standard_action(Workbench::Action_CloseTabsLeft) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseTabsLeft));
			}
			if (workbench_window->get_standard_action(Workbench::Action_CloseTabsRight) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseTabsRight));
			}
			
			if (!menu.isEmpty()) menu.addSeparator();
			
			if (workbench_window->get_standard_action(Workbench::Action_SplitRight) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_SplitRight));
			}
			if (workbench_window->get_standard_action(Workbench::Action_SplitDown) != nullptr) {
				menu.addAction(workbench_window->get_standard_action(Workbench::Action_SplitDown));
			}
			
			if (!menu.isEmpty()) menu.addSeparator();
			menu.addActions(content_pages[index]->menu_actions);
			
			if (menu.isEmpty()) return;
			
			setCurrentIndex(index);
			menu.exec(tabBar()->mapToGlobal(pos));
		}
	}
	
	void ContentTabView::actTabListTriggered() {
		tabBarClicked();
		
		QStringList names;
		QList<QIcon> icons;
		
		for (const ContentPage* i: ListPages()) {
			names << i->get_title();
			icons << i->get_icon();
		}
		
		const int index = QuickDialog::InputItemIndex(workbench_window, NOVA_TR("Tabs"), names, icons, currentIndex(),
		                                              true);
		if (index != -1) setCurrentIndex(index);
	}
	
	ContentSplitView::ContentSplitView(ContentView* view_1, ContentView* view_2, Qt::Orientation orientation,
	                                   ContentSplitView* parent_view, Workbench* window) :
			QSplitter(orientation, window), ContentView(parent_view, window), view_1(view_1), view_2(view_2) {
		setChildrenCollapsible(false);
		
		addWidget(*view_1);
		addWidget(*view_2);
		
		const int large_number = ((orientation == Qt::Vertical) ? QApplication::primaryScreen()->virtualSize().height()
		                                                        : QApplication::primaryScreen()->virtualSize().width());
		setSizes(QList<int>({large_number, large_number}));
	}
	
	QList<ContentPage*> ContentSplitView::ListPages() const {
		QList<ContentPage*> list;
		list.append(view_1->ListPages());
		list.append(view_2->ListPages());
		return list;
	}
	
	void ContentSplitView::Merge(ContentTabView* victim_view) {
		ContentView* other_view = ((view_1 == victim_view) ? view_2 : view_1);
		
		if (parent_view != nullptr) {
			parent_view->Replace(this, other_view, parent_view->sizes());
		} else {
			workbench_window->RootSplitMergeHelper(other_view);
		}
		
		other_view->parent_view = parent_view;
		
		// Activate another view to avoid access violations
		ContentView* current = other_view;
		while (dynamic_cast<ContentSplitView*>(current) != nullptr) {
			current = dynamic_cast<ContentSplitView*>(current)->view_1;
		}
		dynamic_cast<ContentTabView*>(current)->get_current_page()->Activate();
		
		deleteLater();
	}
	
	void ContentSplitView::Replace(ContentView* old_view, ContentView* new_view, const QList<int>& restore_sizes) {
		QWidget* old_widget = *old_view;
		const int index = ((view_1 == old_view) ? 0 : 1);
		
		// Remove the old widget (change its parent to nullptr)
		if (old_widget->parent() == this) old_widget->setParent(nullptr);
		insertWidget(index, *new_view);
		setSizes(restore_sizes);  // Restore the size because inserting a new widget changes it
		
		if (index == 0) view_1 = new_view;
		else view_2 = new_view;
	}
}
