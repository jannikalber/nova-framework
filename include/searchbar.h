/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_SEARCHBAR_H
#define NOVA_FRAMEWORK_SEARCHBAR_H

#include <QObject>
#include <QList>

#include "nova.h"
#include "quickdialog.h"

class QAction;
class QKeyEvent;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace nova { class Workbench; }

namespace nova {
	/**
	 * @brief A dialog which can be used to browse the application's content
	 * @headerfile searchbar.h <nova/searchbar.h>
	 *
	 * The dialog consists of a line edit which proposes matching actions from all nova::ActionProvider subtypes being registered.
	 * The results can be immediately invoked by keyboard. Checkable actions contain a check box to change their state.
	 *
	 * The translations belong to the context "nova/searchbar".
	 *
	 * @sa nova::ActionProvider
	 */
	class NOVA_API SearchBar : public QuickDialog {
		Q_OBJECT
		
		public:
			/**
			 * @brief Creates a new search bar dialog.
			 *
			 * Use exec() to run it.
			 *
			 * @param window The workbench whose actions can be found (optional, default: nova::workbench)
			 */
			explicit SearchBar(Workbench* window = workbench);
			NOVA_DISABLE_COPY(SearchBar)
		
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			void keyPressEvent(QKeyEvent* event) override;
		
		private:
			QLineEdit* search_bar;
			QTreeWidget* results;
			QList<QAction*> action_results;
		
		private slots:
			void suggest();
			void trigger(QTreeWidgetItem* item);
	};
}

#endif  // NOVA_FRAMEWORK_SEARCHBAR_H
