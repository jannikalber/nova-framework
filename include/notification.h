/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_NOTIFICATION_H
#define NOVA_FRAMEWORK_NOTIFICATION_H

#include <functional>

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QMap>

#include "nova.h"

QT_BEGIN_NAMESPACE
QT_USE_NAMESPACE
class QIcon;
QT_END_NAMESPACE

namespace nova {
	class Notifier;
	class Notification;
	
	/**
	 * @brief An action list is a Notification's map (name; lambda) containing every action's name and the
	 * lambda to be run when the action is triggered.
	 *
	 * The typedef is translated to QMap<QString, std::function<void(Notification*)>>.
	 *
	 * The lambdas have the notification itself as argument.
	 *
	 * @sa Notification
	 */
	typedef QMap<QString, std::function<void(Notification*)>> ActionList;
	
	/**
	 * @brief A Notification consists of a message and a title and is used to inform the user about the application's state.
	 * @headerfile notification.h <nova/notification.h>
	 *
	 * Notifications are shown by Notifier objects.
	 *
	 * A notification can contain some actions which allows the user to react directly.
	 *
	 * All translations belong to the context "nova/notification".
	 */
	class NOVA_API Notification {
		public:
			/**
			 * @brief A list of all notification types.
			 *
			 * There are three different types:
			 * - Information (e.g. the task succeeded)
			 * - Warning (e.g. executing the task may end in undefined behaviour)
			 * - Error (e.g. the task failed with exit code XXX)
			 */
			enum NotificationType {
				Information, Warning, Error
			};
			
			/**
			 * Constructs a new notification.
			 *
			 * Every notification has a "Close" action which mustn't be manually added.
			 *
			 * Note: The notification is not shown until Show() is called. Make sure that the object's life time is long
			 * enough. The notification is automatically deleted when closing.
			 *
			 * @param notifier The Notifier displaying the notification
			 * @param title The notification's title
			 * @param message The (detailed) description of the notification
			 * @param type The notification's type (i.e. Information, Warning, Error) (optional, default: Information)
			 * @param high_priority The notifier will draw attention to notifications having this flag set (optional,
			 * default: false)
			 * @param actions A nova::ActionList with the notification's actions. Note: The "Close" action is automatically added.
			 * (optional, default: just "Close")
			 */
			Notification(Notifier* notifier, const QString& title, const QString& message,
			             NotificationType type = Information, bool high_priority = false,
			             const ActionList& actions = ActionList());
			
			/**
			 * @brief Creates a QIcon for the given Notification::NotificationType.
			 *
			 * These icons are also used by QMessageBox and are platform dependent.
			 *
			 * @param type Which icon should be created
			 * @return The icon being created
			 */
			static QIcon ConvertToIcon(NotificationType type);
			
			/**
			 * @brief Returns a HTML string with the notification's actions as anchors.
			 *
			 * The action's name is used for both: the anchor's text and its reference.
			 *
			 * Example: **[Action 1 | Action 2]**:
			 * @code
			 * [<a href="Action 1">Action 1</a> | <a href="Action 2">Action 2</a>]
			 * @endcode
			 *
			 * Note: ActivateAction() is not called automatically.
			 *
			 * @return The label text being created
			 *
			 * @sa ActivateAction()
			 */
			QString CreateLinksLabelText();
			
			/**
			 * @brief Triggers one of the notification's actions.
			 *
			 * Triggering an action means to run the lambda associated (nova::ActionList) with it.
			 *
			 * @param action The action's ID to be activated (the anchor's reference)
			 *
			 * @sa CreateLinksLabelText()
			 */
			void ActivateAction(const QString& action);
			
			/**
			 * @brief Enables the action, adds it to the Notifier's view and shows a popup.
			 *
			 * The notifier's current notification gets updated and the old one is closed.
			 *
			 * The notification is shown until Close() is called.
			 *
			 * @sa Close()
			 * @sa Notifier::get_current_notification()
			 */
			void Show();
			
			/**
			 * @brief Closes the notification and removes it from the Notifier's view.
			 *
			 * The Notifier's current notification gets also updated to nullptr.
			 *
			 * The notifications is automatically deleted when calling Close().
			 *
			 * @sa Show()
			 * @sa Notifier::get_current_notification()
			 */
			void Close();
			
			/**
			 * @brief Returns the notification's title.
			 */
			QString get_title() const { return title; }
			
			/**
			 * @brief Returns the notification's message.
			 */
			QString get_message() const { return message; }
			
			/**
			 * @brief Returns the notification's type.
			 *
			 * @sa Notification::NotificationType
			 */
			NotificationType get_type() const { return type; }
			
			/**
			 * @brief Returns whether the notification has high priority and the Notifier draws extra attention.
			 */
			bool is_high_priority() const { return high_priority; }
		
		private:
			friend class Notifier;
			
			Notifier* notifier;
			const QString title;
			const QString message;
			const NotificationType type;
			const bool high_priority;
			
			QMap<QString, std::function<void(Notification*)>> actions;
	};
	
	/**
	 * @brief A notifier is an abstract class which can display Notification objects.
	 * @headerfile notification.h <nova/notification.h>
	 *
	 * Notification are displayed twice:
	 * - As a popup immediately after its trigger event
	 * - In a separate view until the notification is closed
	 *
	 * Both types are implemented in this class.
	 *
	 * This class must be derived.
	 *
	 * @sa Notification
	 */
	class NOVA_API Notifier {
		public:
			/**
			 * Constructs a new notifier.
			 */
			inline Notifier() : current_notification(nullptr) {
			}
			virtual ~Notifier() noexcept = default;
			
			/**
			 * @brief Shows a notification without creating a Notification object.
			 *
			 * @param title The notification's title
			 * @param message The (detailed) description of the notification
			 * @param type The notification's type (i.e. Information, Warning, Error) (optional, default: Information)
			 *
			 * @sa Notification::Notification()
			 */
			inline void ShowNotification(const QString& title, const QString& message,
			                             Notification::NotificationType type = Notification::Information) {
				auto* notification = new Notification(this, title, message, type);
				notification->Show();
			}
			
			/**
			 * @brief Returns a pointer to the current Notification being displayed.
			 */
			inline Notification* get_current_notification() const { return current_notification; }
		
		protected:
			/**
			 * @brief Triggers an action of the current notification.
			 *
			 * @param action The action's ID to be activated
			 *
			 * @sa Notification::ActivateAction()
			 * @sa get_current_notification()
			 */
			void ActivateNotificationAction(const QString& action);
			
			/**
			 * @brief This method is called when the view must be updated.
			 *
			 * @param is_active is false if there's no notification to be displayed (i.e. the view has to be cleared)
			 * @param notification The notification to be displayed
			 */
			virtual void UpdateNotificationView(bool is_active, Notification* notification) = 0;
			
			/**
			 * @brief This method is called when a popup has to been shown.
			 *
			 * This only happens once per notification.
			 *
			 * @param notification The notification for which the popup is shown
			 */
			virtual void ShowNotificationPopup(Notification* notification) = 0;
		
		private:
			friend class Notification;
			
			Notification* current_notification;
			
			void Enable(Notification* notification);
			void Disable(Notification* notification);
	};
}

#endif  // NOVA_FRAMEWORK_NOTIFICATION_H
