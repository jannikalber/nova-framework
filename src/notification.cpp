/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "notification.h"

#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>

namespace nova {
	Notification::Notification(Notifier* notifier, const QString& title, const QString& message,
	                           NotificationType type, bool high_priority, const ActionList& actions)
			: notifier(notifier), title(title), message(message), type(type),
			  high_priority(high_priority), actions(actions) {
		this->actions[QApplication::translate("nova/notification", "Close")] = [](Notification* notification) {
			notification->Close();
		};
	}
	
	QIcon Notification::ConvertToIcon(Notification::NotificationType type) {
		QStyle::StandardPixmap pixmap;
		switch (type) {
			case Information:
				pixmap = QStyle::SP_MessageBoxInformation;
				break;
			
			case Warning:
				pixmap = QStyle::SP_MessageBoxWarning;
				break;
			
			case Error:
				pixmap = QStyle::SP_MessageBoxCritical;
		}
		
		return QApplication::style()->standardIcon(pixmap);
	}
	
	QString Notification::CreateLinksLabelText() {
		QStringList links;
		
		for (const QString& i : actions.keys()) {
			links << QString("<a href=\"%1\">%1</a>").arg(i);
		}
		
		return "[" + links.join(" | ") + "]";
	}
	
	void Notification::ActivateAction(const QString& action) {
		if (actions.contains("Close")) {
			actions[action](this);
		}
	}
	
	void Notification::Show() {
		notifier->Enable(this);
	}
	void Notification::Close() {
		notifier->Disable(this);
		delete this;
	}
	
	void Notifier::Enable(Notification* notification) {
		if (current_notification != nullptr) current_notification->Close();
		current_notification = notification;
		ShowNotificationPopup(notification);
		UpdateNotificationView(true, notification);
	}
	
	void Notifier::Disable(Notification* notification) {
		if (current_notification == notification) {
			current_notification = nullptr;
			UpdateNotificationView(false, nullptr);
		}
	}
	
	void Notifier::ActivateNotificationAction(const QString& action) {
		if (current_notification != nullptr) {
			current_notification->ActivateAction(action);
		}
	}
}
