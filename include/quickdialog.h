/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_QUICKDIALOG_H
#define NOVA_FRAMEWORK_QUICKDIALOG_H

#include <QList>
#include <QPoint>
#include <QStringList>
#include <QWidget>
#include <QDialog>
#include <QLineEdit>

#include "nova.h"

class QIcon;
class QString;
class QKeyEvent;

namespace Ui { class QuickDialog; }

namespace nova {
	/**
	 * @brief A nova::QuickDialog is a simple dialog which only contains one widget.
	 * @headerfile quickdialog.h <nova/quickdialog.h>
	 *
	 * They are especially suitable for querying a string or an item of a list.
	 *
	 * Quick dialogs should replace Qt's input dialogs, because they provide a modern look
	 * and feel.
	 *
	 * @sa InputText()
	 * @sa InputItem()
	 */
	class NOVA_API QuickDialog : public QDialog {
		public:
			/**
			 * @brief Creates a new nova::QuickDialog.
			 *
			 * Set the content widget by calling set_content_widget() and run the dialog by calling exec().
			 *
			 * @param parent The parent window
			 * @param title is shown in the title bar. This is usually a hint for what should be queried
			 * and not the application's name (e.g. "New File").
			 */
			explicit QuickDialog(QWidget* parent, const QString& title);
			NOVA_DISABLE_COPY(QuickDialog)
			virtual ~QuickDialog() noexcept;
			
			/**
			 * @brief Queries a string using a prefabricated quick dialog with a line edit as content widget.
			 *
			 * @param parent The parent window
			 * @param title The dialog's title
			 * @param placeholder A placeholder which is shown in the line edit if it's empty. (optional, default: none)
			 * @param mode is the echo mode the line edit will use (optional, default: QLineEdit::Normal)
			 * @param default_text is the prefilled text in the line edit (optional, default: none)
			 * @param under_cursor If this option is enabled, the dialog is centered under the cursor's position using PositionAt().
			 * (optional, default: off)
			 *
             * @return The line edit's text or a null string if the dialog was rejected.
             *
			 * @sa InputItem()
			 */
			static QString InputText(QWidget* parent, const QString& title, const QString& placeholder = QString(),
			                         QLineEdit::EchoMode mode = QLineEdit::Normal, const QString& default_text = QString(), bool under_cursor = false);
			
			/**
			 * @brief Queries the index of one item in a list using a prefabricated QuickDialog with a list widget as content widget.
			 *
			 * @param parent The parent window
			 * @param title The dialog's title
			 * @param items The options which can be selected
			 * @param icons A list of QIcons which are displayed in front of the items. If this list contains more icons than items exist,
			 * the last icons of the list are ignored. It's not recommended to have some items without and some with icons.
			 * (optional, default: no icons)
			 * @param index The index of the option being selected by default (optional, default: the first one)
			 * @param under_cursor If this option is enabled, the dialog is centered under the cursor's position using PositionAt().
			 * (optional, default: off)
			 *
			 * The size of the dialog depends on the count of items.
			 *
			 * @return The index of the selected options in the list or -1 if the dialog was rejected.
			 *
			 * @sa InputItem()
			 * @sa InputText()
			 */
			static int InputItemIndex(QWidget* parent, const QString& title, const QStringList& items,
			                          const QList<QIcon>& icons = QList<QIcon>(), int index = 0, bool under_cursor = false);
			
			/**
			 * @brief Queries one item in a list using a prefabricated QuickDialog with a list widget as content widget.
			 *
			 * @param parent The parent window
			 * @param title The title bar's title
			 * @param items The options which can be selected
			 * @param icons A list of QIcons which are displayed in front of the items. If this list contains more icons than items,
			 * the last icons of the list are ignored, if the list contains less icons than items, the last items get no icons.
			 * It's not recommended to have some items without and some with icons.
			 * (optional, default: no icons)
			 * @param index The index of the option being selected by default (optional, default: the first one)
			 * @param under_cursor If this option is enabled, the dialog is centered under the cursor's position using PositionAt().
			 * (optional, default: off)
			 *
			 * The size of the dialog depends on the count of items.
			 *
			 * @return The text of the selected item or a null string if the dialog was rejected.
			 * @sa InputItemIndex()
			 * @sa InputText()
			 */
			static inline QString InputItem(QWidget* parent, const QString& title, const QStringList& items,
			                                const QList<QIcon>& icons = QList<QIcon>(), int index = 0, bool under_cursor = false) {
				const int result = QuickDialog::InputItemIndex(parent, title, items, icons, index, under_cursor);
				return ((result != -1) ? items[result] : QString());
			}
			
			/**
			 * @brief Centers the dialog at a global point.
			 *
			 * This is useful if you want to have the dialog under the cursor.
			 * The dialog never leaves the parent's rectangle.
			 *
			 * @param point The global point to be used as center of the dialog
			 */
			void PositionAt(const QPoint& point);
			
			/**
			 * @brief Returns the current title.
			 */
			QString get_title() const;
			
			/**
			 * @brief Sets the title bar's title. This is usually a hint for what should be queried
			 * and not the application's name (e.g. "New File").
			 */
			void set_title(const QString& title);
			
			/**
			 * @brief Returns the dialog's content widget
			 *
			 * This method also works when the dialog is already closed.
			 *
			 * @return A pointer to the content widget or nullptr if none is set.
			 *
			 * @sa set_content_widget()
			 */
			inline QWidget* get_content_widget() const { return content_widget; }
			
			/**
			 * @brief Sets the dialog's content widget.
			 *
			 * @param content_widget is the widget to be set. Consider to set the quick dialog as its parent.
			 *
			 * @sa get_content_widget()
			 */
			void set_content_widget(QWidget* content_widget);
		
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			void keyPressEvent(QKeyEvent* event) override;
		
		private:
			Ui::QuickDialog* const ui;
			
			QWidget* content_widget;
	};
}

#endif  // NOVA_FRAMEWORK_QUICKDIALOG_H
