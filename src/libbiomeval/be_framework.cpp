/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <string>
#include <sstream>

#include <be_framework.h>

unsigned int
BiometricEvaluation::Framework::getMajorVersion()
{
	return (MAJOR_VERSION);
}

unsigned int
BiometricEvaluation::Framework::getMinorVersion()
{
	return (MINOR_VERSION);
}

std::string
BiometricEvaluation::Framework::getCompiler()
{
#if defined (__INTEL_COMPILER) || defined (__ICL)
	return ("icc");
#elif defined (__clang__)
	return ("clang");
#elif defined (__GNUC__)
	return ("gcc");
#else
	return ("Unknown");
#endif	/* Compilers */
}

std::string
BiometricEvaluation::Framework::getCompilerVersion()
{
	std::stringstream version;

	/* Intel */
#if defined (__INTEL_COMPILER) || defined (__ICL)
#if defined (__INTEL_COMPILER)
	uint16_t iccVersion = __INTEL_COMPILER;
	uint16_t iccMajorVersion = iccVersion / 100;
	uint16_t iccMinorVersion = iccVersion % 100;
	version << iccMajorVersion << "." << iccMinorVersion;
#elif defined (__ICL)
	uint16_t iccVersion = __ICL;
	uint16_t iccMajorVersion = iccVersion / 100;
	uint16_t iccMinorVersion = iccVersion % 100;
	version << iccMajorVersion << "." << iccMinorVersion;
#else
	version << "?.?";
#endif /* __INTEL_COMPILER */
#if defined (__INTEL_COMPILER_UPDATE)
	version << "." << __INTEL_COMPILER_UPDATE;
#else
	version << ".?";
#endif

	/* clang */
#elif defined (__clang__)
#ifdef __clang_major__
	version << __clang_major__;
#endif /* __clang_major__ */
#ifdef __clang_minor__
	version << "." << __clang_minor__;
#else
	version << ".?";
#endif /* __clang_minor__ */
#ifdef __clang_patchlevel__
	version << "." << __clang_patchlevel__;
#else
	version << ".?";
#endif /* __clang_patchlevel__ */

	/* GCC */
#elif defined (__GNUC__)
	version << __GNUC__;
#ifdef __GNUC_MINOR__
	version << "." << __GNUC_MINOR__;
#else
	version << ".?";
#endif /* __GNUC_MINOR__ */
#ifdef __GNUC_PATCHLEVEL__
	version << "." << __GNUC_PATCHLEVEL__;
#else
	version << ".?";
#endif /* __GNUC_PATCHLEVEL__ */

	/* Unknown */
#else
	version << "?.?.?";
#endif	/* Compilers */
	
	return (version.str());
}

std::string
BiometricEvaluation::Framework::getCompileDate()
{
	return (__DATE__);
}

std::string
BiometricEvaluation::Framework::getCompileTime()
{
	return (__TIME__);
}

