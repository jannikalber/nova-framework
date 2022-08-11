/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_NOTIFICATION_H
#define NOVA_FRAMEWORK_NOTIFICATION_H

#include <functional>

#include <QString>
#include <QMap>

#include "nova.h"

class QIcon;

namespace nova {
	class Notifier;
	class Notification;
}

namespace nova {
	/**
	 * @brief nova::ActionList specifies nova::Notification's actions. It is a map (name; lambda) containing every
	 * action's name and the lambda to be run when the action is triggered.
	 *
	 * The typedef is translated to QMap<QString, std::function<void(nova::Notification* triggered_notification)>>.
	 *
	 * The function argument 'notification' is the notification itself.
	 *
	 * @sa nova::Notification
	 */
	typedef QMap<QString, std::function<void(Notification* notification)>> ActionList;
	
	/**
	 * @brief A notification consists of a message and a title and is used to inform the user about the application's state.
	 * @headerfile notification.h <nova/notification.h>
	 *
	 * Notifications are shown by nova::Notifier objects.
	 *
	 * All translations belong to the context "nova/notification".
	 */
	class NOVA_API Notification {
		public:
			/**
			 * @brief A list of all notification types.
			 */
			enum NotificationType {
				//! Information type (e.g. a task succeeded)
				Information,
				//! Warning type (e.g. continuing could cause further issues)
				Warning,
				//! Error type (e.g. the task failed with exit code XXX)
				Error
			};
			
			/**
			 * @brief Creates a new nova::Notification.
			 *
			 * Every notification has a "Close" action which is added automatically.
			 *
			 * Note: The notification is not shown until Show() is called. Make sure that the notification's life time is long
			 * enough. It gets automatically deleted when closing.
			 *
			 * @param notifier The nova::Notifier which will display the notification
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
			 * @brief Creates a QIcon for the given nova::Notification::NotificationType.
			 *
			 * The icons are used by QMessageBox and are platform dependent.
			 *
			 * @param type The requested type
			 * @return The icon being created
			 */
			static QIcon ConvertToIcon(NotificationType type);
			
			/**
			 * @brief Returns a HTML string with the notification's actions as anchors.
			 *
			 * The action's name is used for both: the anchor's text and its reference.
			 *
			 * Example: <b>[Action 1 | Action 2]</b>:
			 * @code
			 * [<a href="Action 1">Action 1</a> | <a href="Action 2">Action 2</a>]
			 * @endcode
			 *
			 * Note: ActivateAction() is not called automatically.
			 *
			 * @return The HTML string being created
			 *
			 * @sa ActivateAction()
			 */
			QString CreateLinksLabelText() const;
			
			/**
			 * @brief Triggers one of the notification's actions.
			 *
			 * Triggering an action means to run the lambda associated (nova::ActionList) with it.
			 * If the action isn't associated, nothing happens.
			 *
			 * The following line will run the lambda of the action with the name "Action 1":
			 * @code
			 * ActivateAction("Action 1");
			 * @endcode
			 *
			 * @param action The name of the action to be activated
			 *
			 * @sa CreateLinksLabelText()
			 */
			void ActivateAction(const QString& action);
			
			/**
			 * @brief Enables the notification and sends it to the associated nova::Notifier.
			 *
			 * The notifier's current notification gets updated and the old one is closed automatically.
			 *
			 * @sa Close()
			 * @sa nova::Notifier::get_current_notification()
			 */
			void Show();
			
			/**
			 * @brief Closes the notification.
			 *
			 * The notification gets automatically deleted.
			 *
			 * @sa Show()
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
			 * @sa nova::Notification::NotificationType
			 */
			NotificationType get_type() const { return type; }
			
			/**
			 * @brief Returns whether the notification has high priority and the nova::Notifier will draw
			 * extra attention.
			 */
			bool is_high_priority() const { return high_priority; }
		
		private:
			friend class Notifier;
			
			Notifier* const notifier;
			const QString title;
			const QString message;
			const NotificationType type;
			const bool high_priority;
			
			ActionList actions;
	};
	
	/**
	 * @brief An abstract class which can display Notification objects.
	 * @headerfile notification.h <nova/notification.h>
	 *
	 * Notifications are displayed twice:
	 * <ol>
	 *  <li>As a popup immediately after its trigger event</li>
	 *  <li>In a separate view until the notification is closed</li>
	 * </ol>
	 *
	 * nova::Workbench is an important nova::Notifier.
	 *
	 * This class must be derived.
	 *
	 * @sa nova::Notification
	 * @sa nova::Workbench
	 */
	class NOVA_API Notifier {
		public:
			NOVA_DISABLE_COPY(Notifier)
			virtual ~Notifier() noexcept = default;
			
			/**
			 * @brief Shows a notification without creating a nova::Notification object.
			 *
			 * @param title The notification's title
			 * @param message The (detailed) description of the notification
			 * @param type The notification's type (i.e. Information, Warning, Error) (optional, default: Information)
			 *
			 * @sa nova::Notification
			 */
			inline void ShowNotification(const QString& title, const QString& message,
			                             Notification::NotificationType type = Notification::Information) {
				(new Notification(this, title, message, type))->Show();
			}
			
			/**
			 * @brief Returns a pointer to the current nova::Notification being displayed.
			 */
			inline Notification* get_current_notification() const { return current_notification; }
		
		protected:
			/**
			 * @brief Creates a new nova::Notifier.
			 */
			inline Notifier():
					current_notification(nullptr) {}
			
			/**
			 * @brief Triggers an action of the current notification.
			 *
			 * The following line will trigger the action with the name "Action 1" of the current
			 * notification:
			 * @code
			 * ActivateAction("Action 1");
			 * @endcode
			 *
			 * @param action The name of the action to be activated
			 *
			 * @sa nova::Notification::ActivateAction()
			 * @sa get_current_notification()
			 */
			void ActivateNotificationAction(const QString& action) const;
			
			/**
			 * @brief This pure virtual method is called when the notification view must be updated.
			 *
			 * @param is_active is false if there's no active notification to be displayed (i.e. the view has to be cleared)
			 * @param notification The notification to be displayed
			 */
			virtual void UpdateNotificationView(bool is_active, const Notification* notification) = 0;
			
			/**
			 * @brief This pure virtual method is called when a popup has to be shown.
			 *
			 * This only happens once per notification.
			 *
			 * @param notification The notification for which the popup has to be shown
			 */
			virtual void ShowNotificationPopup(const Notification* notification) = 0;
		
		private:
			friend class Notification;
			
			Notification* current_notification;
			
			void Enable(Notification* notification);
			void Disable(Notification* notification);
	};
}

#endif  // NOVA_FRAMEWORK_NOTIFICATION_H
