/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_SETTINGS_H
#define NOVA_FRAMEWORK_SETTINGS_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtWidgets/QAction>
#include <QtWidgets/QDialog>

#include "nova.h"
#include "actionprovider.h"

QT_BEGIN_NAMESPACE
QT_USE_NAMESPACE
class QWidget;

namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

namespace nova {
	class Workbench;
	class SettingsPage;
	
	/**
	 * @brief A dialog which is used to manipulate the application's settings.
	 *
	 * The dialog consist of SettingsPage objects.
	 *
	 * The dialog allows also to filter all settings by name.
	 *
	 * Its implementation is complete, you don't have to do extra work.
	 *
	 * The translations belong to the context "nova/settings".
	 *
	 * @sa SettingsPage
	 * @sa Workbench::OpenSettings()
	 */
	class NOVA_API SettingsDialog : public QDialog {
		Q_OBJECT
		
		public:
			/**
			 * Creates a new settings dialog.
			 *
			 * Use exec() to run it.
			 *
			 * @param window The workbench whose SettingPage objects can be manipulated (optional, default: nova::workbench)
			 */
			explicit SettingsDialog(Workbench* window = workbench);
			
			/**
			 * @brief Opens the requested SettingsPage.
			 *
			 * This method also works if the dialog is hidden.
			 *
			 * @param page The page to be opened
			 */
			void OpenSettingsPage(SettingsPage* page);
		
		private:
			Ui::SettingsDialog* ui;
			Workbench* window;
			
			QList<SettingsPage*> pages;
		
		private slots:
			void lneFilterTextChanged(const QString& query);
			
			void apply();
			void restoreDefaults();
	};
	
	/**
	 * @brief Represents one category/context of the application's settings.
	 * @headerfile settings.h <nova/settings.h>
	 *
	 * When having the SettingsDialog open, this class is one page of the dialog. SettingsPage objects
	 * live actually longer, so directly manipulating settings is still possible.
	 *
	 * Therefore, the class provides a "hidden" user interfaces which is used in this case.
	 *
	 * The translations belong to the context "nova/settings".
	 *
	 * This class should be derived.
	 *
	 * @sa SettingsDialog
	 */
	class NOVA_API SettingsPage : public QObject, public TempActionProvider {
		public:
			/**
			 * Creates a new SettingsPage.
			 *
			 * Because such objects also live if there's no active SettingsDialog, the user interfaces
			 * must be dynamically created (see CreateUi()).
			 *
			 * Your subclass must have a constructor with QObject* as parameter (the parent object).
			 * Call Workbench::RegisterSettingsPage<YourSubclass>() to register the settings page class.
			 *
			 * @param parent The QObject's parent
			 * @param title The page's title (shown in SettingsDialog)
			 * @sa Workbench::RegisterSettingsPage()
			 */
			SettingsPage(QObject* parent, const QString& title);
			virtual ~SettingsPage() noexcept;
			
			/**
			 * Reimplements TempActionProvider::RecreateActions()
			 *
			 * Don't call this method. This can result in undefined behaviour.
			 */
			void RecreateActions(void* creation_parameter) override;
			
			/**
			 * @brief Returns the page's title.
			 */
			inline QString get_title() const { return title; }
		
		protected:
			/**
			 * @brief Creates the page's user interface.
			 *
			 * Note that this method will be called more than once per object.
			 *
			 * This method is pure virtual and must be implemented.
			 *
			 * @param start_widget The root widget in which the interface should be inserted
			 */
			virtual void CreateUi(QWidget* start_widget) = 0;
			
			/**
			 * @brief This method is called when the settings have to be loaded and the widgets have to be updated.
			 *
			 * This method is pure virtual and must be implemented.
			 */
			virtual void LoadSettings() = 0;
			
			/**
			 * @brief This method is called when the settings have to be saved (e.g. in a configuration file).
			 *
			 * This is the "reverse" of LoadSettings().
			 *
			 * This method is pure virtual and must be implemented.
			 */
			virtual void Apply() = 0;
			
			/**
			 * @brief This method is called when the settings (just the values; e.g. in a configuration file) are requested
			 * to be reset, i.e. you have to apply the setting's default values.
			 *
			 * This method is pure virtual and must be implemented.
			 */
			virtual void RestoreDefaults() = 0;
		
		private:
			friend class SettingsDialog;
			
			const QString title;
			// For browsing the settings without dialog
			QWidget* virtual_ui;
	};
}

#endif  // NOVA_FRAMEWORK_SETTINGS_H
