/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_PROGRESS_H
#define NOVA_FRAMEWORK_PROGRESS_H

#include <functional>

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QString>

#include "nova.h"

namespace nova {
	class ProgressMonitor;
	/**
	 * @brief Describes if a Task succeeded or not.
	 *
	 * The typedef is translated to QPair<bool, QString>.
	 *
	 * Possible values:
	 * - (true, nullptr): the task succeeded
	 * - (false, error message): the task failed, the error message is shown in the workbench
	 */
	typedef QPair<bool, QString> StatusCode;
	
	/**
	 * @brief A task is bound to a thread which can be shown in ProgressMonitor objects.
	 * @headerfile progress.h <nova/progress.h>
	 *
	 * The task consists of two phases:
	 *
	 * The first phase is the task itself which is represented in ProgressMonitor objects (i.e. which can have a
	 * percentage value).
	 *
	 * The task isn't destroyed, when the first phase ends: It's still possible to receive some events. The task
	 * won't be shown in monitors anymore. This phase can be terminated by calling quit().
	 */
	class NOVA_API Task : public QThread {
		Q_OBJECT
		
		public:
			/**
			 * @brief Constructs a new task.
			 *
			 * The task isn't started. Call start().
			 *
			 * @param monitor The monitor in which the task is shown
			 * @param task_name is a short description being represented in the monitor
			 * @param is_indeterminate If this property is true, the task has no percentage value (optional, default: true).
			 * @param lambda This can be a lambda which is executed in the first phase. It's parameter is the task
			 * itself for changing the percentage value. It must return a StatusCode (optional: please override Run() instead).
			 */
			Task(ProgressMonitor* monitor, const QString& task_name, bool is_indeterminate = true,
			     const std::function<StatusCode(Task*)>& lambda = nullptr);
			
			/**
			 * @brief Returns the task's name.
			 */
			inline QString get_task_name() const { return task_name; }
			
			/**
			 * @brief Returns true if the task is indeterminate (i.e. has no percentage value)
			 */
			inline bool is_indeterminate() const { return indeterminate; }
			
			/**
			 * @brief Updates the percentage value of non-indeterminate tasks.
			 *
			 * Indeterminate tasks can also have a percentage value which is never shown.
			 *
			 * @param value is the percentage value between 0 and 100
			 * @sa get_value()
			 */
			void set_value(int value);
			
			/**
			 * @brief Returns the percentage value of non-indeterminate tasks.
			 * @sa set_value()
			 */
			inline int get_value() const { return value; }
		
		protected:
			/**
			 * @brief If you inherit Task, please override Run().
			 *
			 * This is the same as having a lambda set. The lambda is just called in the method.
			 *
			 * Because of this, please do not call the overridden method, if you
			 * override Run(). This can result in undefined behaviour.
			 *
			 * @return A StatusCode which contains information if the task succeeded or not.
			 * @sa Task() (see parameter lambda)
			 */
			virtual StatusCode Run();
			
			/**
			 * @brief Reimplements QThread::run()
			 */
			void run() override;
		
		private:
			const QString task_name;
			const std::function<StatusCode(Task*)> lambda;
			const bool indeterminate;
			
			int value;
		
		signals:
			/// @cond
			void disabled();
			void updated();
			/// @endcond
	};
	
	/**
	 * @brief A progress monitor is an abstract class which can display Task objects.
	 * @headerfile progress.h <nova/progress.h>
	 *
	 * It often contains a progress bar and a label with the task's name.
	 */
	class NOVA_API ProgressMonitor {
		public:
			/**
			 * @brief Constructs a new progress monitor.
			 *
			 * The workbench's ProgressMonitor shows the hint "Ready" in the status bar, if there's no active task
			 * (context: "nova/progress").
			 */
			ProgressMonitor();
		
		protected:
			/**
			 * @brief This method is called when the widget must be updated.
			 *
			 * @param is_active is false if there's no task to be displayed
			 * @param label_text is the text which should be displayed in the monitor's label
			 * @param max is set to 0 if the task is is indeterminate, else it's set to 100. It's enough to call
			 * QProgressBar::setMaximum()
			 * @param val is the percentage value
			 */
			virtual void UpdateView(bool is_active, const QString& label_text, int max, int val) = 0;
		
		private:
			friend class Task;
			
			QList<Task*> tasks;
			
			void Enable(Task* task);
			void Disable(Task* task);
			void UpdateTasks();
	};
}

#endif  // NOVA_FRAMEWORK_PROGRESS_H
