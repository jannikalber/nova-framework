/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "quickdialog.h"

#include <QtCore/Qt>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QDockWidget>

#include "ui_quickdialog.h"

namespace nova {
	QuickDialog::QuickDialog(QWidget* parent, const QString& title)
			: QDialog(parent), ui(new Ui::QuickDialog()), input_widget(nullptr) {
		ui->setupUi(this);
		set_title(title);
	}
	
	QuickDialog::~QuickDialog() noexcept {
		// Popups can issue some display errors, so the parent should be repainted.
		dynamic_cast<QWidget*>(parent())->repaint();
		
		delete ui;
	}
	
	void QuickDialog::keyPressEvent(QKeyEvent* event) {
		if (event->key() == Qt::Key_Return) accept();
		QDialog::keyPressEvent(event);
	}
	
	void QuickDialog::set_title(const QString& title) {
		this->title = title;
		ui->dcwTitle->setWindowTitle(title);
	}
	
	void QuickDialog::set_input_widget(QWidget* input_widget) {
		this->input_widget = input_widget;
		ui->dcwTitle->setWidget(input_widget);
		
		input_widget->setFocus();
	}
	
	QString QuickDialog::InputText(QWidget* parent, const QString& title, const QString& placeholder,
	                               QLineEdit::EchoMode mode, const QString& default_text) {
		QuickDialog dialog(parent, title);
		
		QLineEdit line_edit(&dialog);
		line_edit.setMinimumWidth(350);
		line_edit.setPlaceholderText(placeholder);
		line_edit.setEchoMode(mode);
		line_edit.setText(default_text);
		dialog.set_input_widget(&line_edit);
		
		if (dialog.exec() == QDialog::Accepted) return line_edit.text();
		else return "";
	}
	
	QString QuickDialog::InputItem(QWidget* parent, const QString& title, const QStringList& items,
	                               const QList<QIcon>& icons) {
		QuickDialog dialog(parent, title);
		
		QListWidget list_widget(&dialog);
		for (const QString& i : items) {
			list_widget.addItem(i);
		}
		for (int i = 0; i < icons.length(); ++i) {
			QListWidgetItem* item = list_widget.item(i);
			if (item != nullptr) item->setIcon(icons.at(i));
		}
		
		dialog.set_input_widget(&list_widget);
		// Make the dialog as small as possible
		list_widget.setMinimumWidth(150);
		list_widget.setMaximumSize(list_widget.sizeHintForColumn(0) + 2 * list_widget.frameWidth(),
		                         list_widget.sizeHintForRow(0) * list_widget.count() + 2 * list_widget.frameWidth());
		list_widget.setCurrentRow(0);
		
		QObject::connect(&list_widget, &QListWidget::itemClicked, &dialog, &QuickDialog::accept);
		
		// Select by hovering
		list_widget.setMouseTracking(true);
		QObject::connect(&list_widget, &QListWidget::itemEntered,
		                 [&list_widget](QListWidgetItem* item) {
			                 list_widget.setCurrentItem(item);
		                 });
		
		if (dialog.exec() == QDialog::Accepted) return list_widget.currentItem()->text();
		else return "";
	}
}
