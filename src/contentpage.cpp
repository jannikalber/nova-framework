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

#include "workbench.h"
#include "quickdialog.h"

#define NOVA_CONTEXT "nova/workbench"

namespace nova {
	ContentPage::ContentPage(QWidget* parent, const QString& title, const QIcon& icon, bool needs_tool_bar):
			QWidget(parent), ActionProvider(title), current_view(nullptr), nested_main_window(new QMainWindow()),
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
		if (current_view != nullptr) current_view->Activate(this);
	}
	
	void ContentPage::Close() {
		if (current_view != nullptr) current_view->Close(this);
	}
	
	QWidget* ContentPage::get_content_widget() const {
		return nested_main_window->centralWidget();
	}
	
	void ContentPage::set_title(const QString& title) {
		ActionProvider::set_title(title);
		UpdateTabText();
	}
	
	void ContentPage::set_suffix(const QString& suffix) {
		this->suffix = suffix;
		UpdateTabText();
	}
	
	void ContentPage::set_content_widget(QWidget* content_widget) {
		nested_main_window->setCentralWidget(content_widget);
	}
	
	void ContentPage::DisplayAction(QAction* action, int index, bool, int) {
		if (tool_bar != nullptr) {
			if (index >= tool_bar->actions().count()) tool_bar->addAction(action);
			else tool_bar->insertAction(tool_bar->actions()[index], action);
		}
	}
	
	void ContentPage::DisplaySeparators(bool show_regular, int index_regular, bool, int) {
		if ((tool_bar != nullptr) && show_regular) {
			if (index_regular >= tool_bar->actions().count()) tool_bar->addSeparator();
			else tool_bar->insertSeparator(tool_bar->actions()[index_regular]);
		}
	}
	
	void ContentPage::UpdateTabText() {
		if (current_view != nullptr) {
			current_view->setTabText(current_view->ListPages().indexOf(this), (get_title() + suffix));
		}
	}
	
	ContentTabView::ContentTabView(Workbench* window):
			ContentView(window), QTabWidget(window) {
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
		
		auto* tab_list_action = new QAction(NOVA_TR("Tabs"), this);
		tab_list_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
		tool_bar->addAction(tab_list_action);
		connect(tab_list_action, &QAction::triggered, this, &ContentTabView::actTabListTriggered);
		
		auto* detach_action = new QAction(NOVA_TR("Detach Group"), this);
		detach_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
		tool_bar->addAction(detach_action);
		
		auto* close_action = new QAction(NOVA_TR("Close Group"), this);
		close_action->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
		tool_bar->addAction(close_action);
		connect(close_action, &QAction::triggered, [this]() { Close(); });
		
		connect(tabBar(), &QTabBar::customContextMenuRequested, this, &ContentTabView::tabBarCustomContextMenuRequested);
		connect(this, &ContentTabView::currentChanged, [this](int index) {
			/*
			 * Also executed when closing and adding new tabs, in this case the list is not synchronized with the tabs,
			 * so the signal has to be emitted manually later on.
			 */
			if (content_pages.count() == count()) {
				emit workbench_window->currentContentPageChanged(content_pages[index], this);
			}
		});
		connect(this, &ContentTabView::tabCloseRequested, [this](int index) { Close(index); });
		connect(tabBar(), &QTabBar::tabMoved, [this](int from, int to) { content_pages.move(from, to); });
	}
	
	void ContentTabView::Activate(int index) {
		if ((index >= 0) && (index < count())) {
			setCurrentIndex(index);
		}
	}
	
	void ContentTabView::Close(int index) {
		removeTab(index);
		delete content_pages[index];
		content_pages.removeAt(index);
		
		if (count() == 0) {
			emit workbench_window->currentContentPageChanged(nullptr, nullptr);
			deleteLater();
		} else {
			emit workbench_window->currentContentPageChanged(content_pages[currentIndex()], this);
		}
	}
	
	void ContentTabView::CloseMultiple(bool left, bool current, bool right) {
		const int left_count = currentIndex();
		const int right_count = (count() - left_count - 1);
		int start_index_right = 0;
		
		if (left) {
			for (int i = 0 ; i < left_count ; ++i) {
				Close(0);
			}
		} else start_index_right += left_count;
		
		if (current) Close(0);
		else ++start_index_right;
		
		if (right) {
			for (int i = start_index_right ; i < (start_index_right + right_count) ; ++i) {
				Close(start_index_right);
			}
		}
	}
	
	void ContentTabView::Open(ContentPage* page) {
		const int new_index = addTab(page, page->get_icon(), (page->get_title() + page->get_suffix()));
		tabBar()->setTabToolTip(new_index, page->get_title());
		content_pages.insert(new_index, page);
		page->current_view = this;
		
		setCurrentIndex(new_index);
		emit workbench_window->currentContentPageChanged(page, this);
	}
	
	void ContentTabView::tabBarCustomContextMenuRequested(const QPoint& pos) {
		const int index = tabBar()->tabAt(pos);
		if (index != -1) {
			QMenu menu(tabBar());
			
			if (workbench_window->get_standard_action(Workbench::Action_Close) != nullptr) menu.addAction(workbench_window->get_standard_action(Workbench::Action_Close));
			if (workbench_window->get_standard_action(Workbench::Action_CloseOthers) != nullptr) menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseOthers));
			if (workbench_window->get_standard_action(Workbench::Action_CloseTabsLeft) != nullptr) menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseTabsLeft));
			if (workbench_window->get_standard_action(Workbench::Action_CloseTabsRight) != nullptr) menu.addAction(workbench_window->get_standard_action(Workbench::Action_CloseTabsRight));
			
			if (menu.isEmpty()) return;
			
			setCurrentIndex(index);
			menu.exec(tabBar()->mapToGlobal(pos));
		}
	}
	
	void ContentTabView::actTabListTriggered() {
		QStringList names;
		QList<QIcon> icons;
		
		for (const ContentPage* i : ListPages()) {
			names << i->get_title();
			icons << i->get_icon();
		}
		
		const int index = QuickDialog::InputItemIndex(workbench_window, NOVA_TR("Tabs"), names, icons, currentIndex(), true);
		if (index != -1) setCurrentIndex(index);
	}
}
