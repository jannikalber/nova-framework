/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_QUICKDIALOG_H
#define NOVA_FRAMEWORK_QUICKDIALOG_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>

#include "nova.h"

QT_BEGIN_NAMESPACE
QT_USE_NAMESPACE
class QStringList;
class QIcon;
class QKeyEvent;

namespace Ui { class QuickDialog; }
QT_END_NAMESPACE

namespace nova {
	/**
	 * @brief A QuickDialog is a simple dialog which only contains one widget.
	 * @headerfile quickdialog.h <nova/quickdialog.h>
	 *
	 * They are especially suitable for querying a string or one item of a list.
	 *
	 * Quick dialogs should replace Qt's input dialogs.
	 *
	 * @sa QuickDialog::InputText()
	 * @sa QuickDialog::InputItem()
	 */
	class NOVA_API QuickDialog : public QDialog {
		public:
			/**
			 * @brief Constructs a new QuickDialog.
			 *
			 * Set the input widget by calling set_input_widget() and call exec() to run the dialog.
			 *
			 * @param parent The parent window
			 * @param title is shown in the title bar. This is usually a hint for what should be queried
			 * and not the application's name (e.g. "New File").
			 */
			QuickDialog(QWidget* parent, const QString& title = "");
			virtual ~QuickDialog() noexcept;
			
			/**
			 * @brief Sets the title bar's title.
			 *
			 * @param title is shown in the title bar. This is usually a hint for what should be queried
			 * and not the application's name (e.g. "New File").
			 * @sa get_title()
			 */
			void set_title(const QString& title);
			
			/**
			 * @brief Sets the dialog's input widget.
			 *
			 * @param input_widget is the widget to be set. Consider to set the dialog as its parent.
			 * @sa get_input_widget()
			 */
			inline void set_input_widget(QWidget* input_widget);
			
			/**
			 * @brief Returns the current title.
			 *
			 * @return The title or an empty string if none is set.
			 * @sa set_title()
			 */
			inline QString get_title() const { return title; }
			
			/**
			 * @brief Returns the dialog's input widget
			 *
			 * This method also works when the dialog is already closed.
			 *
			 * @return The input widget or nullptr if none is set.
			 * @sa set_input_widget()
			 */
			inline QWidget* get_input_widget() const { return input_widget; }
			
			/**
			 * @brief Queries a string using a prefabricated QuickDialog with a line edit as input widget.
			 *
			 * @param parent The parent window
			 * @param title The title bar's title
			 * @param placeholder A placeholder which is shown in the line edit if it's empty. (optional, default: none)
			 * @param mode is the echo mode the line edit will use (optional, default: QLineEdit::Normal)
			 * @param default_text is the prefilled text in the line edit (optional, default: none)
			 *
			 * @return The line edit's text or an empty string if the dialog was rejected.
			 * @sa InputItem()
			 */
			static QString InputText(QWidget* parent, const QString& title, const QString& placeholder = "",
			                         QLineEdit::EchoMode mode = QLineEdit::Normal, const QString& default_text = "");
			
			/**
			 * @brief Queries one item of a list using a prefabricated QuickDialog with a list widget as input widget.
			 *
			 * @param parent The parent window
			 * @param title The title bar's title
			 * @param items The options which can be selected
			 * @param icons A list of QIcons which are displayed in front of the items. If this list contains more icons than items,
			 * the last icons of the list are ignored, if the list contains less icons than items, the last items get no icons.
			 * It's not recommended to have some items without and some with icons.
			 * (optional, default: no icons)
			 *
			 * The size of the dialog depends on the count of items.
			 *
			 * @return The text of the selected item or an empty string if the dialog was rejected.
			 * @sa InputText()
			 */
			static QString InputItem(QWidget* parent, const QString& title, const QStringList& items,
			                         const QList<QIcon>& icons = QList<QIcon>());
		
		protected:
			/**
			 * @brief Reimplements QWidget::keyPressEvent()
			 */
			void keyPressEvent(QKeyEvent* event) override;
		
		private:
			Ui::QuickDialog* ui;
			QWidget* input_widget;
			QString title;
	};
}

#endif  // NOVA_FRAMEWORK_QUICKDIALOG_H
