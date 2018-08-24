/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include <unistd.h>

#include <be_time_timer.h>
#include <be_time_watchdog.h>
#include <be_error_signal_manager.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

bool
isPrime(
    uint64_t number)
{
	if (number <= 1)
		return (false);

	for (uint64_t i = 2; i < number; i++)
		if ((number % i == 0) && (i != number))
			return (false);
	return (true);
}
/** @return true, after ~1 second */
bool returnTrueAfterDelay() { return (isPrime(179424673)); }

void
testWatchdogAndSignalManager(
    const std::unique_ptr<BE::Time::Watchdog> &theDog)
{
	/*
	 * Create a signal manager object to test its interaction
	 * with the Watchdog object, because both classes depend on
	 * POSIX signal handling.
	 */
	std::unique_ptr<BE::Error::SignalManager> sigmgr{
	    new BE::Error::SignalManager()};

	/*
	 * Test timer expiration with signal handling inside.
	 */
	theDog->setInterval(700);
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock1);
		/* Should throw a signal */
		BEGIN_SIGNAL_BLOCK(sigmgr, sigblock1);
			std::raise(SIGSEGV);
			/* Shouldn't get here */
			EXPECT_FALSE(true);
		END_SIGNAL_BLOCK(sigmgr, sigblock1);

		/* Should time out */
		EXPECT_FALSE(returnTrueAfterDelay());
	END_WATCHDOG_BLOCK(theDog, watchdogblock1);
	EXPECT_TRUE(sigmgr->sigHandled());
	EXPECT_TRUE(theDog->expired());

	/*
	 * Test signal handling with timer expiration inside.
	 */
	BEGIN_SIGNAL_BLOCK(sigmgr, sigblock2);
		BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock2);
			/* Should time out */
			EXPECT_FALSE(returnTrueAfterDelay());

		END_WATCHDOG_BLOCK(theDog, watchdogblock2);
		/* Should throw a signal */
		std::raise(SIGSEGV);
		EXPECT_FALSE(true);
	END_SIGNAL_BLOCK(sigmgr, sigblock2);
	EXPECT_TRUE(theDog->expired());
	EXPECT_TRUE(sigmgr->sigHandled());
}

void
testWatchdog(
    const std::unique_ptr<BE::Time::Watchdog> &theDog)
{
	/* Expiration */
	theDog->setInterval(300);
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock1);
		/* Should time out */
		EXPECT_FALSE(returnTrueAfterDelay());
	END_WATCHDOG_BLOCK(theDog, watchdogblock1);
	EXPECT_TRUE(theDog->expired());

	/* Cancelation */
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock2);
	END_WATCHDOG_BLOCK(theDog, watchdogblock2);
	EXPECT_FALSE(theDog->expired());
	EXPECT_TRUE(returnTrueAfterDelay());

	/*
	 * Test that the actual Watchdog timeout matches reality.
	 */
	BE::Time::Timer timer;
	theDog->setInterval(BE::Time::OneHalfSecond);
	timer.start();
	BEGIN_WATCHDOG_BLOCK(theDog, watchdogblock3);
		/* Should time out */
		EXPECT_FALSE(returnTrueAfterDelay());
	END_WATCHDOG_BLOCK(theDog, watchdogblock3);
	EXPECT_TRUE(theDog->expired());
	timer.stop();
	/* Allow 5% tolearance */
	int diff = BE::Time::OneHalfSecond - timer.elapsed();
	EXPECT_LT(abs(diff), BE::Time::OneHalfSecond * 0.05);
}

TEST(Watchdog, Creation)
{
	std::unique_ptr<BE::Time::Watchdog> Indy;
#ifdef __CYGWIN__
	ASSERT_THROW(Indy.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::PROCESSTIME), BE::Error::NotImplemented));
#else
	EXPECT_NO_THROW(Indy.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::PROCESSTIME)));
	ASSERT_TRUE(Indy != nullptr);
	Indy.reset(nullptr);
#endif

	EXPECT_NO_THROW(Indy.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::REALTIME)));
	ASSERT_TRUE(Indy != nullptr);
}


#ifndef __CYGWIN__
TEST(Watchdog, ProcessTime)
{
	std::unique_ptr<BE::Time::Watchdog> watchdog;

	EXPECT_NO_THROW(watchdog.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::PROCESSTIME)));
	ASSERT_NE(watchdog, nullptr);
	testWatchdog(watchdog);
}

TEST(Watchdog, ProcessTimeSignalManager)
{
	std::unique_ptr<BE::Time::Watchdog> watchdog;
	EXPECT_NO_THROW(watchdog.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::PROCESSTIME)));
	ASSERT_NE(watchdog, nullptr);
	testWatchdogAndSignalManager(watchdog);
}
#endif

TEST(Watchdog, RealTime)
{
	std::unique_ptr<BE::Time::Watchdog> watchdog;
	EXPECT_NO_THROW(watchdog.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::REALTIME)));
	ASSERT_NE(watchdog, nullptr);
	testWatchdog(watchdog);
}

TEST(Watchdog, RealTimeSignalManager)
{
	std::unique_ptr<BE::Time::Watchdog> watchdog;
	EXPECT_NO_THROW(watchdog.reset(new BE::Time::Watchdog(
	    BE::Time::Watchdog::REALTIME)));
	ASSERT_NE(watchdog, nullptr);
	testWatchdogAndSignalManager(watchdog);
}

