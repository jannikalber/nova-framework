/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "settings.h"

#include <Qt>
#include <QtGlobal>
#include <QVariant>
#include <QMetaType>
#include <QRegExp>
#include <QList>
#include <QStringList>
#include <QHideEvent>
#include <QAction>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QCompleter>
#include <QMessageBox>

#include "ui_settingsdialog.h"
#include "workbench.h"

#define NOVA_CONTEXT "nova/settings"
#define NOVA_SETTING_PROPERTY_NAME "nova/setting"

// Helper which evaluates the setting's name by considering the property "nova/setting"
static QString EvaluateSettingsName(const QWidget* widget) {
	const QVariant& property = widget->property(NOVA_SETTING_PROPERTY_NAME);
	
	switch (static_cast<QMetaType::Type>(property.type())) {
		case QMetaType::Bool:
			// Automatic "nova/setting"
			return (property.toBool() ? widget->property("text").toString() : QString());
		
		case QMetaType::QString:
			return property.toString();
		
		default:
			return QString();
	}
}

namespace nova {
	SettingsDialog::SettingsDialog(Workbench* window):
			QDialog(window), ui(new Ui::SettingsDialog()), window(window), pages(window->settings_pages) {
		ui->setupUi(this);
		
		ui->lneFilter->setPlaceholderText(NOVA_TR("Filter"));
		ui->lneFilter->setToolTip(NOVA_TR("<b>Note:</b> Wildcard syntax available"));
		ui->btbButtonBox->button(QDialogButtonBox::RestoreDefaults)->setWhatsThis(
				NOVA_TR("Resets the current page's settings to their default values. The changes have to be manually applied."));
		ui->lblRestartInfo->setText(NOVA_TR("<b>Note:</b> Some changes require a restart of the application to take full effect."));
		
		// Add the pages
		for (SettingsPage* i : pages) {
			// Add the dialog as parent
			i->content_widget->setParent(this);
			
			// Create a title label
			auto* title_widget = new QWidget(this);
			auto* title_label = new QLabel("### " + i->get_title(), title_widget);
			title_label->setTextFormat(Qt::MarkdownText);
			
			auto* layout = new QVBoxLayout(title_widget);
			layout->setContentsMargins(9, 0, 9, 0);
			layout->addWidget(title_label);
			
			ui->lswNavigation->addItem(i->get_title());
			ui->stwTitles->addWidget(title_widget);
			ui->stwPages->addWidget(i->content_widget);
			
			i->LoadSettings();
		}
		
		ui->splNavigationPages->setSizes({300, 700});
		ui->lswNavigation->setCurrentRow(0);
		
		// Completer
		QStringList list;
		for (const SettingsPage* i : pages) {
			for (const QAction* j : i->ListActions()) {
				list << j->text();
			}
		}
		
		auto* completer = new QCompleter(list, this);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		completer->setCompletionMode(QCompleter::InlineCompletion);
		completer->setMaxVisibleItems(5);
		ui->lneFilter->setCompleter(completer);
		
		connect(ui->lneFilter, &QLineEdit::textChanged, this, &SettingsDialog::lneFilterTextChanged);
		
		connect(ui->btbButtonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
		        this, &SettingsDialog::restoreDefaults);
		connect(ui->btbButtonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
		        this, &SettingsDialog::apply);
	}
	
	void SettingsDialog::OpenSettingsPage(SettingsPage* page) {
		const int index = pages.indexOf(page);
		if (index != -1) ui->lswNavigation->setCurrentRow(index);
	}
	
	void SettingsDialog::hideEvent(QHideEvent* event) {
		// hideEvent() is required, because accept() and reject() don't fire closeEvent()
		QDialog::hideEvent(event);
		
		if (event->spontaneous()) {
			event->accept();
			return;
		}
		
		// Because the virtual Ui will be soon created, the settings must be applied now (and not by signals and slot which are too late)
		if (result() == QDialog::Accepted) apply();
		
		Properties parameters;
		parameters["workbench"] = reinterpret_cast<quintptr>(window);
		for (SettingsPage* i : pages) {
			// Remove the parent
			i->content_widget->setParent(nullptr);
			i->RecreateActions(parameters);
		}
		
		event->accept();
	}
	
