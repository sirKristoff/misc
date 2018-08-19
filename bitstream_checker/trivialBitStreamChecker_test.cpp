/*
 * trivialBitStreamChecker_test.cpp
 *
 *  Created on: 19.08.2018
 *      Author: Krzysztof Lasota
 */

#include <gtest/gtest.h>

#include <iostream>

extern "C" {
#include "trivialBitStreamChecker.h"
}


class TrivialBitStreamChecker_Test: public ::testing::Test
{
public:
	static uint8_t streamNo;

protected:
	virtual void SetUp()
	{
		++streamNo;
	}
	virtual void TearDown()
	{
	}
};
uint8_t TrivialBitStreamChecker_Test::streamNo = -1;


TEST_F(TrivialBitStreamChecker_Test, T01_Simple)
{
	auto checkNo = __LINE__ + 1;
#define LIST_OF_CHECKS  \
	CHK(1, true)  \
	CHK(1, true)  \
	CHK(1, false)  \
	CHK(1, false)  \
	CHK(0, true)  \
	CHK(1, true)  \
	CHK(1, true)  \
	CHK(1, false)  \
	CHK(0, true)   \
	CHK(0, true)

#define CHK(bit_, expectedResult_) \
	++checkNo;  \
	EXPECT_EQ(expectedResult_, verify(bit_, streamNo))  \
		<< (__FILE__) << ":" << checkNo << ": check failed";
	LIST_OF_CHECKS
#undef CHK
#undef LIST_OF_CHECKS
}


TEST_F(TrivialBitStreamChecker_Test, T02_Simple)
{
	auto checkNo = __LINE__ + 1;
#define LIST_OF_CHECKS  \
	CHK(0, true)  \
	CHK(0, true)  \
	CHK(0, false)  \
	CHK(0, false)  \
	CHK(1, true)  \
	CHK(0, true)  \
	CHK(0, true)  \
	CHK(0, false)

#define CHK(bit_, expectedResult_) \
	++checkNo;  \
	EXPECT_EQ(expectedResult_, verify(bit_, streamNo))  \
		<< (__FILE__) << ":" << checkNo << ": check failed";
	LIST_OF_CHECKS
#undef CHK
#undef LIST_OF_CHECKS
}


TEST_F(TrivialBitStreamChecker_Test, T03_Simple)
{
	auto checkNo = __LINE__ + 1;
#define LIST_OF_CHECKS  \
	CHK(1, true )  \
	CHK(1, true )  \
	CHK(1, false)  \
	CHK(0, true )  \
	CHK(0, true )  \
	CHK(0, false)  \
	CHK(0, false)  \
	CHK(1, true )

#define CHK(bit_, expectedResult_) \
	++checkNo;  \
	EXPECT_EQ(expectedResult_, verify(bit_, streamNo))  \
		<< (__FILE__) << ":" << checkNo << ": check failed";
	LIST_OF_CHECKS
#undef CHK
#undef LIST_OF_CHECKS
}


TEST_F(TrivialBitStreamChecker_Test, T04_TwoParallelStreams)
{
	auto checkNo = __LINE__ + 1;
#define LIST_OF_CHECKS  \
	CHK(0, 1, true )  \
	CHK(0, 1, true )  \
	CHK( 1, 1, true )  \
	CHK( 1, 0, true )  \
	CHK(0, 1, false)  \
	CHK( 1, 0, true )  \
	CHK(0, 1, false)  \
	CHK( 1, 0, false)

#define CHK(streamNoIdx_, bit_, expectedResult_) \
	++checkNo;  \
	EXPECT_EQ(expectedResult_, verify(bit_, streamNo+streamNoIdx_))  \
		<< (__FILE__) << ":" << checkNo << ": check failed";
	LIST_OF_CHECKS
#undef CHK
#undef LIST_OF_CHECKS
}
