/*
 * Copyright (c) 2022 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_CONTENTPAGE_H
#define NOVA_FRAMEWORK_CONTENTPAGE_H

#include <Qt>
#include <QObject>
#include <QIcon>
#include <QList>
#include <QString>
#include <QWidget>
#include <QTabWidget>
#include <QSplitter>

#include "nova.h"
#include "actionprovider.h"

class QPoint;
class QAction;
class QMainWindow;
class QToolBar;

namespace nova {
	class Workbench;
	class ContentTabView;
	class ContentSplitView;
}

namespace nova {
	/**
	 * @brief A nova::ContentPage is a page in the workbench's main view (e.g. a text editor).
	 * @headerfile contentpage.h <nova/contentpage.h>
	 *
	 * Content pages can have a tool bar with actions. Therefore, a content page is a nova::ActionProvider.
	 * In difference to other action providers, content pages are only registered when they're active.
	 *
	 * Content pages are opened in nova::ContentView subclasses (especially nova::ContentTabView). The workbench
	 * automatically manages its content views. Use nova::Workbench::OpenContentPage() to add a new page.
	 *
	 * This class must be derived.
	 *
	 * @sa nova::Workbench::OpenContentPage()
	 * @sa nova::ContentView
	 */
	class NOVA_API ContentPage : public QWidget, public ActionProvider {
		public:
			NOVA_DISABLE_COPY(ContentPage)
			inline virtual ~ContentPage() noexcept = default;
			
			/**
			 * @brief Focuses this page (i.e. the tab of the content view will be changed).
			 *
			 * The page must be opened in a content view for the method to work.
			 *
			 * @sa nova::ContentTabView::Activate(int)
			 * @sa nova::ContentTabView::Activate(ContentPage*)
			 * @sa IsActive()
			 */
			void Activate();
			
			/**
			 * @brief Checks if the page is currently active.
			 *
			 * @return true if active
			 * @sa Activate()
			 */
			bool IsActive() const;
			
			/**
			 * @brief Closes the page in its content view if possible.
			 *
			 * The page must be opened in a content view for the method to work.
			 *
			 * @return if the page was closed successfully
			 *
			 * @sa nova::Workbench::CloseAllContentPages()
			 * @sa nova::ContentTabView::Close(int)
			 * @sa nova::ContentTabView::Close(nova::ContentPage*)
			 * @sa nova::ContentTabView::CloseCurrent()
			 * @sa nova::ContentTabView::CloseMultiple()
			 */
			bool Close();
			
			/**
			 * @brief Returns the page's current tab view or nullptr if there's none.
			 */
			inline ContentTabView* get_current_view() const { return current_view; }
			
			/**
			 * @brief Returns the page's content widget.
			 */
			QWidget* get_content_widget() const;
			
			/**
			 * @brief Returns a pointer to the content page's tool bar or nullptr if it's never created.
			 */
			inline QToolBar* get_tool_bar() const { return tool_bar; }
			
			/**
			 * @brief nova::ContentPage supports changeable titles.
			 *
			 * This method is internally required.
			 */
			void set_title(const QString& title) override;
			
			/**
			 * @brief Returns the page's icon being displayed in its content view.
			 */
			inline QIcon get_icon() const { return icon; }
			
			/**
			 * @brief Allows to add a prefix to the page's title.
			 *
			 * For example, this could be useful for indicators like "*" for pages that
			 * have unsaved changes.
			 *
			 * @param prefix The prefix to be applied or an empty string to clear the current one
			 *
			 * @sa get_prefix()
			 * @sa get_suffix()
			 * @sa set_suffix()
			 */
			void set_prefix(const QString& prefix);
			
			/**
			 * @brief Returns the current prefix or an empty string if there's none.
			 */
			inline QString get_prefix() const { return prefix; }
			
			/**
			 * @brief Allows to add a suffix to the page's title.
			 *
			 * For example, this could be useful for indicators like "*" for pages that
			 * have unsaved changes.
			 *
			 * @param suffix The suffix to be applied or an empty string to clear the current one
			 *
			 * @sa get_suffix()
			 * @sa get_prefix()
			 * @sa set_prefix()
			 */
			void set_suffix(const QString& suffix);
			
			/**
			 * @brief Returns the current suffix or an empty string if there's none.
			 */
			inline QString get_suffix() const { return suffix; }
		
		protected:
			/**
			 * @brief Creates a new nova::ContentPage.
			 *
			 * Use set_content_widget() to change the page's content.
			 *
			 * @param title The page's title. The title can be changed later.
			 * @param icon The icon to be displayed in the content view of the page
			 * @param window The associated workbench (optional, default: nova::workbench)
			 * @param needs_tool_bar if a tool bar should also be created and shown (optional, default: false)
			 */
			ContentPage(const QString& title, const QIcon& icon, Workbench* window = workbench, bool needs_tool_bar = false);
			
			/**
			 * @brief This virtual method is called when the page is about to close.
			 *
			 * If the return value evaluates to false, the page won't be closed.
			 * Your implementation could for example ask the user if dirty files should be saved before closing.
			 *
			 * The default implementation returns true.
			 *
			 * @return true if the page can be closed
			 */
			inline virtual bool CanClose() { return true; }
			
			/**
			 * @brief Sets the page's content widget.
			 *
			 * Call this method in the constructor of your content page.
			 * The method takes ownership of the pointer and deletes it at appropriate time.
			 *
			 * @param content_widget The widget to be set
			 */
			void set_content_widget(QWidget* content_widget);
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplayAction(QAction* action, int index, bool is_important_action, int important_actions_index) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplaySeparators(bool show_regular, int index_regular,
			                       bool show_important_actions, int index_important_actions) override;
			
		private:
			friend class ContentTabView;
			
			ContentTabView* current_view;
			
			QMainWindow* const nested_main_window;
			QToolBar* const tool_bar;
			QList<QAction*> menu_actions;
			
			QIcon icon;
			QString prefix;
			QString suffix;
			
			void UpdateTabText();
	};
	
	/**
	 * @brief A nova::ContentView can display nova::ContentPage objects.
	 * @headerfile contentpage.h <nova/contentpage.h>
	 *
	 * This class is abstract and has exactly two subclasses: nova::ContentTabView
	 * and nova::ContentSplitView. Please do not provide an own implementation of
	 * nova::ContentView.
	 *
	 * Content views can be nested.
	 */
	class NOVA_API ContentView {
		public:
			NOVA_DISABLE_COPY(ContentView)
			virtual ~ContentView() noexcept = default;
			
			/**
			 * @brief Allows to use nova::ContentView objects as QWidget.
			 */
			virtual operator QWidget*() = 0;
			
			/**
			 * @brief Lists all pages of the view in the correct order.
			 *
			 * If the view contains nested subviews, all pages are listed (including the pages
			 * of the subviews).
			 *
			 * @return A list of all content pages
			 * @sa nova::Workbench::ListPages()
			 */
			virtual QList<ContentPage*> ListPages() const = 0;
		
		protected:
			/**
			 * This constructor is internally required and should not be used.
			 */
			inline ContentView(ContentSplitView* parent_view, Workbench* window):
					workbench_window(window), parent_view(parent_view) {}
			
			/**
			 * This method is internally required and should not be called.
			 */
			virtual bool Close() = 0;
			
		private:
			friend class Workbench;
			friend class ContentSplitView;
			friend class ContentTabView;
			friend class ContentPage;
			
			Workbench* const workbench_window;
			ContentSplitView* parent_view;
	};
	
	/**
	 * @brief A nova::ContentTabView can display nova::ContentPage objects.
	 * @headerfile contentpage.h <nova/contentpage.h>
	 *
	 * The view can display multiple content pages. It cannot contain other content
	 * views.
	 *
	 * The pages are displayed in different tabs. There's always an active tab.
	 * So, empty content tab views are not possible.
	 *
	 * The view has three additional buttons:
	 * <ol>
	 *  <li>"Tabs" allows to switch between the tabs by providing an overview of all available tabs.</li>
	 *  <li>"Detach Group" to move the whole view to a separate window.</li>
	 *  <li>"Close Group" to close the whole group (every content page).</li>
	 * </ol>
	 *
	 * It's not possible to create a content tab view manually.
	 *
	 * The translations belong to the context "nova/workbench".
	 */
	class NOVA_API ContentTabView : public QTabWidget, public ContentView {
		Q_OBJECT
		
		public:
			NOVA_DISABLE_COPY(ContentTabView)
			
			/**
			 * @brief See nova::ContentView::operator QWidget*().
			 * @sa nova::ContentView::operator QWidget*()
			 */
			inline operator QWidget*() override { return this; }
			
			/**
			 * @brief See nova::ContentView::ListPages().
			 * @sa nova::ContentView::ListPages()
			 */
			inline QList<ContentPage*> ListPages() const override { return content_pages; }
			
			/**
			 * @brief Focuses the content page at the given index (i.e. the tab will be changed).
			 *
			 * @param index The index of the content page to be focused
			 *
			 * @sa nova::ContentPage::Activate()
			 * @sa Activate(nova::ContentPage*)
			 */
			void Activate(int index);
			
			/**
			 * @brief Focuses the given content page (i.e. the tab will be changed).
			 *
			 * This method only works when the view contains the given page.
			 *
			 * @param page The content page to be focused
			 *
			 * @sa nova::ContentPage::Activate()
			 * @sa Activate(int)
			 */
			inline void Activate(ContentPage* page) { Activate(content_pages.indexOf(page)); }
			
			/**
			 * @brief Closes the content page at the given index if possible.
			 *
			 * @param index The index of the content page to be closed
			 * @return if the page was closed successfully
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa nova::Workbench::CloseAllContentPages()
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseCurrent()
			 * @sa CloseMultiple()
			 */
			bool Close(int index);
			
			/**
			 * @brief Closes the given content page if possible.
			 *
			 * This method only works when the view contains the given page.
			 *
			 * @param page The content page to be closed
			 * @return if the page was closed successfully
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa nova::Workbench::CloseAllContentPages()
			 * @sa Close(int)
			 * @sa CloseCurrent()
			 * @sa CloseMultiple()
			 */
			inline bool Close(ContentPage* page) { return Close(content_pages.indexOf(page)); }
			
			/**
			 * @brief Closes the active content page if possible.
			 *
			 * @return if the page was closed successfully
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa nova::Workbench::CloseAllContentPages()
			 * @sa Close(int)
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseMultiple()
			 */
			inline bool CloseCurrent() { return Close(currentIndex()); }
			
			/**
			 * @brief Closes multiple content pages at once.
			 *
			 * @param left If this flag is set, all pages to the left of the current one are closed if possible.
			 * @param current If this flag is set, the current page is closed if possible.
			 * @param right If this flag is set, all pages to the right of the current one are closed if possible.
			 * @return true if all pages were closed successfully
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa nova::Workbench::CloseAllContentPages()
			 * @sa Close(int)
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseCurrent()
			 */
			bool CloseMultiple(bool left, bool current, bool right);
			
			/**
			 * @brief Spits the view at the given index.
			 *
			 * @param index The index of the page which should be moved to the new view
			 * @param orientation Qt::Vertical for "Split Down" or Qt::Horizontal for "Split right"
			 *
			 * @sa Split(nova::ContentPage*, Qt::Orientation)
			 */
			void Split(int index, Qt::Orientation orientation);
			
			/**
			 * @brief Spits the view at the given page.
			 *
			 * @param page The age which should be moved to the new view. The page must belong to this view.
			 * @param orientation Qt::Vertical for "Split Down" or Qt::Horizontal for "Split right"
			 *
			 * @sa Split(int, Qt::Orientation)
			 */
			inline void Split(ContentPage* page, Qt::Orientation orientation) {
				Split(content_pages.indexOf(page), orientation);
			}
			
			/**
			 * @brief Opens the given page in the view.
			 *
			 * @param page The page to be opened
			 * @sa nova::Workbench::OpenContentPage()
			 */
			void Open(ContentPage* page);
			
			/**
			 * @brief Returns the active content page.
			 *
			 * In fact, the active page of the view is returned and not the one of the workbench.
			 *
			 * @sa nova::Workbench::get_current_page()
			 */
			inline ContentPage* get_current_page() const { return content_pages[currentIndex()]; }
		
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			inline bool Close() override { return CloseMultiple(true, true, true); }
		
		private:
			friend class ContentPage;
			friend class Workbench;
			
			QList<ContentPage*> content_pages;
			
			ContentTabView(ContentSplitView* parent_view, Workbench* window);
		
		private slots:
			void tabBarClicked();
			void tabBarCustomContextMenuRequested(const QPoint& pos);
			void actTabListTriggered();
	};
	
	/**
	 * @brief A nova::ContentSplitView can display two nova::ContentViewobjects.
	 * @headerfile contentpage.h <nova/contentpage.h>
	 *
	 * The view can display multiple content views. The class is internally required for split
	 * functionality and should not be used.
	 */
	class NOVA_API ContentSplitView : public QSplitter, public ContentView {
		public:
			NOVA_DISABLE_COPY(ContentSplitView)
			
			/**
			 * @brief See nova::ContentView::operator QWidget*().
			 * @sa nova::ContentView::operator QWidget*()
			 */
			inline operator QWidget*() override { return this; }
			
			/**
			 * @brief See nova::ContentView::ListPages().
			 * @sa nova::ContentView::ListPages()
			 */
			QList<ContentPage*> ListPages() const override;
			
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			inline bool Close() override { return view_1->Close() && view_2->Close(); }
			
		private:
			friend class ContentTabView;
			
			ContentSplitView(ContentView* view_1, ContentView* view_2, Qt::Orientation orientation,
							 ContentSplitView* parent_view, Workbench* window);
			
			// The view gets replaced by the remaining view when removing victim_view
			void Merge(ContentTabView* victim_view);
			
			void Replace(ContentView* old_view, ContentView* new_view, const QList<int>& restore_sizes);
			
			ContentView* view_1;
			ContentView* view_2;
	};
}

#endif  // NOVA_FRAMEWORK_CONTENTPAGE_H
