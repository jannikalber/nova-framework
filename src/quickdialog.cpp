/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "quickdialog.h"

#include <Qt>
#include <QObject>
#include <QString>
#include <QSize>
#include <QRect>
#include <QIcon>
#include <QKeyEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDockWidget>

#include "ui_quickdialog.h"

namespace nova {
	QuickDialog::QuickDialog(QWidget* parent, const QString& title):
			QDialog(parent), ui(new Ui::QuickDialog()), content_widget(nullptr) {
		ui->setupUi(this);
		set_title(title);
	}
	
	QuickDialog::~QuickDialog() noexcept {
		// Popups can issue some display errors, so the parent should be repainted.
		dynamic_cast<QWidget*>(parent())->repaint();
		
		delete ui;
	}
	
	QString QuickDialog::InputText(QWidget* parent, const QString& title, const QString& placeholder,
	                               QLineEdit::EchoMode mode, const QString& default_text, bool under_cursor) {
		QuickDialog dialog(parent, title);
		
		QLineEdit line_edit(&dialog);
		line_edit.setMinimumWidth(350);
		line_edit.setPlaceholderText(placeholder);
		line_edit.setEchoMode(mode);
		line_edit.setText(default_text);
		line_edit.selectAll();
		dialog.set_content_widget(&line_edit);
		
		if (under_cursor) dialog.PositionAt(QCursor::pos());
		
		if (dialog.exec() == QDialog::Accepted) return line_edit.text();
        else return QString();
	}
	
	int QuickDialog::InputItemIndex(QWidget* parent, const QString& title, const QStringList& items,
	                                const QList<QIcon>& icons, int current_index, bool under_cursor) {
		QuickDialog dialog(parent, title);
		
		QListWidget list_widget(&dialog);
		for (const QString& i : items) {
			list_widget.addItem(i);
		}
		for (int i = 0 ; i < icons.length() ; ++i) {
			QListWidgetItem* item = list_widget.item(i);
			if (item != nullptr) item->setIcon(icons[i]);
		}
		
		dialog.set_content_widget(&list_widget);
		// Make the dialog as small as possible
		list_widget.setMinimumWidth(150);
		list_widget.setIconSize(QSize(16, 16));
		list_widget.setMaximumSize(list_widget.sizeHintForColumn(0) + 2 * list_widget.frameWidth(),
		                           list_widget.sizeHintForRow(0) * list_widget.count() + 2 * list_widget.frameWidth());
		list_widget.setMinimumHeight(list_widget.maximumHeight());
		list_widget.setCurrentRow(current_index);
		
		if (under_cursor) dialog.PositionAt(QCursor::pos());
		
		QObject::connect(&list_widget, &QListWidget::itemClicked, &dialog, &QuickDialog::accept);
		
		// Select by hovering
		list_widget.setMouseTracking(true);
		QObject::connect(&list_widget, &QListWidget::itemEntered,
		                 [&list_widget](QListWidgetItem* item) {
			                 list_widget.setCurrentItem(item);
		                 });
		
		if (dialog.exec() == QDialog::Accepted) return list_widget.currentRow();
		else return -1;
	}
	
	void QuickDialog::PositionAt(const QPoint& point) {
		// Issues internal size calculation, because the size is not available yet.s
		layout()->update();
		layout()->activate();
		
		const int width = geometry().width();
		const int height = geometry().height();
		
		int x = point.x();
		int y = point.y();
		
		const int max_x = (parentWidget()->x() + parentWidget()->width() - 10);
		const int max_y = (parentWidget()->y() + parentWidget()->height() - 10);
		
		if ((x + width) >= max_x) {
			x = (max_x - width);
		} else {
			x -= (width / 2);
			
			if (x < (parentWidget()->x() + 10)) {
				x = (parentWidget()->x() + 10);
			}
		}
		
		if ((y + height) >= max_y) {
			y = (max_y - height);
		} else {
			y -= 10; // 10px = 20px/2 (the dialog's title bar is 20px high)
			
			if (y < (parentWidget()->y() + 10)) {
				y = (parentWidget()->y() + 10);
			}
		}
		
		move(x, y);
	}
	
	QString QuickDialog::get_title() const {
		return ui->dcwTitle->windowTitle();
	}
	
	void QuickDialog::set_title(const QString& title) {
		ui->dcwTitle->setWindowTitle(title);
	}
	
	void QuickDialog::set_content_widget(QWidget* content_widget) {
		this->content_widget = content_widget;
		ui->dcwTitle->setWidget(content_widget);
		
		content_widget->setFocus();
	}
	
	void QuickDialog::keyPressEvent(QKeyEvent* event) {
		QDialog::keyPressEvent(event);
		if (event->key() == Qt::Key_Return) accept();
	}
}
