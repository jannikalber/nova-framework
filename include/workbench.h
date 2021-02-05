/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include "nova.h"

#include <QtCore/QtGlobal>
#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Ui { class Workbench; }

namespace nova {
	class NOVA_API Workbench : public QMainWindow {
		public:
			explicit Workbench(QWidget* parent = nullptr);
			Workbench(const Workbench&) = delete;
			Workbench(Workbench&&) = delete;
			
			virtual ~Workbench() noexcept;
			
		private:
			Ui::Workbench* ui;
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
