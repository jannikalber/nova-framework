/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include "nova.h"

#include <QtCore/QtGlobal>
#include <QtWidgets/QMainWindow>

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE
class QWidget;

namespace Ui { class Workbench; }
QT_END_NAMESPACE

namespace nova {
	/**
	 * @brief This class represents the main window of the application.
	 * @headerfile workbench.h <nova/workbench.h>
	 *
	 * The workbench has a prefabricated Ui layout. Its content is a tab widget. One can add content types
	 * which can be displayed in this widget. The window also contains areas for tool windows, menus and
	 * a status bar which can be extended too.
	 *
	 * Only one workbench window should be constructed in one application.
	 *
	 * @sa nova::workbench
	 */
	class NOVA_API Workbench : public QMainWindow {
		public:
			/**
			 * Constructs a new workbench. The constructor should only be called once in one application.
			 * Calling this constructor updates automatically the reference of nova::workbench.
			 *
			 * @param parent The parent window which blocks its input until the workbench window is closed.
			 * @sa nova::workbench
			 */
			explicit Workbench(QWidget* parent = nullptr);
			Workbench(const Workbench&) = delete;
			Workbench(Workbench&&) = delete;
			
			virtual ~Workbench() noexcept;
		
		private:
			Ui::Workbench* ui;
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
