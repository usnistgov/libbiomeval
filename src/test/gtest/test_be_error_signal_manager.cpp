/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <unistd.h>

#include <be_error_signal_manager.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(SignalManager, Construction)
{
	std::unique_ptr<BE::Error::SignalManager> sigmgr;
	EXPECT_NO_THROW(sigmgr.reset(new BE::Error::SignalManager()));
}

TEST(SignalManager, InvalidSignalSet)
{
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGSTOP);
	EXPECT_THROW(BE::Error::SignalManager bsigmgr(sigset),
	    BE::Error::ParameterError);

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGKILL);
	EXPECT_THROW(BE::Error::SignalManager bsigmgr(sigset),
	    BE::Error::ParameterError);

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGKILL);
	std::unique_ptr<BE::Error::SignalManager> sigmgr;
	EXPECT_NO_THROW(sigmgr.reset(new BE::Error::SignalManager()));
	EXPECT_THROW(sigmgr->setSignalSet(sigset),
	    BE::Error::ParameterError);
}

TEST(SignalManager, DefaultHandling)
{
	std::unique_ptr<BE::Error::SignalManager> sigmgr;
	EXPECT_NO_THROW(sigmgr.reset(new BE::Error::SignalManager()));

	BEGIN_SIGNAL_BLOCK(sigmgr, sigblock1);
		EXPECT_FALSE(sigmgr->sigHandled());
		std::raise(SIGSEGV);
		/* Shouldn't get here */
		EXPECT_FALSE(true);
	END_SIGNAL_BLOCK(sigmgr, sigblock1);
	EXPECT_TRUE(sigmgr->sigHandled());
}

TEST(SignalManager, CustomSignalList)
{
	std::unique_ptr<BE::Error::SignalManager> sigmgr;
	EXPECT_NO_THROW(sigmgr.reset(new BE::Error::SignalManager()));

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	sigmgr->setSignalSet(sigset);

	for (uint8_t i = 0; i < 3; i++) {
		BEGIN_SIGNAL_BLOCK(sigmgr, sigblock2);
			::kill(::getpid(), SIGUSR1);
			/* Shouldn't get here */
			EXPECT_FALSE(true);
		END_SIGNAL_BLOCK(sigmgr, sigblock2);
		EXPECT_TRUE(sigmgr->sigHandled());
	}

	EXPECT_DEATH(::kill(::getpid(), SIGUSR1), "");
	EXPECT_DEATH(::kill(::getpid(), SIGABRT), "");
}

