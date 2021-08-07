/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <QtCore/Qt>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

#include <workbench.h>
#include <toolwindow.h>
#include <settings.h>
#include <quickdialog.h>
#include <actionprovider.h>
#include <progress.h>
#include <notification.h>

QSettings settings("this is a", "test storage");

class TestToolWindow : public nova::ToolWindow {
	public:
		inline explicit TestToolWindow(QWidget* parent) : nova::ToolWindow(parent, "My Tool Window",
		                                                                   Qt::Vertical, true,
		                                                                   Qt::LeftDockWidgetArea) {
			set_content_widget(new QTextEdit(this));
			
			QAction* action1 = ConstructAction("Tool Window 1");
			QAction* action2 = ConstructAction("Tool Window 2");
			action1->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
			action2->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
			ShowAction(action1);
			ShowAction(action2, true);
		}
};

class TestSettingsPage : public nova::SettingsPage {
	public:
		inline explicit TestSettingsPage(QObject* parent) : nova::SettingsPage(parent, "My Settings Page"),
		                                                    edit_1(nullptr), edit_2(nullptr) {}
	
	protected:
		inline void CreateUi(QWidget* start_widget) override {
			auto* layout = new QVBoxLayout(start_widget);
			
			edit_1 = new QLineEdit(start_widget);
			edit_1->setPlaceholderText("String setting");
			edit_1->setProperty("nova/setting", "String setting");
			
			edit_2 = new QCheckBox(start_widget);
			edit_2->setText("Bool setting");
			edit_2->setProperty("nova/setting", "Bool setting");
			
			layout->addWidget(edit_1);
			layout->addWidget(edit_2);
		}
		
		inline void LoadSettings() override {
			edit_1->setText(settings.value("edit_1").toString());
			edit_2->setChecked(settings.value("edit_2").toBool());
		}
		
		inline void Apply() override {
			settings.setValue("edit_1", edit_1->text());
			settings.setValue("edit_2", edit_2->isChecked());
		}
		
		inline void RestoreDefaults() override {
			settings.setValue("edit_1", "My string");
			settings.setValue("edit_2", true);
		}
	
	private:
		QLineEdit* edit_1;
		QCheckBox* edit_2;
};

class Workbench : public nova::Workbench {
	public:
		inline Workbench() : nova::Workbench() {
			RegisterToolWindow<TestToolWindow>();
			RegisterSettingsPage<TestSettingsPage>();
			
			// Status bar
			AddStatusBarWidget(new QLabel("Label 1", this), 2);
			AddStatusBarWidget(new QLabel("Label 2", this));
			
			ConstructSystemTrayIcon();
			
			// Menu demo
			nova::MenuActionProvider* menu_file = ConstructMenu(Workbench::Menu_File, true);
			
			QAction* check_action = get_standard_menu(Workbench::Menu_File)->ConstructAction("&Checkable Action");
			check_action->setCheckable(true);
			check_action->setWhatsThis("What's This?");
			menu_file->ShowAction(check_action, false, true);
			
			QAction* action_exit = ConstructStandardAction(Workbench::Action_Exit, menu_file);
			menu_file->ShowAction(action_exit, true);
			
			get_system_tray_menu()->ShowAction(get_system_tray_menu()->ConstructAction("&Test"));
			get_system_tray_menu()->ShowAction(action_exit, true);
			
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Menu_Edit, true);
			
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
			menu_edit->ShowAction(edit_action, false, true);
			menu_edit->ShowAction(check_action, true);
			
			menu_edit->ShowAction(ConstructStandardAction(Workbench::Action_Settings, menu_edit), true);
			
			ConstructMenu(Workbench::Menu_Window);
			get_standard_menu(Workbench::Menu_Window)->ShowAction(
					ConstructStandardAction(Workbench::Action_ResetLayout, get_standard_menu(Workbench::Menu_Window)));
			
			ConstructMenu(Workbench::Menu_Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Menu_Help);
			
			menu_help->ShowAction(ConstructStandardAction(Workbench::Action_SearchBar, menu_help));
			menu_help->ShowAction(ConstructStandardAction(Workbench::Action_DirectHelp, menu_help), true);
			
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
