/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_FRAMEWORK_H__
#define __BE_FRAMEWORK_H__

#include <string>

namespace BiometricEvaluation
{
	/**
	 * @brief
	 * Information about the framework.
	 */
	namespace Framework
	{
		/**
		 * @brief
		 * Framework major version.
		 *
		 * @return
		 * The major version number of the BiometricFramework
		 */
		unsigned int
		getMajorVersion();
	
		/**
		 * @brief
		 * Framework minor version.
		 *
		 * @return
		 * The minor version of the BiometricEvaluation framework.
		 */
		unsigned int
		getMinorVersion();

		/**
		 * @brief
		 * Compiler used to compile this framework.
		 *
		 * @return
		 * The name of the compiler used to compile this framework.
		 */
		std::string
		getCompiler();

		/**
		 * @brief
		 * Version string of compiler used to compile this framework.
		 *
		 * @return
		 * Major, minor, and patch level of the compiler used.
		 */
		std::string
		getCompilerVersion();
	}
}

#endif /* __BE_FRAMEWORK_H__ */

