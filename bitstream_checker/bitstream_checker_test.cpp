/**
 * @file bitstream_checker_test.cpp
 * @brief Test for BitStreamCkecker class
 *
 * @author Krzysztof Lasota
 */

#include <gtest/gtest.h>

#include <stdint.h>
#include <string>

extern "C" {
#include "tripleBitStreamChecker.h"
}

// TC(TestCaseName, testInput, expectedResult)
#define LIST_OF_TESTCASES_TRIPLE_BIT \
	TC(T01_OddRaised, 0x55u, true) \
	TC(T02_EvenRaised, 0xAAu /*"10101010"*/, true) \
	TC(T03_OddPairRaised, 0x33u /*00110011*/, true) \
	TC(T04_EvenPairRaised, 0xCCu /*11001100*/, true) \
	TC(T05_OddTripleRaised, 0xC7 /*11000111*/, false) \
	TC(T06_EvenTripleRaised, 0x38u /*00111000*/, false) \
	TC(T07_FirstTripleRaised, 0x57 /*01010111*/, false) \
	TC(T08_FirstTripleFalled, 0xA8u /*10101000*/, false) \
	TC(T09_MiddleTripleRaised, 0xDD /*11011101*/, false) \
	TC(T10_MiddleTripleFalled, 0x22u /*00100010*/, false) \
	TC(T11_MiddlePairRaised, 0x5Au /*01011010*/, true) \
	TC(T12_MiddlePairFalled, 0xA5u /*10100101*/, true) \
	TC(T13_SpareBitRaised, 0x24u /*00100100*/, true) \
	TC(T14_SpareBitFalled, 0xDBu /*11011011*/, true) \
	TC(T15_Random, 0xDFu /*11011111*/, false) \
	TC(T16_Random, 0xB4u /*10110100*/, true) \
	TC(T17_Random, 0x15u /*00010101*/, false) \
	TC(T18_Random, 0x8Eu /*10001110*/, false) \
	TC(T19_Random, 0x59u /*01011001*/, true) \
	TC(T20_Random, 0xE6u /*11100110*/, false)


std::string cast_to_binary_string(BitChunk value)
{
	std::string binary_string = "";
	for( unsigned i = 0 ; i< sBitStreamChecker.bitChunkNoOfBits ; ++i ){
		binary_string.insert(0, (value & 0x01) ? "1": "0");
		value >>= 1;
	}
	return binary_string;
}


class TripleBitStream_Test: public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		bsc = (BitStreamChecker*)alloc_TripleBitStreamChecker();
//		bsc->ctxt->secretKey = 666;  // can't use private field of BitStreamChecker
	}

	virtual void TearDown()
	{
		bsc->free_self(bsc);
	}

public:
	BitStreamChecker* bsc;
};

#define TC(TestCaseName_, testInput_, expectedResult_) \
	TEST_F(TripleBitStream_Test, TestCaseName_) \
	{  \
		uint8_t testInput = testInput_;  \
		EXPECT_EQ(expectedResult_, bsc->verify(bsc, testInput))  \
			<< "  testInput is: " << cast_to_binary_string(testInput);  \
	}
LIST_OF_TESTCASES_TRIPLE_BIT
#undef TC
#undef LIST_OF_TESTCASES_TRIPLE_BIT
