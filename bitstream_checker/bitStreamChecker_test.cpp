/**
 * @file bitStreamChecker_test.cpp
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
	TC(T01_OddRaised, 0x55u /*01010101*/, true) \
	TC(T02_EvenRaised, 0xAAu /*10101010*/, true) \
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

/**
 * List of Test Cases for class TripleBitStreamChecker.
 * Call verify method three times.
 *
 * TC(TestCaseName, testInput1, expectedResult1,
 *    testInput2, expectedResult2,
 *    testInput3, expectedResult3)
 */
#define LIST_OF_TESTCASES_TRIPLE_BIT_3_CALLS  \
	TC(T301_SecondCheckAfterFailure,  \
		0x84u /*10000001*/, false,  \
		0x7Eu /*01111110*/, false,  \
		0x4Au /*01001010*/, true)  \
	TC(T302_ThirdCheckAfterFailure,  \
		0x69u /*01101001*/, true,  \
		0x7Eu /*01111110*/, false,  \
		0x4Au /*01001010*/, true)  \
	TC(T311_CheckBitOrderInBytesPositive,  \
		0x5Bu /*01011011*/, true,  \
		0xD6u /*11010110*/, true,  \
		0x4Au /*01001010*/, true)  \
	TC(T312_CheckBitOrderInBytesNegative,  \
		0x5Bu /*01011011*/, true,  \
		0xCBu /*11001011*/, true,  \
		0x49u /*01001001*/, false)  \
	TC(T321_TripleRaisedBetweenBytes,  \
		0xD3u /*11010011*/, true,  \
		0x99u /*10011001*/, false,  \
		0x49u /*01001001*/, true)  \
	TC(T322_QuadraFalledBetweenBytes,  \
		0x34u /*00110100*/, true,  \
		0x2Cu /*00101100*/, false,  \
		0xC9u /*11001001*/, true)


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


#define TC(TestCaseName_, testInput1_, expectedResult1_,  \
						  testInput2_, expectedResult2_,  \
						  testInput3_, expectedResult3_)  \
	TEST_F(TripleBitStream_Test, TestCaseName_) \
	{  \
		uint8_t testInput1 = testInput1_;  \
		EXPECT_EQ(expectedResult1_, bsc->verify(bsc, testInput1))  \
			<< "  testInput is: " << cast_to_binary_string(testInput1);  \
		uint8_t testInput2 = testInput2_;  \
		EXPECT_EQ(expectedResult2_, bsc->verify(bsc, testInput2))  \
			<< "  testInput is: " << cast_to_binary_string(testInput2);  \
		uint8_t testInput3 = testInput3_;  \
		EXPECT_EQ(expectedResult3_, bsc->verify(bsc, testInput3))  \
			<< "  testInput is: " << cast_to_binary_string(testInput3);  \
	}
LIST_OF_TESTCASES_TRIPLE_BIT_3_CALLS
#undef TC
#undef LIST_OF_TESTCASES_TRIPLE_BIT_3_CALLS
