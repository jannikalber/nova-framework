/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_NOVA_H
#define NOVA_FRAMEWORK_NOVA_H

#include <QApplication>

/**
 * @mainpage
 *
 * Nova is an additional library for <a href="https://www.qt.io">Qt</a>. Its goal is to simplify the
 * development of modern applications by providing a prefabricated Ui layout and common tools
 * like settings dialogs. Nova also supports plugins.
 *
 * <b>Note:</b> This library requires Qt.
 *
 * <hr>
 * Nova is licensed under the <a href="https://www.gnu.org/licenses/gpl-3.0.de.html">GNU General Public License v3.0</a>.<br>
 * Copyright (c) by Jannik Alber
 */

#ifdef _MSC_VER
	#ifdef NovaFramework_EXPORTS
		#define NOVA_API __declspec(dllexport)
	#else
		#define NOVA_API __declspec(dllimport)
	#endif
#else
	#define NOVA_API
#endif

#define NOVA_DISABLE_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;

// Helper translation macro, only works if NOVA_CONTEXT is defined (this is the case in every source file having translations)
#define NOVA_TR(text) QApplication::translate(NOVA_CONTEXT, text)

/**
 * @brief Nova library namespace
 *
 * All strings are translated, their contexts can be found in this documentation and usually begin with "nova/".
 * The translations may contain place markers (%1, %2, %3, ...) for QString::arg().
 */
namespace nova {
	class Workbench;
	
	/**
	 * @brief Points to the active workbench window.
	 *
	 * If two windows are created, it points to the last window being created. Creating more
	 * than one workbench should be avoided.
	 *
	 * Please do not manually change this pointer.
	 *
	 * @sa nova::Workbench::Workbench()
	 */
	extern NOVA_API Workbench* workbench;
}

#endif  // NOVA_FRAMEWORK_NOVA_H
