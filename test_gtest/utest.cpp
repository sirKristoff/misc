/*
 * @file utest.cpp
 * @author Krzysztof Lasota
 */

#include <gtest/gtest.h>

TEST(Test1, Boolean)
{
	EXPECT_TRUE(true) << "Bad boolean";
}