	void SettingsDialog::lneFilterTextChanged(const QString& query) {
		if (ui->lswNavigation->count() == 0) return;
		
		const QRegExp reg_exp(query, Qt::CaseInsensitive, QRegExp::WildcardUnix);
		
		// Undo previous filters
		for (int i = 0 ; i < ui->lswNavigation->count() ; ++i) {
			ui->lswNavigation->item(i)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);  // Normal item
		}
		
		if (query.isEmpty()) {
			ui->lswNavigation->setCurrentRow(0);
			ui->lblMatches->setVisible(false);
		} else {
			int matches_sum = 0;
			
			for (int i = 0 ; i < pages.count() ; ++i) {
				const SettingsPage* page = pages[i];
				int matches = 0;
				
				for (const QAction* j : page->ListActions()) {
					if (reg_exp.indexIn(j->toolTip()) != -1) ++matches;
				}
				
				matches_sum += matches;
				
				// No matches in this page
				if (matches == 0) ui->lswNavigation->item(i)->setFlags(Qt::NoItemFlags);  // Disabled
			}
			
			ui->lblMatches->setText(NOVA_TR("Filtered: %1 match(es)").arg(matches_sum));
			ui->lblMatches->setVisible(true);
			
			// Scroll to first item being enabled
			if (!(ui->lswNavigation->currentItem()->flags() & Qt::ItemIsEnabled)) {
				int index = 0;
				const int count = ui->lswNavigation->count();
				while ((index < count) && !(ui->lswNavigation->item(index)->flags() & Qt::ItemIsEnabled)) {
					++index;
				}
				
				if (index < count) ui->lswNavigation->setCurrentRow(index);
			}
		}
	}
	
	void SettingsDialog::apply() {
		for (SettingsPage* i : pages) {
			i->Apply();
		}
	}
	
	void SettingsDialog::restoreDefaults() {
		QMessageBox msg_box(this);
		
		msg_box.setIcon(QMessageBox::Warning);
		msg_box.setText(NOVA_TR("Do you really want to reset the application to default settings? "
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
	
	SettingsPage::SettingsPage(QObject* parent, const QString& title):
			QObject(parent), TempActionProvider(NOVA_TR("Settings > ") + title),
			title(title), content_widget(new QWidget()) {}
	
	SettingsPage::~SettingsPage() noexcept {
		delete content_widget;
	}
	
	void SettingsPage::RecreateActions(const Properties& creation_parameters) {
		if (!creation_parameters.contains("workbench")) return;
		
		auto* window = reinterpret_cast<Workbench*>(creation_parameters["workbench"].toULongLong());
		ClearActions();
		
		const QList<QWidget*>& settings_widgets = content_widget->findChildren<QWidget*>();
		for (QWidget* i : settings_widgets) {
			// Match group boxes automatically
			auto* group_box = dynamic_cast<QGroupBox*>(i);
			if (group_box != nullptr) group_box->setProperty(NOVA_SETTING_PROPERTY_NAME, group_box->title());
			
			if (i->property(NOVA_SETTING_PROPERTY_NAME).isValid()) {
				QAction* action = ConstructAction(EvaluateSettingsName(i));
				action->setWhatsThis(i->whatsThis());
				
				auto* check_box = dynamic_cast<QCheckBox*>(i);
				if (check_box != nullptr) {
					// Bool setting
					action->setEnabled(i->isEnabled());
					action->setCheckable(true);
					action->setChecked(check_box->isChecked());
					
					connect(action, &QAction::toggled, [check_box, this](bool toggled) {
						check_box->setChecked(toggled);
						Apply();
					});
				} else {
					connect(action, &QAction::triggered, [this, window, i]() {
						window->OpenSettings(this, i);
					});
				}
			}
		}
	}
	
	void SettingsPage::set_content_widget(QWidget* content_widget) {
		// Destruction required if the parent is missing
		delete this->content_widget;
		
		this->content_widget = content_widget;
		this->content_widget->setParent(nullptr); // It's important to prevent having a parent because of destruction processes
	}
	
	void SettingsPage::ConstructNavigationAction(ActionProvider* provider, Workbench* window) {
		connect(provider->ConstructAction(title), &QAction::triggered, [this, window]() {
			window->OpenSettings(this);
		});
	}
}
