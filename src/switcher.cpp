/*
 Copyright (c) 2024 by Jannik Alber.
 All rights reserved.
 */

#include "switcher.h"

#include <Qt>
#include <QBrush>
#include <QString>
#include <QIcon>
#include <QApplication>
#include <QWidget>
#include <QHideEvent>
#include <QKeyEvent>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>

#include "workbench.h"
#include "toolwindow.h"
#include "contentpage.h"
#include "ui_switcher.h"

#define NOVA_CONTEXT "nova/switcher"

namespace nova {
	Switcher::Switcher(Workbench* window):
			QuickDialog(window, NOVA_TR("Switcher")), ui(new Ui::Switcher()), current(nullptr) {
		auto* content_widget = new QWidget(this);
		ui->setupUi(content_widget);
		set_content_widget(content_widget);
		
		keyboard_triggered = (QApplication::keyboardModifiers() & Qt::ControlModifier);
		
		// Fill the switcher
		for (ContentPage* i: window->ListPages()) {
			new QListWidgetItem(i->get_icon(), i->get_title(), ui->liwContentPages);
			content_page_selections << i;
		}
		
		for (ToolWindow* i: window->get_tool_windows()) {
			if (i->isVisible()) {
				new QListWidgetItem(i->get_title(), ui->liwToolWindows);
				tool_window_selections << i;
			}
		}
		
		// If nothing is found
		if (content_page_selections.isEmpty()) {
			auto* item = new QListWidgetItem(ui->liwContentPages);
			item->setText(NOVA_TR("Nothing found"));
			item->setFlags(Qt::ItemIsEnabled);
			item->setForeground(QBrush(Qt::gray));
		}
		
		if (tool_window_selections.isEmpty()) {
			ui->liwToolWindows->setVisible(false);
		}
		
		// Find current item
		QWidget * current_widget = QApplication::focusWidget();
		while ((current_widget != nullptr) && (dynamic_cast<ToolWindow*>(current_widget) == nullptr)) {
			current_widget = current_widget->parentWidget();
		}
		
		if (current_widget != nullptr) {
			ToolWindow* tool_window = dynamic_cast<ToolWindow*>(current_widget);
			int index = tool_window_selections.indexOf(tool_window);
			if (index == -1) index = 0;
			
			ui->liwToolWindows->setCurrentRow(index);
			ui->liwToolWindows->setFocus();
			current = ui->liwToolWindows;
		} else if (window->get_current_page() != nullptr) {
			int index = content_page_selections.indexOf(window->get_current_page());
			if (index == -1) index = 0;
			
			ui->liwContentPages->setCurrentRow(index);
			ui->liwContentPages->setFocus();
			current = ui->liwContentPages;
		} else if (!content_page_selections.isEmpty()) {
			ui->liwContentPages->setCurrentRow(0);
			ui->liwContentPages->setFocus();
			current = ui->liwContentPages;
		}
		
		// If the switch was requested by keyboard, directly switch to the next item
		if (keyboard_triggered) {
			SelectNextItem(true);
		}
		
		connect(ui->liwContentPages, &QListWidget::itemClicked, this, &Switcher::accept);
		connect(ui->liwToolWindows, &QListWidget::itemClicked, this, &Switcher::accept);
		connect(ui->liwContentPages, &QListWidget::itemSelectionChanged,
		        [this]() { ChangeCurrentView(ui->liwContentPages); });
		connect(ui->liwToolWindows, &QListWidget::itemSelectionChanged,
		        [this]() { ChangeCurrentView(ui->liwToolWindows); });
	}
	
	Switcher::~Switcher() noexcept {
		delete ui;
	}
	
	void Switcher::hideEvent(QHideEvent* event) {
		// hideEvent() is required, because accept() and reject() don't fire closeEvent()
		QuickDialog::hideEvent(event);
		
		if (event->spontaneous()) {
			return;
		}
		
		if (result() == QDialog::Accepted) {
			// Activate the selected view
			if (!ui->liwContentPages->selectedItems().isEmpty()) {
				content_page_selections[ui->liwContentPages->currentRow()]->Activate();
			} else if (!ui->liwToolWindows->selectedItems().isEmpty()) {
				tool_window_selections[ui->liwToolWindows->currentRow()]->Activate();
			}
		}
	}
	
	void Switcher::keyPressEvent(QKeyEvent* event) {
		switch (event->key()) {
			case Qt::Key_Tab:
				SelectNextItem();
				break;
			
			case Qt::Key_Left:
				ChangeCurrentView(ui->liwToolWindows);
				break;
			
			case Qt::Key_Right:
				ChangeCurrentView(ui->liwContentPages);
				break;
			
			case Qt::Key_Return:
				accept();
				break;
			
			default:
				QuickDialog::keyPressEvent(event);
				break;
		}
	}
	
	void Switcher::keyReleaseEvent(QKeyEvent* event) {
		QuickDialog::keyReleaseEvent(event);
		
		if (keyboard_triggered && event->key() == Qt::Key_Control) {
			accept();
		}
	}
	
	void Switcher::ChangeCurrentView(QListWidget* new_view) {
		if ((current == nullptr) || (new_view == current) || !new_view->isVisible()) {
			return;
		}
		
		// Work on the current view before changing it
		int index = current->currentRow();
		current->clearSelection();
		
		current = new_view;
		
		// Select an appropriate item in the new view if necessary
		if (new_view->selectedItems().isEmpty()) {
			if (index >= new_view->count()) {
				index = (new_view->count() - 1);
			}
			
			new_view->setCurrentRow(index);
		}
		
		new_view->setFocus();
	}
	
	void Switcher::SelectNextItem(bool stay_in_current_column) {
		if (current != nullptr) {
			if ((current->currentRow() + 1) < current->count()) {
				current->setCurrentRow(current->currentRow() + 1);
			} else if ((((current == ui->liwContentPages) && !stay_in_current_column)
			            || ((current == ui->liwToolWindows) && stay_in_current_column))
			           && !tool_window_selections.isEmpty()) {
				ui->liwToolWindows->setCurrentRow(0);
			} else {
				ui->liwContentPages->setCurrentRow(0);
			}
		}
	}
}
