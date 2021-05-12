/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "searchbar.h"

#include <QtCore/Qt>
#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtGui/QIcon>
#include <QtGui/QBrush>
#include <QtGui/QKeySequence>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QMessageBox>

#include "workbench.h"
#include "actionprovider.h"

namespace nova {
	SearchBar::SearchBar(Workbench* parent) : QuickDialog(parent), action_results(QList<QAction*>()) {
		set_title(QApplication::translate("nova/searchbar", "Search..."));
		
		auto* widget = new QWidget(this);
		auto* layout = new QVBoxLayout(widget);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		
		search_bar = new QLineEdit(widget);
		results = new QTreeWidget(widget);
		
		search_bar->setMinimumWidth(350);
		search_bar->setPlaceholderText(QApplication::translate("nova/searchbar", "Browse the application"));
		search_bar->setToolTip(QApplication::translate("nova/searchbar", "<b>Note:</b> Wildcard syntax available"));
		
		results->setMouseTracking(true);
		results->setFocusProxy(search_bar);
		results->header()->hide();
		results->headerItem()->setText(1, QString());  // Second column
		results->setRootIsDecorated(false);
		
		results->hide();
		
		layout->addWidget(search_bar);
		layout->addWidget(results);
		
		set_input_widget(widget);
		search_bar->setFocus();
		
		connect(search_bar, &QLineEdit::textEdited, this, &SearchBar::suggest);
		connect(results, &QTreeWidget::itemClicked, this, &SearchBar::trigger);
		connect(results, &QTreeWidget::itemDoubleClicked, this, &SearchBar::trigger);
		connect(results, &QTreeWidget::itemEntered,
		        [this](QTreeWidgetItem* item) {
			        results->setCurrentItem(item);
		        });
	}
	
	void SearchBar::keyPressEvent(QKeyEvent* event) {
		// The dialog shouldn't be immediately closed when return is pressed -> skip QuickDialog::keyPressEvent()
		QDialog::keyPressEvent(event);  // NOLINT
		
		// Key events (see below) might come back if they're not accepted by the widget
		static QKeyEvent* send_event;
		if (event == send_event) return;
		
		switch (event->key()) {
			case Qt::Key_Up:
			case Qt::Key_Down:
			case Qt::Key_PageUp:
			case Qt::Key_PageDown:
				send_event = new QKeyEvent(QEvent::KeyPress, event->key(), event->modifiers());
				QApplication::sendEvent(results, send_event);
				break;
			
			case Qt::Key_Return:
			case Qt::Key_Enter:
				if (results->currentItem() != nullptr) trigger(results->currentItem());
				break;
			
			default:
				break;
		}
	}
	
	// Signals and slots
	void SearchBar::suggest() {
		if (!search_bar->text().isEmpty()) {
			results->show();
			results->clear();
			
			action_results.clear();
			
			const QRegExp reg_exp(search_bar->text(), Qt::CaseInsensitive, QRegExp::WildcardUnix);
			
			for (ActionProvider* i : dynamic_cast<Workbench*>(parent())->providers) {
				for (QAction* j : i->ListActions()) {
					if (!j->toolTip().contains(reg_exp)) continue;
					action_results << j;
					
					auto* item = new QTreeWidgetItem(results);
					
					item->setText(0, j->toolTip() +
					                 (j->shortcut().isEmpty() ? "" : " (" + j->shortcut().toString() +
					                                                 ")"));  // Adding the shortcut if available
					item->setText(1, i->get_title());
					item->setToolTip(0, j->whatsThis());
					item->setFlags(Qt::ItemIsEnabled |
					               Qt::ItemIsSelectable);  // Don't allow the user to check items (see trigger())
					
					item->setTextAlignment(1, Qt::AlignTrailing | Qt::AlignVCenter);  // Right aligned
					QFont font;
					font.setItalic(true);
					item->setFont(1, font);
					
					if (!j->icon().isNull()) item->setIcon(0, j->icon());
					if (j->isCheckable()) item->setCheckState(0, j->isChecked() ? Qt::Checked : Qt::Unchecked);
				}
			}
			
			// If nothing is found
			if (action_results.isEmpty()) {
				auto* item = new QTreeWidgetItem(results);
				item->setText(0, QApplication::translate("nova/searchbar", "Nothing found"));
				item->setFlags(Qt::ItemIsEnabled);
				item->setForeground(0, QBrush(Qt::gray));
			}
			
			results->resizeColumnToContents(0);
			results->setCurrentItem(results->topLevelItem(0));
		} else results->hide();
	}
	
	void SearchBar::trigger(QTreeWidgetItem* item) {
		if (action_results.isEmpty()) return;
		
		QAction* action = action_results.at(results->indexOfTopLevelItem(item));
		
		if (!action->isCheckable()) {
			accept();
			action->trigger();
		} else {
			action->setChecked(!action->isChecked());
			item->setCheckState(0, action->isChecked() ? Qt::Checked : Qt::Unchecked);
		}
	}
}
