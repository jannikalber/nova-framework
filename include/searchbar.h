/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_SEARCHBAR_H
#define NOVA_FRAMEWORK_SEARCHBAR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QList>

#include "nova.h"
#include "quickdialog.h"

QT_BEGIN_NAMESPACE
QT_USE_NAMESPACE
class QAction;
class QKeyEvent;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace nova {
	class Workbench;
	
	/**
	 * @brief Represents the dialog which is used to browse the application's content.
	 * @headerfile searchbar.h <nova/searchbar.h>
	 *
	 * The dialog is a line edit which proposes matching actions from all ActionProvider subtypes having the workbench as parent.
	 * The results can be invoked immediately by keyboard. These results being checkable contain a check box to change their state.
	 */
	class NOVA_API SearchBar : public QuickDialog {
		Q_OBJECT
		
		public:
			/**
			 * @brief Creates a new search bar.
			 *
			 * Use exec() to run it.
			 *
			 * The translations belong to the context "nova/searchbar".
			 *
			 * @param parent The workbench whose actions can be found (optional, default: nova::workbench)
			 */
			explicit SearchBar(Workbench* parent = workbench);
		
		protected:
			/**
			 * @brief Reimplements QWidget::keyPressEvent()
			 */
			void keyPressEvent(QKeyEvent* event);
		
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