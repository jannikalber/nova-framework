/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "progress.h"

#include <QApplication>

#include "notification.h"

namespace nova {
	Task::Task(ProgressMonitor* monitor, const QString& task_name, bool is_indeterminate,
	           const TaskLambda& lambda, bool needs_event_queue):
			QThread(), task_name(task_name), lambda(lambda), indeterminate(is_indeterminate),
			needs_event_queue(needs_event_queue), value(0) {
		connect(this, &Task::finished, this, &Task::deleteLater);
		// Run the following lambdas on the main thread
		connect(this, &Task::started, qApp, [this, monitor] { monitor->Enable(this); });
		connect(this, &Task::disabled, qApp, [this, monitor]() { monitor->Disable(this); });
		connect(this, &Task::updated, qApp, [monitor]() { monitor->UpdateTasks(); });
		connect(this, &Task::errorOccurred, qApp, [this, monitor](const QString& message) {
			monitor->ReportError(get_task_name(), message);
		});
	}
	
	void Task::set_value(int value) {
		this->value = qBound(0, value, 100);
		emit updated();
	}
	
	TaskResult Task::Run() {
		return lambda(this);
	}
	
	void Task::run() {
		const TaskResult status_code = Run();
		if (!status_code.first) emit errorOccurred(status_code.second);
		
		emit disabled();
		
		// Event loop for further signals and slots operations
		if (needs_event_queue) exec();
	}
	
	ProgressMonitor::ProgressMonitor(Notifier* notifier):
			notifier(notifier) {}
	
	Task* ProgressMonitor::get_current_task() const {
		return tasks.isEmpty() ? nullptr : tasks[0];
	}
	
	void ProgressMonitor::Enable(Task* task) {
		tasks << task;
		UpdateTasks();
	}
	
	void ProgressMonitor::Disable(Task* task) {
		tasks.removeAll(task);
		UpdateTasks();
	}
	
	void ProgressMonitor::ReportError(const QString& title, const QString& message) {
		if (notifier != nullptr) {
			auto* notification = new Notification(notifier, title, message, Notification::Error, true);
			notification->Show();
		}
	}
	
	void ProgressMonitor::UpdateTasks() {
		if (tasks.isEmpty()) UpdateProgressView(false, nullptr);
		else UpdateProgressView(true, tasks[0]);
	}
}
