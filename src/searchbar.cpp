/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "searchbar.h"

#include <Qt>
#include <QString>
#include <QRegExp>
#include <QIcon>
#include <QBrush>
#include <QKeySequence>
#include <QKeyEvent>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

#include "workbench.h"
#include "actionprovider.h"

#define NOVA_CONTEXT "nova/searchbar"

namespace nova {
	SearchBar::SearchBar(Workbench* window) :
			QuickDialog(window, NOVA_TR("Search...")) {
		auto* widget = new QWidget(this);
		auto* layout = new QVBoxLayout(widget);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		
		search_bar = new QLineEdit(widget);
		results = new QTreeWidget(widget);
		
		search_bar->setMinimumWidth(350);
		search_bar->setMaximumWidth(350);
		search_bar->setPlaceholderText(NOVA_TR("Browse the application"));
		search_bar->setToolTip(NOVA_TR("<b>Note:</b> Wildcard syntax available"));
		
		results->setMouseTracking(true);
		results->setFocusProxy(search_bar);
		results->header()->hide();
		results->headerItem()->setText(1, QString());  // Second column
		results->setRootIsDecorated(false);
		
		results->hide();
		
		layout->addWidget(search_bar);
		layout->addWidget(results);
		
		set_content_widget(widget);
		search_bar->setFocus();
		
		connect(search_bar, &QLineEdit::textEdited, this, &SearchBar::suggest);
		connect(results, &QTreeWidget::itemClicked, this, &SearchBar::trigger);
		connect(results, &QTreeWidget::itemDoubleClicked, this, &SearchBar::trigger);
		connect(results, &QTreeWidget::itemEntered, [this](QTreeWidgetItem* item) {
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
	
	void SearchBar::suggest() {
		if (!search_bar->text().isEmpty()) {
			results->show();
			results->clear();
			
			action_results.clear();
			
			const QRegExp reg_exp(search_bar->text(), Qt::CaseInsensitive, QRegExp::WildcardUnix);
			
			for (const ActionProvider* i: dynamic_cast<const Workbench*>(parent())->providers) {
				for (QAction* j: i->ListActions()) {
					if (!j->isVisible() || (reg_exp.indexIn(j->toolTip()) == -1)) continue;
					action_results << j;
					
					auto* item = new QTreeWidgetItem(results);
					
					item->setText(0, j->toolTip() +  // Adding the shortcut if available
					                 (j->shortcut().isEmpty() ? "" : " (" + j->shortcut().toString() + ")"));
					item->setText(1, i->get_title());
					item->setToolTip(0, j->whatsThis());
					// Don't allow the user to check items (see trigger()) (no Qt::ItemIsUserCheckable)
					item->setFlags(j->isEnabled() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags);
					
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
				item->setText(0, NOVA_TR("Nothing found"));
				item->setFlags(Qt::ItemIsEnabled);
				item->setForeground(0, QBrush(Qt::gray));
			}
			
			results->resizeColumnToContents(0);
			results->setCurrentItem(results->topLevelItem(0));
		} else results->hide();
	}
	
	void SearchBar::trigger(QTreeWidgetItem* item) {
		if (action_results.isEmpty()) return;
		
		QAction* action = action_results[results->indexOfTopLevelItem(item)];
		if (!action->isEnabled()) return;
		
		if (!action->isCheckable()) {
			accept();
			action->trigger();
		} else {
			action->setChecked(!action->isChecked());
			item->setCheckState(0, action->isChecked() ? Qt::Checked : Qt::Unchecked);
		}
	}
}
