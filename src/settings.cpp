/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "settings.h"

#include <QtCore/Qt>
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QList>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>

#include "workbench.h"
#include "ui_settingsdialog.h"

namespace nova {
	SettingsDialog::SettingsDialog(Workbench* window) : QDialog(window), ui(new Ui::SettingsDialog()),
	                                                    window(window), pages(window->settings_pages) {
		ui->setupUi(this);
		
		ui->lneFilter->setPlaceholderText(QApplication::translate("nova/settings", "Filter"));
		ui->lneFilter->setToolTip(QApplication::translate("nova/settings", "<b>Note:</b> Wildcard syntax available"));
		ui->btbButtonBox->button(QDialogButtonBox::RestoreDefaults)->setWhatsThis(
				QApplication::translate("nova/settings",
				                        "Resets the application to default settings. All changes will be lost. "
				                        "This step cannot be undone."));
		ui->lblRestartInfo->setText(QApplication::translate("nova/settings",
		                                                    "<b>Note:</b> Some changes require a restart of the application to take full effect."));
		
		// Add all pages
		for (SettingsPage* i : pages) {
			auto* content_widget = new QWidget(this);
			i->CreateUi(content_widget);
			
			ui->lswNavigation->addItem(i->get_title());
			ui->stwPages->addWidget(content_widget);
			
			i->LoadSettings();
		}
		
		ui->splNavigationPages->setStretchFactor(0, 1);
		ui->splNavigationPages->setStretchFactor(1, 2);
		
		ui->lswNavigation->setCurrentRow(0);
		
		connect(ui->lneFilter, &QLineEdit::textChanged, this, &SettingsDialog::lneFilterTextChanged);
		
		connect(this, &QDialog::accepted, this, &SettingsDialog::apply);
		connect(ui->btbButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
		        this, &SettingsDialog::restoreDefaults);
		connect(ui->btbButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
		        this, &SettingsDialog::apply);
	}
	
	void SettingsDialog::OpenSettingsPage(SettingsPage* page) {
		const int index = pages.indexOf(page);
		ui->lswNavigation->setCurrentRow(index);
	}
	
	void SettingsDialog::apply() {
		for (SettingsPage* i : pages) {
			i->Apply();
			i->RecreateActions(window);
		}
	}
	
	void SettingsDialog::restoreDefaults() {
		QMessageBox msg_box(this);
		
		msg_box.setIcon(QMessageBox::Warning);
		msg_box.setText(QApplication::translate("nova/settings",
		                                        "Do you really want to reset the application to default settings? "
		                                        "All changes will be lost. This step cannot be undone."));
		msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		
		if (msg_box.exec() == QMessageBox::Yes) {
			// Reset and reload settings
			for (SettingsPage* i : pages) {
				i->RestoreDefaults();
				i->LoadSettings();
			}
		}
	}
	
	void SettingsDialog::lneFilterTextChanged(const QString& query) {
		const QRegExp reg_exp(query, Qt::CaseInsensitive, QRegExp::WildcardUnix);
		
		// Undo previous filters
		for (int i = 0; i < ui->lswNavigation->count(); ++i) {
			ui->lswNavigation->item(i)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // Normal item
		}
		
		if (query.isEmpty()) {
			ui->lswNavigation->setCurrentRow(0);
			ui->lblMatches->setVisible(false);
		} else {
			int matches_sum = 0;
			
			for (int i = 0; i < ui->stwPages->count(); ++i) {
				const QWidget* page = ui->stwPages->widget(i);
				int matches = 0;
				
				for (const QWidget* j : page->findChildren<QWidget*>()) {
					const QVariant& property = j->property("nova/setting");
					if (property.isValid()) {
						if (property.toString().contains(reg_exp)) ++matches;
					}
				}
				
				// Match also by page's name
				if (ui->lswNavigation->item(i)->text().contains(reg_exp)) ++matches;
				
				matches_sum += matches;
				
				// No matches in this page
				if (matches == 0) ui->lswNavigation->item(i)->setFlags(Qt::NoItemFlags);  // Disabled
			}
			
			ui->lblMatches->setText(QApplication::translate("nova/settings", "Filtered: %1 match(es)")
					                        .arg(matches_sum));
			ui->lblMatches->setVisible(true);
		}
	}
	
	SettingsPage::SettingsPage(QObject* parent, const QString& title)
			: QObject(parent), TempActionProvider(QApplication::translate("nova/settings",
			                                                              "Settings > ") + title),
			  title(title), virtual_ui(nullptr) {}
	
	SettingsPage::~SettingsPage() noexcept {
		delete virtual_ui;
	}
	
	void SettingsPage::RecreateActions(void* creation_parameter) {
		auto* window = reinterpret_cast<Workbench*>(creation_parameter);
		ClearActions();
		
		// Create all search bar actions
		delete virtual_ui;  // Delete old one
		virtual_ui = new QWidget();
		CreateUi(virtual_ui);
		LoadSettings();
		
		const QList<QWidget*>& settings_widgets = virtual_ui->findChildren<QWidget*>();
		for (QWidget* i : settings_widgets) {
			const QVariant& property = i->property("nova/setting");
			if (property.isValid()) {
				QAction* action = ConstructAction(property.toString());
				
				auto* check_box = dynamic_cast<QCheckBox*>(i);
				if (check_box != nullptr) {
					// Bool setting
					action->setCheckable(true);
					action->setChecked(check_box->isChecked());
					
					connect(action, &QAction::toggled, [check_box, this](bool toggled) {
						check_box->setChecked(toggled);
						Apply();
					});
				} else {
					connect(action, &QAction::triggered, [this, window]() {
						window->OpenSettings(this);
					});
				}
			}
		}
		
		// Create a page action
		connect(ConstructAction(title), &QAction::triggered, [this, window]() {
			window->OpenSettings(this);
		});
	}
}
