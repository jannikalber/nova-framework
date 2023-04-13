/*
 * Copyright (c) 2022 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_CONTENTPAGE_H
#define NOVA_FRAMEWORK_CONTENTPAGE_H

#include <QObject>
#include <QIcon>
#include <QList>
#include <QString>
#include <QWidget>
#include <QTabWidget>

#include "nova.h"
#include "actionprovider.h"

class QPoint;
class QAction;
class QMainWindow;
class QToolBar;
class QMouseEvent;

namespace nova {
	class Workbench;
	class ContentTabView;
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
			virtual ~ContentPage() noexcept = default;
			
			/**
			 * @brief Focuses this page (i.e. the tab of the content view will be changed).
			 *
			 * The page must be opened in a content view for the method to work.
			 *
			 * @sa nova::ContentTabView::Activate(int)
			 * @sa nova::ContentTabView::Activate(ContentPage*)
			 */
			void Activate();
			
			/**
			 * @brief Closes the page in its content view if possible.
			 *
			 * The page must be opened in a content view for the method to work.
			 *
			 * @sa nova::ContentTabView::Close(int)
			 * @sa nova::ContentTabView::Close(nova::ContentPage*)
			 * @sa nova::ContentTabView::CloseCurrent()
			 * @sa nova::ContentTabView::CloseMultiple()
			 */
			void Close();
			
			/**
			 * @brief Returns the page's current tab view or nullptr if there's none.
			 */
			inline ContentTabView* get_current_view() const { return current_view; }
			
			/**
			 * @brief Returns the page's content widget.
			 */
			QWidget* get_content_widget() const;
			
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
			 * @brief Allows to add a suffix to the page's title.
			 *
			 * For example, this could be useful for indicators like "*" for pages that
			 * have unsaved changes.
			 *
			 * @param suffix The suffix to be applied or an empty string to clear the current one
			 *
			 * @sa get_suffix()
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
			 * @param parent The parent window
			 * @param title The page's title. The title can be changed later.
			 * @param icon The icon to be displayed in the content view of the page
			 * @param needs_tool_bar if a tool bar should also be created and shown (optional, default: false)
			 */
			ContentPage(QWidget* parent, const QString& title, const QIcon& icon, bool needs_tool_bar = false);
			
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
			void DisplayAction(QAction* action, int index, bool, int) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void DisplaySeparators(bool show_regular, int index_regular, bool, int) override;
		
		private:
			friend class ContentTabView;
			
			ContentTabView* current_view;
			
			QMainWindow* const nested_main_window;
			QToolBar* const tool_bar;
			
			QIcon icon;
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
			/**
			 * @brief Allows to use nova::ContentView objects as QWidget.
			 */
			virtual operator QWidget*() = 0;
			
			/**
			 * @brief Lists all pages of the view's content pages in the correct order.
			 *
			 * If the view contains nested subviews, all pages are listed (including the pages
			 * of the subviews).
			 *
			 * @return A list of all content pages
			 */
			virtual QList<ContentPage*> ListPages() const = 0;
		
		protected:
			/**
			 * This constructor is internally required and should not be used.
			 */
			inline explicit ContentView(Workbench* window = workbench):
					workbench_window(window) {}
			
			/**
			 * This method is internally required and should not be called.
			 */
			virtual void Open(ContentPage* page) = 0;
			
			/**
			 * This method is internally required and should not be called.
			 */
			virtual void Close() = 0;
			
			/**
			 * This attribute is internally required and should not be used.
			 */
			Workbench* const workbench_window;
		
		private:
			friend class Workbench;
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
	 * The translations belong to the context "nova/workbench".
	 *
	 * It's not possible to create a content tab view manually.
	 */
	class NOVA_API ContentTabView : public QTabWidget, public ContentView {
		Q_OBJECT
		
		public:
			NOVA_DISABLE_COPY(ContentTabView)
			
			/**
			 * This method is internally required.
			 */
			inline operator QWidget*() override { return this; }
			
			/**
			 * @brief See ContentView::ListPages().
			 *
			 * This method is internally required.
			 *
			 * @sa ContentView::ListPages()
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
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseCurrent()
			 * @sa CloseMultiple()
			 */
			void Close(int index);
			
			/**
			 * @brief Closes the given content page if possible.
			 *
			 * This method only works when the view contains the given page.
			 *
			 * @param page The content page to be closed
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa Close(int)
			 * @sa CloseCurrent()
			 * @sa CloseMultiple()
			 */
			inline void Close(ContentPage* page) { Close(content_pages.indexOf(page)); }
			
			/**
			 * @brief Closes the active content page if possible.
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa Close(int)
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseMultiple()
			 */
			inline void CloseCurrent() { Close(currentIndex()); }
			
			/**
			 * @brief Closes multiple content pages at once.
			 *
			 * @param left If this flag is set, all pages to the left of the current one are closed if possible.
			 * @param current If this flag is set, the current page is closed if possible.
			 * @param right If this flag is set, all pages to the right of the current one are closed if possible.
			 *
			 * @sa nova::ContentPage::Close()
			 * @sa Close(int)
			 * @sa Close(nova::ContentPage*)
			 * @sa CloseCurrent()
			 */
			void CloseMultiple(bool left, bool current, bool right);
			
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
			void Open(ContentPage* page) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			inline void Close() override { CloseMultiple(true, true, true); }
		
		private:
			friend class Workbench;
			
			QList<ContentPage*> content_pages;
			
			explicit ContentTabView(Workbench* window = workbench);
		
		private slots:
			void tabBarCustomContextMenuRequested(const QPoint& pos);
			void actTabListTriggered();
	};
}

#endif  //NOVA_FRAMEWORK_CONTENTPAGE_H
