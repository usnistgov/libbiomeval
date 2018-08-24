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

#include <be_time_timer.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

TEST(Timer, constructor)
{
	BE::Time::Timer *timer = nullptr;
	ASSERT_NO_THROW(timer = new BE::Time::Timer());
	ASSERT_NE(nullptr, timer);
	delete timer;
}

TEST(Timer, stop)
{
	BE::Time::Timer *timer = new BE::Time::Timer();
	ASSERT_NE(nullptr, timer);
	EXPECT_THROW(timer->stop(), BE::Error::StrategyError);
	delete timer;
}

TEST(Timer, elapsed)
{
	BE::Time::Timer *timer = new BE::Time::Timer();
	ASSERT_NE(nullptr, timer);

	/* Call elapsed before starting */
	EXPECT_NO_THROW(EXPECT_EQ(0, timer->elapsed()));
	//EXPECT_EQ(0, timer->elapsed())
	
	/* Call elapsed before after starting but before stopping */
	timer->start();
	EXPECT_THROW(timer->elapsed(), BE::Error::StrategyError);
	timer->stop();
	delete timer;
}

TEST(Timer, oneSecondSleep)
{
	BE::Time::Timer *timer = new BE::Time::Timer();
	ASSERT_NE(nullptr, timer);

	timer->start();
	sleep(1);
	timer->stop();

	/* 0.2% accuracy for 1 second */
	EXPECT_LT(timer->elapsed(), 1002000);
	EXPECT_GT(timer->elapsed(), 998000);

	delete timer;
}

