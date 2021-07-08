/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_NOVA_H
#define NOVA_FRAMEWORK_NOVA_H

/**
 * @mainpage
 *
 * Nova is an additional library for [Qt5](https://www.qt.io). Its goal is to simplify the
 * development of modern applications by providing a prefabricated Ui layout and common tools
 * like settings dialogs. Nova also supports plugins.
 *
 * **Note:** This library requires Qt5.
 *
 * ---
 * Nova is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.de.html). @n
 * Copyright (c) by Jannik Alber
 */

#ifdef _MSC_VER
	#ifdef Nova_EXPORTS
		#define NOVA_API __declspec(dllexport)
	#else
		#define NOVA_API __declspec(dllimport)
	#endif
#else
	#define NOVA_API
#endif

/**
 * @brief Nova library namespace
 *
 * All strings are translated, their contexts can be found in this documentation and usually begin with "nova/".
 * The translations may contain place markers (%1, %2, %3, ...) which are also used by QString::arg().
 */
namespace nova {
	class Workbench;
	
	/**
	 * @brief Points to the active workbench window.
	 *
	 * If two windows are created, it points to the last window being created.
	 *
	 * @sa Workbench::Workbench()
	 */
	extern NOVA_API Workbench* workbench;
}

#endif  // NOVA_FRAMEWORK_NOVA_H
