/**
 * @file iostream_test.cpp
 * @brief Test for iostream functionality
 *
 * @author Krzysztof Lasota
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <sstream>
#include <string>


/**
 * @note What ios::tie() does:
 * @url https://4programmers.net/C/Wymuszanie_wysokiej_wydajno%C5%9Bci_iostream
 */
class IosTie_Test: public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		orig_cin_tie = std::cin.tie();
		orig_cout_tie = std::cout.tie();
	}

	virtual void TearDown()
	{
		std::cin.tie(orig_cin_tie);
		std::cout.tie(orig_cout_tie);
	}

public:
	std::ostream* orig_cin_tie;
	std::ostream* orig_cout_tie;
};

namespace mock {
class ostream : public std::ostream
{
public:
	MOCK_METHOD0(flush, std::ostream&());
};
}

TEST_F(IosTie_Test, CinTie)
{
	std::string expectedStr= "basic string";
	std::ostringstream sout;

	EXPECT_EQ( &std::cout, std::cin.tie(&sout) )
		<< "ERROR: std::cin has different tied output stream, expected std::cout";

	*std::cin.tie() << expectedStr;
	EXPECT_EQ( expectedStr, sout.str() )
		<< "ERROR: sout wasn't tied to std::cin";
}

TEST_F(IosTie_Test, CoutTie)
{
	std::string expectedStr= "multi line\n\t string\n";
	std::ostringstream sout;

	EXPECT_EQ( NULL, std::cout.tie(&sout) )
		<< "ERROR: std::cout has tied output stream";

	*std::cout.tie() << expectedStr;

	EXPECT_EQ( expectedStr, sout.str() )
		<< "ERROR: sout wasn't tied to std::cout";
}

//ACTION(RetThisRef) { return *this; }
TEST_F(IosTie_Test, TieFlushCheck)
{
	mock::ostream mock_out;

	std::cout.tie(&mock_out);

	EXPECT_CALL(mock_out, flush())
		.Times(1)
		.WillOnce( ::testing::ReturnRef(mock_out) );

	mock_out.flush(); // TODO: remove flushing to test cout tie

	std::cout << "anything\n";

	EXPECT_TRUE(true);
}

/**
 * @brief Unit Test for ios::streambuf::rdbuf()
 */
TEST(IosStreamBuf_Test, Rdbuf)
{
	const float expFloat = 3.1415;
	const unsigned expInt = 10;
	std::stringstream sin;
	std::istream& stream_in = std::cin;

	std::streambuf* orig_cin_streambuf_ptr = std::cin.rdbuf(sin.rdbuf());

	// Set input stream
	sin << expFloat << " " << expInt;

	// test read from std input stream
	float actualFloat = 0.0;
	stream_in >> actualFloat;
	EXPECT_FLOAT_EQ(expFloat, actualFloat);

	unsigned actualInt = 0;
	stream_in >> actualInt;
	EXPECT_EQ(expInt, actualInt);

	std::cin.rdbuf(orig_cin_streambuf_ptr);
}


class IosExceptions_Test: public ::testing::Test
{
public:
	IosExceptions_Test() :
		ssioPtr()
	{
	}

protected:
	virtual void SetUp()
	{
		ssioPtr = new std::stringstream();
		std::stringstream& ssio= *ssioPtr;

		ssio << "c " << expFloat << " " << expInt;
	}

	virtual void TearDown()
	{
		delete ssioPtr;
	}

public:
	std::stringstream* ssioPtr;
	const float expFloat = 3.1415;
	const unsigned expInt = 10;
};

TEST_F(IosExceptions_Test, Basic)
{
	std::stringstream& ssio= *ssioPtr;

	float actualFloat = 0.0;
//	unsigned actualInt = 0;

	ssio.exceptions( std::ios_base::failbit);

	// TODO: investigate type of exeption which should be thrown
	EXPECT_THROW((ssio>>actualFloat), std::ios_base::failure);
}
