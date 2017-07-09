/*
 * @file mtest.cpp
 * @author Krzysztof Lasota
 */

class Abstract
{
public:
	virtual ~Abstract() {}
	virtual int fun(char c) = 0;
};

class User
{
public:
	User(Abstract& a) : a_(a) {}
	int run(char c)
	{
		return a_.fun(c);
	}
protected:
	Abstract& a_;
};


#include <gmock/gmock.h>

class MockAbstract : public Abstract
{
public:
	MOCK_METHOD1(fun, int(char c));
};


#include <gtest/gtest.h>

using testing::_;
using testing::AtLeast;
using testing::Return;
using testing::WithoutArgs;

ACTION(Ret0int) { return int(arg0); }

TEST(TestMock, Fun)
{
	MockAbstract mock;
	User u(mock);

	EXPECT_CALL(mock, fun(_))
		.Times(AtLeast(1))
		.WillOnce( Ret0int() );
	// ACTION(Ret) { return int(arg0); }
	// WithoutArgs(Ret)

	EXPECT_EQ( int('a'), u.run('a') ) << "Bad value";
}
