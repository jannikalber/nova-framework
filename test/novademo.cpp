/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include <Qt>
#include <QThread>
#include <QString>
#include <QVariant>
#include <QKeySequence>
#include <QSettings>
#include <QList>
#include <QIcon>
#include <QStyle>
#include <QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QGraphicsView>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>

#include <workbench.h>
#include <contentpage.h>
#include <toolwindow.h>
#include <settings.h>
#include <quickdialog.h>
#include <actionprovider.h>
#include <progress.h>
#include <notification.h>

#define NOVA_QT_STD_ICON(name) QApplication::style()->standardIcon(name)

QSettings settings("these are", "test settings");

class TestToolWindow : public nova::ToolWindow {
	public:
		inline explicit TestToolWindow(nova::Workbench* window):
				nova::ToolWindow("My Tool Window", Qt::Vertical, window, true, Qt::LeftDockWidgetArea) {
			set_content_widget(new QTextEdit(this));
			
			QAction* action1 = ConstructAction("Tool Window 1");
			QAction* action2 = ConstructAction("Tool Window 2");
			QAction* action3 = ConstructAction("Tool Window 3");
			action1->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DirClosedIcon));
			action2->setIcon(NOVA_QT_STD_ICON(QStyle::SP_ComputerIcon));
			action3->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DialogSaveButton));
			
			connect(action1, &QAction::triggered, static_cast<QTextEdit*>(get_content_widget()), &QTextEdit::clear);
			
			auto* group = new nova::ActionGroup();
			group->AddAction(action1);
			group->AddAction(action2, true);
			
			ShowActionGroup(group);
			ShowAction(action3, true);
		}
};

class TestSettingsPage : public nova::SettingsPage {
	public:
		inline explicit TestSettingsPage(nova::Workbench* window):
				nova::SettingsPage("My Settings Page", window), edit_1(nullptr), edit_2(nullptr) {
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

template <class T>
class TestContentPage : public nova::ContentPage {
	public:
		inline explicit TestContentPage(nova::Workbench* window):
				nova::ContentPage("New Page", NOVA_QT_STD_ICON(QStyle::SP_FileIcon), window, true) {
			set_content_widget(new T(this));
			Initialize();
		}
		
		inline void Initialize() {
			auto* action_test_page = ConstructAction("Alternative Page");
			action_test_page->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DriveHDIcon));
			ShowAction(action_test_page, true);
		}
		
	protected:
		inline bool CanClose() override {
			return (QMessageBox::question(this, QApplication::applicationDisplayName(), "Close?") == QMessageBox::Yes);
		}
};

template <>
void TestContentPage<QGraphicsView>::Initialize() {
	set_title("Untitled");
	auto* action_test_page = ConstructAction("Content Page 1");
	action_test_page->setShortcut(QKeySequence("F1"));
	action_test_page->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DirIcon));
	connect(action_test_page, &QAction::triggered, [this]() {
		static bool saved = false;
		if (!saved) {
			set_title("Test Page");
			set_suffix("*");
			saved = true;
		} else set_suffix(QString());
	});
	ShowAction(action_test_page, true);
	
	auto* action_test_page_2 = ConstructAction("Content Page 2");
	action_test_page_2->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DriveDVDIcon));
	action_test_page_2->setCheckable(true);
	ShowAction(action_test_page_2, true);
}

