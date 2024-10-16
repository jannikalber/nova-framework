/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_PROGRESS_H
#define NOVA_FRAMEWORK_PROGRESS_H

#include <functional>

#include <QObject>
#include <QThread>
#include <QPair>
#include <QList>
#include <QString>

#include "nova.h"

namespace nova {
	class Task;
	class Notifier;
	class ProgressMonitor;
}

namespace nova {
	/**
	 * @brief Describes if a nova::Task succeeded or not.
	 *
	 * The typedef is translated to QPair<bool, QString>.
	 *
	 * Possible values:
	 * <ul>
	 *  <li>(true, nullptr): the task succeeded</li>
	 *  <li>(false, error message): the task failed, the error message is shown in the workbench</li>
	 * </ul>
	 *
	 * @sa nova::Task
	 */
	typedef QPair<bool, QString> TaskResult;
	
	/**
	 * @brief Specifies the lambda which is run when a nova::Task is executed.
	 *
	 * The typedef is translated to std::function<nova::TaskResult(nova::Task* task)>.
	 *
	 * The function parameter 'task' is the task the lambda belongs to. It returns a nova::TaskResult object.
	 *
	 * @sa nova::TaskResult
	 */
	typedef std::function<TaskResult(Task* task)> TaskLambda;
	
	/**
	 * @brief A nova::Task is bound to a thread whose progress can be shown in nova::ProgressMonitor objects.
	 * @headerfile progress.h <nova/progress.h>
	 *
	 * The task consists of two phases:
	 *
	 * The first phase is the task itself whose progress is monitored.
	 *
	 * The task isn't destroyed, when the first phase ends: It's still possible to receive some events. This phase
	 * is the second one and it isn't shown in monitors anymore. The phase can be terminated by calling quit().
	 *
	 * @sa Nova::ProgressMonitor
	 */
	class NOVA_API Task : public QThread {
		Q_OBJECT
		
		public:
			/**
			 * @brief Creates a new nova::Task.
			 *
			 * The task has to be started manually by calling start().
			 *
			 * The task gets automatically deleted when it finished.
			 *
			 * @param monitor The monitor in which the task is shown
			 * @param task_name A short description of the task (e.g. "Connecting")
			 * @param is_indeterminate If this property is true, the task has no percentage value and a busy progress bar is
			 * shown instead (optional, default: true).
			 * @param lambda This can be a lambda which is executed when the task starts. It's parameter is the task
			 * itself for changing the percentage value. It must return a nova::TaskResult object. (optional: You can
			 * reimplement Run() instead).
			 * @param needs_event_queue If this flag is set, the task does start an event queue for signals and slots to work.
			 * Please make sure to use quit() to terminate the queue. The second phase of the description above only exists if this
			 * flag is set.
			 */
			Task(ProgressMonitor* monitor, const QString& task_name, bool is_indeterminate = true,
			     const TaskLambda& lambda = nullptr, bool needs_event_queue = false);
			
			/**
			 * @brief Returns the task's name.
			 */
			inline QString get_task_name() const { return task_name; }
			
			/**
			 * @brief Returns true when the task is indeterminate.
			 */
			inline bool is_indeterminate() const { return indeterminate; }
			
			/**
			 * @brief Updates the percentage value of non-indeterminate tasks.
			 *
			 * Indeterminate tasks internally also have a percentage value
			 * being ignored by progress monitors.
			 *
			 * @param value is the percentage value between 0 and 100
			 */
			void set_value(int value);
			
			/**
			 * @brief Returns the percentage value of non-indeterminate tasks.
			 */
			inline int get_value() const { return value; }
		
		protected:
			/**
			 * @brief If you inherit nova::Task, please reimplement Run().
			 *
			 * This is the same as having a lambda set. The lambda is just called in the default implementation.
			 *
			 * Because of this, please do not call the overridden method, if you
			 * reimplement Run(). This can result in undefined behavior.
			 *
			 * @return A nova::TaskResult object which contains information about whether the task succeeded or not.
			 * @sa Task() (see parameter lambda)
			 */
			virtual TaskResult Run();
			
			/**
			 * This method is internally required and should not be called.
			 */
			void run() override;
		
		private:
			const QString task_name;
			const TaskLambda lambda;
			const bool indeterminate;
			int value;
			
			const bool needs_event_queue;
		
		signals:
			//! @cond
			void errorOccurred(const QString&);
			void disabled();
			void updated();
			//! @endcond
	};
	
	/**
	 * @brief A nova::ProgressMonitor is an abstract class which can display nova::Task objects.
	 * @headerfile progress.h <nova/progress.h>
	 *
	 * It often contains a progress bar and a label with the task's name.
	 *
	 * nova::Workbench is an important nova::ProgressMonitor.
	 *
	 * The workbench's progress monitor shows the hint "Ready" in the status bar if there's no active task
	 * (translation context: "nova/workbench").
	 *
	 * This class must be derived.
	 *
	 * @sa nova::Task
	 */
	class NOVA_API ProgressMonitor {
		public:
			NOVA_DISABLE_COPY(ProgressMonitor)
			inline virtual ~ProgressMonitor() noexcept = default;
			
			/**
			 * @brief Returns a pointer to the active task or nullptr if there's none.
			 */
			Task* get_current_task() const;
		
		protected:
			/**
			 * @brief Creates a new nova::ProgressMonitor.
			 *
			 * @param notifier If a task fails, an error message is sent to this notifier (optional, default: none).
			 */
			explicit ProgressMonitor(Notifier* notifier = nullptr);
			
			/**
			 * @brief This pure virtual method is called when the monitor's view must be updated.
			 *
			 * @param is_active is false if there's no task to be displayed (i.e. the view has to be cleared)
			 * @param task The task to be displayed
			 */
			virtual void UpdateProgressView(bool is_active, const Task* task) = 0;
		
		private:
			friend class Task;
			
			Notifier* const notifier;
			QList<Task*> tasks;
			
			void Enable(Task* task);
			void Disable(Task* task);
			void ReportError(const QString& title, const QString& message);
			void UpdateTasks();
	};
}

#endif  // NOVA_FRAMEWORK_PROGRESS_H
