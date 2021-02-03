/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_WORKBENCH_H
#define NOVA_FRAMEWORK_WORKBENCH_H

#include "nova.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>

namespace nova {
	class NOVA_API Workbench : public QMainWindow {
		public:
			explicit Workbench(QWidget* parent = nullptr);
	};
}

#endif  // NOVA_FRAMEWORK_WORKBENCH_H
