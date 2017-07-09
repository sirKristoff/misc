
CPPFLAGS += 
CXXFLAGS += -g -Wall -Wextra

TEST_TRGT := utest
TEST_SRCS := utest.cpp
TEST_OBJS := $(TEST_SRCS:%.cpp=%.o)

MOCK_TRGT := mtest
MOCK_SRCS := mtest.cpp
MOCK_OBJS := $(MOCK_SRCS:%.cpp=%.o)


RM := rm -rf


test :  $(TEST_TRGT)
mock : $(MOCK_TRGT)

test-run :  test
	./$(TEST_TRGT)
mock-run :  mock
	./$(MOCK_TRGT)

clean :
	$(RM) *.o *.exe $(TEST_TRGT) $(MOCK_TRGT)

%.o :  %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<


$(TEST_TRGT) :  $(TEST_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread -lgtest_main -o $@  $^

$(MOCK_TRGT) :  $(MOCK_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pthread -lgmock_main -o $@  $^
