/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>

#include <workbench.h>
#include <progress.h>
#include <notification.h>
#include <actionprovider.h>
#include <quickdialog.h>

class Workbench : public nova::Workbench {
	public:
		inline Workbench() : nova::Workbench() {
			// Status bar
			AddStatusBarWidget(new QLabel("Label 1", this), 2);
			AddStatusBarWidget(new QLabel("Label 2", this));
			
			ConstructSystemTrayIcon();
			
			// Menu demo
			nova::MenuActionProvider* menu_file = ConstructMenu(Workbench::File);
			
			QAction* check_action = get_standard_menu(Workbench::File)->ConstructAction("&Checkable Action");
			check_action->setCheckable(true);
			check_action->setWhatsThis("What's This?");
			menu_file->ShowAction(check_action);
			
			QAction* action_exit = ConstructStandardAction(Workbench::Exit, menu_file);
			menu_file->ShowAction(action_exit, true);
			
			get_system_tray_menu()->ShowAction(get_system_tray_menu()->ConstructAction("&Test"));
			get_system_tray_menu()->ShowAction(action_exit, true);
			
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Edit);
			ConstructMenu(Workbench::Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Help);
			
			// QuickDialog / Notification demo 1
			QAction* edit_action = menu_edit->ConstructAction("&Edit Demo");
			edit_action->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon));
			connect(edit_action, &QAction::triggered, [this]() {
				nova::ActionList action_list;
				action_list["Quit"] = [this](nova::Notification* notification) { close(); };
				
				const QString& message = nova::QuickDialog::InputText(this, "Enable Message",
				                                                      "Message",
				                                                      QLineEdit::Normal,
				                                                      "My message");
				
				auto* notification = new nova::Notification(this, "Notification", message,
				                                            nova::Notification::Information, false, action_list);
				notification->Show();
			});
			menu_edit->ShowAction(edit_action);
			menu_edit->ShowAction(check_action, true);
			
			menu_help->ShowAction(ConstructStandardAction(Workbench::SearchBar, menu_help));
			menu_help->ShowAction(ConstructStandardAction(Workbench::DirectHelp, menu_help), true);
			
			// QuickDialog demo 2
			QAction* help_action = menu_help->ConstructAction("&Help Demo");
			connect(help_action, &QAction::triggered, [this]() {
				const QList<QIcon>& icons = QList<QIcon>({QApplication::style()->standardIcon(QStyle::SP_MediaVolume),
				                                          QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon),
				                                          QIcon()});
				const QString& text = nova::QuickDialog::InputItem(this, "Help Topics",
				                                                   QStringList({"Page 1", "Page 2"}), icons);
				
				// Enable the page's title in another QuickDialog
				nova::QuickDialog dialog(this, "Edit Help Topic");
				
				QTextEdit text_edit(&dialog);
				text_edit.setText(text);
				
				dialog.set_input_widget(&text_edit);
				dialog.exec();
			});
			menu_help->ShowAction(help_action);
			
			// ProgressMonitor demo
			auto* task1 = new nova::Task(this, "Testing 1", true,
			                             [](nova::Task* task) -> nova::StatusCode {
				                             QThread::sleep(5);
				                             return nova::StatusCode(true, nullptr);
			                             });
			auto* task2 = new nova::Task(this, "Testing 2", false,
			                             [](nova::Task* task) -> nova::StatusCode {
				                             for (int i = 1; i <= 100; ++i) {
					                             task->set_value(i);
					                             QThread::msleep(100);
				                             }
				                             return nova::StatusCode(false, "Testing failed");
			                             });
			
			task1->start();
			task2->start();
		}
};

int main(int argc, char** argv) {
	new QApplication(argc, argv);
	QApplication::setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
	
	Workbench workbench;
	workbench.show();
	
	return QApplication::exec();
}
