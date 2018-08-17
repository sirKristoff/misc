/*
 * setContainer_test.cpp
 *
 *  Created on: 14.08.2018
 *      Author: Krzysztof Lasota
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdlib>

extern "C" {
#include "vectorContainer.h"
}


using ::testing::_;

class VectorContainer_Test: public ::testing::Test
{
	struct FreeSetElementFunctorAbstract {
		virtual ~FreeSetElementFunctorAbstract() {}
		virtual void free(void* setElement) = 0;
	};
	struct FreeSetElementFunctorMock : public FreeSetElementFunctorAbstract {
		MOCK_METHOD1(free, void(void*));
	};

public:
	static void verifyAndResetFreeFunMock()
	{
		delete freeFun;
		freeFun = new FreeSetElementFunctorMock();
	}

	SetContainer* sc;
	static FreeSetElementFunctorMock* freeFun;
	const size_t notSpecifiedElementKey = 0;

protected:
	virtual void SetUp()
	{
		freeFun = new FreeSetElementFunctorMock();
		sc = (SetContainer*) alloc_VectorContainer(
				[=](void* setElement){freeFun->free(setElement);});
	}

	virtual void TearDown()
	{
		sc->free_self(sc);
		delete freeFun;
	}
};

VectorContainer_Test::FreeSetElementFunctorMock* VectorContainer_Test::freeFun = 0;



TEST_F(VectorContainer_Test, T01_CheckInitialization)
{
	EXPECT_EQ(0, sc->size(sc));
	EXPECT_LT(0, sc->capacity(sc));
}

TEST_F(VectorContainer_Test, T02_SingleInsertion)
{
	SetElement e = {1};

	EXPECT_EQ(&e, sc->insert(sc, &e));
	EXPECT_EQ(1, sc->size(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(1);
}

TEST_F(VectorContainer_Test, T03_NoKeyInsertion)
{
	const size_t numElem = 2;
	SetElement e[numElem] = {notSpecifiedElementKey, notSpecifiedElementKey};

	size_t idx = 0;
	EXPECT_EQ(&e[idx], sc->insert(sc, &e[idx]));
	EXPECT_EQ(1, sc->size(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));
	EXPECT_EQ(1, e[idx].key);

	++idx;
	EXPECT_EQ(&e[idx], sc->insert(sc, &e[idx]));
	EXPECT_EQ(2, sc->size(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));
	EXPECT_EQ(2, e[idx].key);

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[1])).Times(1);
	EXPECT_CALL(*freeFun, free(&e[0])).Times(1);
}

TEST_F(VectorContainer_Test, T04_InsertionOfFarKey)
{
	SetElement e = {4};

	EXPECT_EQ(&e, sc->insert(sc, &e));
	EXPECT_EQ(4, sc->size(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(1);
}

TEST_F(VectorContainer_Test, T05_ExtendCapacityIfRequiredOnInsertion)
{
	SetElement e = {notSpecifiedElementKey};
	ASSERT_EQ(0, sc->size(sc));

	while( sc->size(sc) < sc->capacity(sc) )
		sc->insert(sc, &e)->key = notSpecifiedElementKey;
	EXPECT_EQ(sc->size(sc), sc->capacity(sc));
	auto prevCapacity = sc->capacity(sc);

	// TRIGGER
	EXPECT_EQ(&e, sc->insert(sc, &e));
	// VERIFY
	EXPECT_LT(prevCapacity, sc->capacity(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(sc->size(sc));
}

TEST_F(VectorContainer_Test, T06_ExtendCapacityTwiceIfRequiredOnInsertion)
{
	SetElement e = {notSpecifiedElementKey};
	ASSERT_EQ(0, sc->size(sc));

	while( sc->size(sc) < sc->capacity(sc) )
		sc->insert(sc, &e)->key = notSpecifiedElementKey;
	sc->insert(sc, &e)->key = notSpecifiedElementKey;
	while( sc->size(sc) < sc->capacity(sc) )
		sc->insert(sc, &e)->key = notSpecifiedElementKey;
	EXPECT_EQ(sc->size(sc), sc->capacity(sc));
	auto prevCapacity = sc->capacity(sc);

	// TRIGGER
	EXPECT_EQ(&e, sc->insert(sc, &e));
	// VERIFY
	EXPECT_LT(prevCapacity, sc->capacity(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(sc->size(sc));
}

TEST_F(VectorContainer_Test, T07_InsertionWithReplacement)
{
	const size_t numElem = 2;
	SetElement e[numElem] = {1, 1};

	EXPECT_EQ(&e[0], sc->insert(sc, &e[0]));
	auto size = sc->size(sc);

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(&e[0])).Times(1);
	// TRIGGER
	EXPECT_EQ(&e[1], sc->insert(sc, &e[1]));
	// VERIFY
	EXPECT_EQ(&e[1], sc->find(sc, e[1].key))
		<< "Element in container was not replaced";
	EXPECT_EQ(size, sc->size(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[1])).Times(1);
}

TEST_F(VectorContainer_Test, T08_InsertionWithResizeVectorBecomeSparse)
{
	size_t sparseKey = 13;
	SetElement e = {sparseKey};

	// TRIGGER
	EXPECT_EQ(&e, sc->insert(sc, &e));
	// VERIFY
	EXPECT_EQ(sparseKey, sc->size(sc));
	EXPECT_LE(sc->size(sc), sc->capacity(sc));
	EXPECT_EQ(&e, sc->find(sc, e.key));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(1);
}

TEST_F(VectorContainer_Test, T08a_InsertionWithoutResizeIntoSparseContainer)
{
	const size_t numElem = 2;
	const size_t sparseKey = 7;
	const size_t newSparseKey = 5;
	SetElement e[numElem] = {sparseKey, newSparseKey};

	ASSERT_EQ(&e[0], sc->insert(sc, &e[0]));
	ASSERT_EQ(sparseKey, sc->size(sc));

	// TRIGGER
	EXPECT_EQ(&e[1], sc->insert(sc, &e[1]));
	// VERIFY
	EXPECT_EQ(&e[1], sc->find(sc, e[1].key));
	EXPECT_EQ(sparseKey, sc->size(sc));

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[1])).Times(1);
	EXPECT_CALL(*freeFun, free(&e[0])).Times(1);
}

TEST_F(VectorContainer_Test, T09_ClearContainer)
{
	const size_t numElem = 3;
	SetElement e[numElem] = {notSpecifiedElementKey};

	for( auto idx = 0u ; idx < numElem ; ++idx )
		sc->insert(sc, &e[idx]);
	ASSERT_EQ(numElem, sc->size(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	for( auto idx = 0u ; idx < numElem ; ++idx ){
		EXPECT_CALL(*freeFun, free(&e[idx])).Times(1);
	}
	// TRIGGER
	sc->clear(sc);
	// VERIFY
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
}

TEST_F(VectorContainer_Test, T10_ClearSparseContainer)
{
	const size_t sparseKey = 13;
	SetElement e = {sparseKey};

	ASSERT_EQ(&e, sc->insert(sc, &e));
	ASSERT_EQ(sparseKey, sc->size(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(1);

	// TRIGGER
	sc->clear(sc);
	// VERIFY
	EXPECT_EQ(0u, sc->size(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
}

TEST_F(VectorContainer_Test, T11_ClearContainerReduceCapacity)
{
	const size_t sparseKey = 6;
	SetElement e = {sparseKey};

	ASSERT_EQ(&e, sc->insert(sc, &e));
	ASSERT_EQ(sparseKey, sc->size(sc));
	ASSERT_EQ(sparseKey, sc->capacity(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e)).Times(1);

	// TRIGGER
	sc->clear(sc);
	// VERIFY
//	EXPECT_EQ(1, sc->capacity(sc)); TODO: uncomment if implemented
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
}


TEST_F(VectorContainer_Test, T12_EraseLastElement)
{
	SetElement e = {notSpecifiedElementKey};

	ASSERT_EQ(&e, sc->insert(sc, &e));
	ASSERT_EQ(1, sc->size(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e);
	// VERIFY
	EXPECT_EQ(0, sc->size(sc));
	EXPECT_EQ(1, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
}

TEST_F(VectorContainer_Test, T13_EraseLastElementSparseContainer)
{
	const size_t sparseKey = 6;
	SetElement e = {sparseKey};

	ASSERT_EQ(&e, sc->insert(sc, &e));
	ASSERT_EQ(sparseKey, sc->size(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e);
	// VERIFY
	EXPECT_EQ(0, sc->size(sc));
	EXPECT_EQ(1, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
}

TEST_F(VectorContainer_Test, T14_EraseOneElementOnBack)
{
	const size_t sparseKey = 10;
	const size_t numElem = 2;
	SetElement e[numElem] = {sparseKey, notSpecifiedElementKey};

	ASSERT_EQ(&e[0], sc->insert(sc, &e[0]));
	ASSERT_EQ(&e[1], sc->insert(sc, &e[1]));
	ASSERT_EQ(e[1].key, sc->size(sc));

	const auto prevCapacity = sc->capacity(sc);

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e[1]);
	// VERIFY
	EXPECT_EQ(sparseKey, sc->size(sc));
	EXPECT_EQ(prevCapacity, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[0])).Times(1);
}

TEST_F(VectorContainer_Test, T15_EraseOneElementOnBackSparseContainer)
{
	const size_t sparseKey1 = 5;
	const size_t sparseKey2 = 11;
	const size_t numElem = 2;
	SetElement e[numElem] = {sparseKey1, sparseKey2};

	ASSERT_EQ(&e[0], sc->insert(sc, &e[0]));
	ASSERT_EQ(&e[1], sc->insert(sc, &e[1]));
	ASSERT_EQ(e[1].key, sc->size(sc));

	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e[1]);
	// VERIFY
	EXPECT_EQ(sparseKey1, sc->size(sc));
	EXPECT_EQ(sparseKey1, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[0])).Times(1);
}

TEST_F(VectorContainer_Test, T16_EraseNotExistingElement)
{
	const size_t sparseKey0 = 5;
	const size_t sparseKey1 = 11;
	const size_t sparseKey2 = 16;
	const size_t numElem = 3;
	SetElement e[numElem] = {sparseKey0, sparseKey1, sparseKey2};

	ASSERT_EQ(&e[1], sc->insert(sc, &e[1]));
	ASSERT_EQ(sparseKey1, sc->size(sc));
	ASSERT_EQ(sparseKey1, sc->capacity(sc));

	/// Try to remove element not present in container ///
	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e[0]);
	// VERIFY
	EXPECT_EQ(sparseKey1, sc->size(sc));
	EXPECT_EQ(sparseKey1, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	/// Try to remove element with greater key than size ///
	// EXPECTATIONS
	EXPECT_CALL(*freeFun, free(_)).Times(0);
	// TRIGGER
	sc->erase(sc, &e[2]);
	// VERIFY
	EXPECT_EQ(sparseKey1, sc->size(sc));
	EXPECT_EQ(sparseKey1, sc->capacity(sc));
	verifyAndResetFreeFunMock();

	EXPECT_CALL(*freeFun, free(_)).Times(0);
	EXPECT_CALL(*freeFun, free(&e[1])).Times(1);
}
