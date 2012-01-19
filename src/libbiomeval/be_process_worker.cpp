/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <be_process_worker.h>

BiometricEvaluation::Process::Worker::Worker() :
    _stopRequested(false),
    _parameters(ParameterList())
{

}

bool
BiometricEvaluation::Process::Worker::stopRequested()
    const
{
	return (_stopRequested);
}

void
BiometricEvaluation::Process::Worker::setParameter(
    const string &name, 
    tr1::shared_ptr<void> argument)
{
	_parameters[name] = argument;
}

tr1::shared_ptr<void>
BiometricEvaluation::Process::Worker::getParameter(
    const string &name)
{
	return (_parameters[name]);
}

void
BiometricEvaluation::Process::Worker::stop()
{
	_stopRequested = true;
}

BiometricEvaluation::Process::Worker::~Worker()
{

}