class Workbench : public nova::Workbench {
	public:
		inline Workbench():
				nova::Workbench() {
			RegisterToolWindow<TestToolWindow>();
			RegisterSettingsPage<TestSettingsPage>();
			OpenContentPage(new TestContentPage<QGraphicsView>(this));
			
			// Status bar
			AddStatusBarWidget(new QLabel("Label 1", this), 2);
			AddStatusBarWidget(new QLabel("Label 2", this));
			
			ConstructSystemTrayIcon();
			
			// Menu demo
			nova::MenuActionProvider* menu_file = ConstructMenu(Workbench::Menu_File, true);
			
			auto* action_new = menu_file->ConstructAction("&New Page");
			action_new->setIcon(NOVA_QT_STD_ICON(QStyle::SP_FileIcon));
			connect(action_new, &QAction::triggered, [this]() { OpenContentPage(new TestContentPage<QTextEdit>(this)); });
			menu_file->ShowAction(action_new, true);
			
			QAction* check_action = get_standard_menu(Workbench::Menu_File)->ConstructAction("&Checkable Action");
			check_action->setCheckable(true);
			check_action->setWhatsThis("What's This?");
			menu_file->ShowAction(check_action, true);
			
			auto* close_group = new nova::ActionGroup();
			close_group->AddAction(ConstructStandardAction(Workbench::Action_Close, menu_file));
			close_group->AddAction(ConstructStandardAction(Workbench::Action_CloseGroup, menu_file));
			close_group->AddAction(ConstructStandardAction(Workbench::Action_CloseAll, menu_file));
			close_group->AddAction(ConstructStandardAction(Workbench::Action_CloseOthers, menu_file));
			close_group->AddAction(ConstructStandardAction(Workbench::Action_CloseTabsLeft, menu_file));
			close_group->AddAction(ConstructStandardAction(Workbench::Action_CloseTabsRight, menu_file));
			menu_file->ShowActionGroup(close_group);
			
			auto* split_group = new nova::ActionGroup();
			split_group->AddAction(ConstructStandardAction(Workbench::Action_SplitRight, menu_file));
			split_group->AddAction(ConstructStandardAction(Workbench::Action_SplitDown, menu_file));
			menu_file->ShowActionGroup(split_group);
			
			QAction* action_exit = ConstructStandardAction(Workbench::Action_Exit, menu_file);
			menu_file->ShowAction(action_exit);
			
			QAction* action_tray = get_system_tray_menu()->ConstructAction("&Test");
			action_tray->setEnabled(false);
			get_system_tray_menu()->ShowAction(action_tray);
			get_system_tray_menu()->ShowAction(action_exit, true);
			
			nova::MenuActionProvider* menu_edit = ConstructMenu(Workbench::Menu_Edit, true);
			
			// QuickDialog / Notification demo 1
			QAction* edit_action = menu_edit->ConstructAction("&Edit Demo");
			edit_action->setIcon(NOVA_QT_STD_ICON(QStyle::SP_DriveCDIcon));
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
			
			QAction* settings_action = ConstructStandardAction(Workbench::Action_Settings, menu_edit);
			menu_edit->ShowAction(settings_action, true);
			
			ConstructMenu(Workbench::Menu_Window);
			get_standard_menu(Workbench::Menu_Window)->ShowAction(
					ConstructStandardAction(Workbench::Action_RestoreLayout, get_standard_menu(Workbench::Menu_Window)));
			
			ConstructMenu(Workbench::Menu_Help);
			nova::MenuActionProvider* menu_help = get_standard_menu(Workbench::Menu_Help);
			
			auto* group_help = new nova::ActionGroup();
			QAction* search_bar_action = ConstructStandardAction(Workbench::Action_SearchBar, menu_help);
			
			group_help->AddAction(search_bar_action);
			group_help->AddAction(ConstructStandardAction(Workbench::Action_DirectHelp, menu_help));
			menu_help->ShowActionGroup(group_help);
			
			nova::MenuActionProvider* sub_menu = menu_help->ConstructMenu(this, "&Sub Menu");
			menu_help->ShowMenu(sub_menu);
			sub_menu->ShowAction(sub_menu->ConstructAction("Sub Menu &Action"));
			
			RegisterActionProvider(sub_menu);
			
			// QuickDialog demo 2
			QAction* help_action = menu_help->ConstructAction("&Help Demo");
			connect(help_action, &QAction::triggered, [this]() {
				const QList<QIcon>& icons = {NOVA_QT_STD_ICON(QStyle::SP_MediaVolume),
				                             NOVA_QT_STD_ICON(QStyle::SP_DirHomeIcon),
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
			menu_help->ShowAction(help_action);
			
			// Actions can also be added to groups when the group is already shown (useful for plugins)
			group_help->AddAction(menu_help->ConstructAction("&Plugin Action"));
			
			QList<QAction*> welcome_actions;
			welcome_actions << search_bar_action;
			welcome_actions << settings_action;
			set_welcome_actions(welcome_actions);
			
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
	QApplication::setApplicationDisplayName("Nova Framework");
	QApplication::setWindowIcon(NOVA_QT_STD_ICON(QStyle::SP_MediaPlay));
	
	auto* workbench = new Workbench();
	workbench->show();
	
	const int exit_code = QApplication::exec();
	
	delete workbench;
	return exit_code;
}
