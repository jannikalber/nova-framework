/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QMessageBox>

#include <workbench.h>
#include <actionprovider.h>
#include <quickdialog.h>

class Workbench : public nova::Workbench {
	public:
		inline Workbench() : nova::Workbench() {
			ConstructMenu(Workbench::File)->ShowAction(ConstructStandardAction(Workbench::Exit));
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Edit);
			ConstructMenu(Workbench::Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Help);
			
			// QuickDialog demo 1
			QAction* edit_action = menu_edit->ConstructAction("&Edit Demo");
			connect(edit_action, &QAction::triggered, [this]() {
				QMessageBox::information(this, "", nova::QuickDialog::InputText(this, "Show Message", "Message",
				                                                                QLineEdit::Normal, "My message"));
			});
			menu_edit->ShowAction(edit_action);
			
			menu_help->ShowAction(ConstructStandardAction(Workbench::SearchBar));
			menu_help->ShowAction(ConstructStandardAction(Workbench::DirectHelp), true);
			
			// QuickDialog demo 2
			QAction* help_action = menu_help->ConstructAction("&Help Demo");
			connect(help_action, &QAction::triggered, [this]() {
				const QList<QIcon>& icons = QList<QIcon>({QApplication::style()->standardIcon(QStyle::SP_MediaVolume),
				                                          QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon),
				                                          QIcon()});
				const QString& text = nova::QuickDialog::InputItem(this, "Help Topics",
				                                                   QStringList({"Page 1", "Page 2"}), icons);
				
				// Show the page's title in another QuickDialog
				nova::QuickDialog dialog(this, "Edit Help Topic");
				
				QTextEdit text_edit(&dialog);
				text_edit.setText(text);
				
				dialog.set_input_widget(&text_edit);
				dialog.exec();
			});
			menu_help->ShowAction(help_action);
		}
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
