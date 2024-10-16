/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_SETTINGS_H
#define NOVA_FRAMEWORK_SETTINGS_H

#include <QObject>
#include <QList>
#include <QString>
#include <QWidget>
#include <QAction>
#include <QDialog>

#include "nova.h"
#include "actionprovider.h"

class QHideEvent;

namespace Ui { class SettingsDialog; }

namespace nova {
	class Workbench;
	class SettingsPage;
}

namespace nova {
	/**
	 * @brief A dialog to manipulate the application's settings.
	 *
	 * The dialog consist of nova::SettingsPage objects and allows
	 * to filter all settings by name too.
	 *
	 * The translations belong to the context "nova/settings".
	 *
	 * @sa nova::SettingsPage
	 * @sa nova::Workbench::OpenSettings()
	 */
	class NOVA_API SettingsDialog : public QDialog {
		Q_OBJECT
		
		public:
			/**
			 * @brief Creates a new settings dialog.
			 *
			 * Use exec() to run it.
			 *
			 * @param window The workbench whose nova::SettingPage objects can be manipulated
			 * (optional, default: nova::workbench)
			 */
			explicit SettingsDialog(Workbench* window = workbench);
			NOVA_DISABLE_COPY(SettingsDialog)
			
			/**
			 * @brief Opens the requested nova::SettingsPage.
			 */
			void OpenSettingsPage(SettingsPage* page);
		
		protected:
			/**
			 * This method is internally required and should not be called.
			 */
			void hideEvent(QHideEvent* event) override;
		
		private:
			Ui::SettingsDialog* const ui;
			Workbench* const window;
			
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
	 * When having nova::SettingsDialog open, this class represents one page of the dialog. Actually, settings pages
	 * live longer, so directly manipulating settings is still possible.
	 *
	 * Your subclass must have a constructor with QObject* as parameter (the parent object).
	 * Call nova::Workbench::RegisterSettingsPage<YourSubclass>() to register the settings page class.
	 *
	 * Use set_content_widget() to create a layout with some widgets which represent the settings you want the user to change.
	 * If you define the dynamic property "nova/setting" (type: string) in your settings widget, it can be found using
	 * nova::SearchBar. Alternatively, you can define "nova/setting" just as true (type: bool). In this case Nova uses
	 * the property "text" as title (only if "text" exists). This way is recommended to avoid redundancies.
	 *
	 * The translations belong to the context "nova/settings".
	 *
	 * This class must be derived.
	 *
	 * @sa nova::SettingsDialog
	 */
	class NOVA_API SettingsPage : public QObject, public TempActionProvider {
		public:
			NOVA_DISABLE_COPY(SettingsPage)
			virtual ~SettingsPage() noexcept;
			
			/**
			 * This method is internally required and should not be called.
			 */
			void RecreateActions(const Properties& creation_parameters = Properties()) override;
			
			/**
			 * @brief This pure virtual method is called when the settings are requested
			 * to be reset, i.e. you have to apply the setting's default values.
			 *
			 * You only have to manipulate the configuration files.
			 */
			virtual void RestoreDefaults() = 0;
			
			/**
			 * @brief This method checks the configuration file.
			 *
			 * It returns false in one of the following cases:
			 * <ol>
			 *  <li>the settings aren't complete</li>
			 *  <li>they are corrupted</li>
			 * </ol>
			 *
			 * It's is a hint for the application to recreate its configuration.
			 * The method allows the application to work on computers which aren't
			 * well-set-up. Consider checking your configuration on start up by calling
			 * ValidateConfiguration() and RestoreDefaults() if necessary for each settings
			 * page.
			 *
			 * The default implementation returns true.
			 *
			 * @return if the configuration looks okay
			 */
			inline virtual bool ValidateConfiguration() { return true; }
			
			/**
			 * @brief Settings pages don't allow changeable titles.
			 *
			 * This method is internally required and should not be called.
			 */
			inline void set_title(const QString&) override {}
			
			/**
			 * This method is internally required to evaluate the correct title.
			 */
			inline QString get_title() { return title; }
			
			/**
			 * @brief Returns the page's content widget.
			 *
			 * Note: If there's no active nova::SettingsDialog, this method doesn't return nullptr.
			 * Else, a pointer to an invisible widget is returned. This is used to manipulate
			 * the settings directly (e.g. used by nova::SearchBar).
			 *
			 * @return A pointer to the page's content (or the "hidden" user interface)
			 *
			 * @sa set_content_widget()
			 */
			inline QWidget* get_content_widget() const { return content_widget; }
		
		protected:
			/**
			 * @brief Creates a new nova::SettingsPage.
			 *
			 * @param title The page's title (shown in nova::SettingsDialog)
			 * @param window The associated workbench (subclass constructors should forward this pointer)
			 *
			 * @sa nova::Workbench::RegisterSettingsPage()
			 */
			SettingsPage(const QString& title, Workbench* window);
			
			/**
			 * @brief Sets the page's content widget.
			 *
			 * The framework takes ownership of the pointer.
			 *
			 * The page must not be initialized with the current configuration.
			 * This job is done automatically by LoadSettings().
			 *
			 * This method must not be called after the settings page is registered.
			 *
			 * If you define the dynamic property "nova/setting" (type: string, the setting's name)
			 * in the children widgets, they can be found using the SearchBar. Alternatively, you can define "nova/setting"
			 * just as true (type: bool). In this case Nova uses the property "text" as title. This way is recommended
			 * to avoid redundancies.
			 *
			 * @param content_widget The new widget to be used
			 */
			void set_content_widget(QWidget* content_widget);
			
			/**
			 * @brief This pure virtual method is called when the settings have to be loaded and the widgets have to be updated.
			 *
			 * Your implementation should fill the widgets by loading the configuration files.
			 *
			 * Usually, you don't have to call this method manually.
			 */
			virtual void LoadSettings() = 0;
			
			/**
			 * @brief This pure virtual method is called when the settings have to be saved (e.g. in a configuration file).
			 *
			 * This is the "reverse" of LoadSettings().
			 *
			 * If you manually change your settings widget, you should always call this method.
			 */
			virtual void Apply() = 0;
		
		private:
			friend class SettingsDialog;
			friend class Workbench;
			
			Workbench* const workbench_window;
			
			const QString title;
			QWidget* content_widget;
			
			// Creates an action to open this page
			void ConstructNavigationAction(ActionProvider* provider, Workbench* window);
	};
}

#endif  // NOVA_FRAMEWORK_SETTINGS_H
