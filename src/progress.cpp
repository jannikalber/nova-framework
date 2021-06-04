/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#include "progress.h"

#include <QtWidgets/QApplication>

namespace nova {
	Task::Task(ProgressMonitor* monitor, const QString& task_name, bool is_indeterminate,
	           const std::function<StatusCode(Task*)>& lambda)
			: QThread(), task_name(task_name), indeterminate(is_indeterminate), lambda(lambda), value(0) {
		connect(this, &Task::finished, this, &Task::deleteLater);
		// Run the following lambdas on the main thread
		connect(this, &Task::started, qApp, [this, monitor] { monitor->Enable(this); });
		connect(this, &Task::disabled, qApp, [this, monitor]() { monitor->Disable(this); });
		connect(this, &Task::updated, qApp, [monitor]() { monitor->UpdateTasks(); });
	}
	
	void Task::set_value(int value) {
		this->value = qBound(0, value, 100);
		emit updated();
	}
	
	StatusCode Task::Run() {
		return lambda(this);
	}
	
	void Task::run() {
		const StatusCode status_code = Run();
		// if (!status_code.first) TODO: Errors must be implemented
		
		emit disabled();
		
		// Event loop for further signals and slots operations
		exec();
	}
	
	ProgressMonitor::ProgressMonitor() : tasks(QList<Task*>()) {
	}
	
	void ProgressMonitor::Enable(Task* task) {
		tasks << task;
		UpdateTasks();
	}
	
	void ProgressMonitor::Disable(Task* task) {
		tasks.removeAll(task);
		UpdateTasks();
	}
	
	void ProgressMonitor::UpdateTasks() {
		if (tasks.isEmpty()) UpdateView(false, nullptr, 0, 0);
		else {
			auto* task = tasks.at(0);
			UpdateView(true, task->get_task_name(), task->is_indeterminate() ? 0 : 100, task->get_value());
		}
	}
}
