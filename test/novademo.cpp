/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <Qt>
#include <QThread>
#include <QString>
#include <QVariant>
#include <QSettings>
#include <QList>
#include <QIcon>
#include <QStyle>
#include <QApplication>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>

#include <workbench.h>
#include <toolwindow.h>
#include <settings.h>
#include <quickdialog.h>
#include <actionprovider.h>
#include <progress.h>
#include <notification.h>

QSettings settings("these are", "test settings");

class TestToolWindow : public nova::ToolWindow {
	public:
		inline explicit TestToolWindow(QWidget* parent):
				nova::ToolWindow(parent, "My Tool Window", Qt::Vertical, true, Qt::LeftDockWidgetArea) {
			set_content_widget(new QTextEdit(this));
			
			QAction* action1 = ConstructAction("Tool Window 1");
			QAction* action2 = ConstructAction("Tool Window 2");
			QAction* action3 = ConstructAction("Tool Window 3");
			action1->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
			action2->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
			action3->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
			
			auto* group = new nova::ActionGroup();
			group->AddAction(action1);
			group->AddAction(action2);
			
			ShowActionGroup(group);
			ShowAction(action3);
		}
};

class TestSettingsPage : public nova::SettingsPage {
	public:
		inline explicit TestSettingsPage(QObject* parent):
				nova::SettingsPage(parent, "My Settings Page"), edit_1(nullptr), edit_2(nullptr) {
			auto* root_widget = new QWidget();
			auto* root_layout = new QVBoxLayout(root_widget);
			
			auto* group = new QGroupBox("Settings group", root_widget);
			root_layout->addWidget(group);
			auto* layout = new QVBoxLayout(group);
			
			edit_1 = new QLineEdit(root_widget);
			edit_1->setPlaceholderText("String setting");
			edit_1->setWhatsThis("Some information");
			edit_1->setProperty("nova/setting", "String setting");
			
			edit_2 = new QCheckBox(root_widget);
			edit_2->setText("Bool setting");
			edit_2->setProperty("nova/setting", true);
			
			layout->addWidget(edit_1);
			layout->addWidget(edit_2);
			
			root_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
			set_content_widget(root_widget);
		}
		
		inline void RestoreDefaults() override {
			settings.setValue("edit_1", "My string");
			settings.setValue("edit_2", true);
		}
	
	protected:
		inline void LoadSettings() override {
			edit_1->setText(settings.value("edit_1").toString());
			edit_2->setChecked(settings.value("edit_2").toBool());
		}
		
		inline void Apply() override {
			settings.setValue("edit_1", edit_1->text());
			settings.setValue("edit_2", edit_2->isChecked());
		}
	
	private:
		QLineEdit* edit_1;
		QCheckBox* edit_2;
};

class Workbench : public nova::Workbench {
	public:
		inline Workbench():
				nova::Workbench() {
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
			menu_file->ShowAction(check_action, true);
			
			QAction* action_exit = ConstructStandardAction(Workbench::Action_Exit, menu_file);
			menu_file->ShowAction(action_exit);
			
			QAction* action_tray = get_system_tray_menu()->ConstructAction("&Test");
			action_tray->setEnabled(false);
			get_system_tray_menu()->ShowAction(action_tray);
			get_system_tray_menu()->ShowAction(action_exit, true);
			
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Menu_Edit, true);
			
			// QuickDialog / Notification demo 1
			QAction* edit_action = menu_edit->ConstructAction("&Edit Demo");
			edit_action->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon));
			connect(edit_action, &QAction::triggered, [this]() {
				nova::ActionList action_list;
				action_list.insert("Quit", [this](nova::Notification* notification) { close(); });
				
				const QString& message = nova::QuickDialog::InputText(this, "Enable Message",
				                                                      "Message",
				                                                      QLineEdit::Normal,
				                                                      "My message");
				
				auto* notification = new nova::Notification(this, "Notification", message,
				                                            nova::Notification::Information, false, action_list);
				notification->Show();
			});
			menu_edit->ShowAction(edit_action, true);
			menu_edit->ShowAction(check_action);
			
			menu_edit->ShowAction(ConstructStandardAction(Workbench::Action_Settings, menu_edit), true);
			
			ConstructMenu(Workbench::Menu_Window);
			get_standard_menu(Workbench::Menu_Window)->ShowAction(
					ConstructStandardAction(Workbench::Action_RestoreLayout, get_standard_menu(Workbench::Menu_Window)));
			
			ConstructMenu(Workbench::Menu_Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Menu_Help);
			
			auto* group_help = new nova::ActionGroup();
			group_help->AddAction(ConstructStandardAction(Workbench::Action_SearchBar, menu_help));
			group_help->AddAction(ConstructStandardAction(Workbench::Action_DirectHelp, menu_help));
			menu_help->ShowActionGroup(group_help);
			
			// QuickDialog demo 2
			nova::MenuActionProvider* sub_menu = menu_help->ConstructSubMenu("Demos", this);
			
			QAction* help_action = sub_menu->ConstructAction("&Help Demo");
			connect(help_action, &QAction::triggered, [this]() {
				const QList<QIcon>& icons = {QApplication::style()->standardIcon(QStyle::SP_MediaVolume),
				                             QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon),
				                             QIcon()};
				const QString& text = nova::QuickDialog::InputItem(this, "Help Topics",
				                                                   QStringList({"Page 1", "Page 2"}), icons);
				
				// Enable the page's title in another QuickDialog
				nova::QuickDialog dialog(this, "Edit Help Topic");
				
				QTextEdit text_edit(&dialog);
				text_edit.setText(text);
				
				dialog.set_content_widget(&text_edit);
				dialog.exec();
			});
			
			menu_help->ShowMenu(sub_menu);
			sub_menu->ShowAction(help_action);
			
			// Actions can also be added to groups when the group is already shown (useful for plugins)
			group_help->AddAction(menu_help->ConstructAction("Plugin Action"));
			
			// ProgressMonitor demo
			auto* task1 = new nova::Task(this, "Testing 1", true,
			                             [](nova::Task* task) -> nova::TaskResult {
				                             QThread::sleep(5);
				                             return nova::TaskResult(true, nullptr);
			                             });
			auto* task2 = new nova::Task(this, "Testing 2", false,
			                             [](nova::Task* task) -> nova::TaskResult {
				                             for (int i = 1 ; i <= 100 ; ++i) {
					                             task->set_value(i);
					                             QThread::msleep(100);
				                             }
				                             return nova::TaskResult(false, "Testing failed");
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
