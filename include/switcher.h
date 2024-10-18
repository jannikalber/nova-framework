/*
 Copyright (c) 2024 by Jannik Alber.
 All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_SWITCHER_H
#define NOVA_FRAMEWORK_SWITCHER_H

#include <QList>

#include "nova.h"
#include "quickdialog.h"

class QHideEvent;
class QKeyEvent;
class QListWidget;

namespace Ui { class Switcher; }

namespace nova {
	class Workbench;
	class ToolWindow;
	class ContentPage;
}

namespace nova {
	/**
	 * @brief A dialog which can be used to navigate through the application's content pages and tool windows
	 * @headerfile switcher.h <nova/switcher.h>
	 *
	 * The dialog consists of list widgets which contain all opened content pages and tool windows. The user may
	 * select an item to activate it.
	 *
	 * The dialog is designed to be controlled by keyboard. The selected page or window gets automatically
	 * activated when the dialog closes.
	 *
	 * The translations belong to the context "nova/switcher".
	 *
	 * @sa nova::ContentPage
	 * @sa nova::ToolWindow
	 */
	class NOVA_API Switcher : public QuickDialog {
		public:
			/**
			 * @brief Creates a new switcher dialog.
			 *
			 * The constructor automatically detects the content pages and tool windows
			 * of the given workbench.
			 *
			 * If the [Ctrl] key is pressed during initialization, the dialog will be automatically closed
			 * when releasing the [Ctrl] key. Note that the dialog is designed to be used by the [Ctrl+Tab] shortcut.
			 *
			 * Use exec() to run the dialog.
			 *
			 * @param window The workbench whose content will be displayed in the dialog
			 * (optional, default: nova::workbench)
			 */
			explicit Switcher(Workbench* window = workbench);
			NOVA_DISABLE_COPY(Switcher)
			~Switcher() noexcept;
		
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			void hideEvent(QHideEvent* event) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void keyPressEvent(QKeyEvent* event) override;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void keyReleaseEvent(QKeyEvent* event) override;
		
		private:
			Ui::Switcher* const ui;
			
			// If this option is set, the dialog is closed when releasing [Ctrl].
			bool keyboard_triggered;
			
			QListWidget* current;
			
			QList<ContentPage*> content_page_selections;
			QList<ToolWindow*> tool_window_selections;
			
			void ChangeCurrentView(QListWidget* new_view);
			void SelectNextItem(bool stay_in_current_column = false);
	};
}

#endif  // NOVA_FRAMEWORK_SWITCHER_H
