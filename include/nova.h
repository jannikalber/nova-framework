/*
 * Copyright (c) 2021 by Jannik Alber.
 * All rights reserved.
 */

#ifndef NOVA_FRAMEWORK_NOVA_H
#define NOVA_FRAMEWORK_NOVA_H

#ifdef _MSC_VER
	#ifdef novaf_EXPORTS
		#define NOVA_API __declspec(dllexport)
	#else
		#define NOVA_API __declspec(dllimport)
	#endif
#else
	#define NOVA_API
#endif

namespace nova {
	class Workbench;
	
	extern NOVA_API Workbench* workbench;
}

#endif  // NOVA_FRAMEWORK_NOVA_H
