CPPFLAGS += 
CXXFLAGS += -Wall -Wextra -std=gnu++11
# -g -std=c++11

APPL_TRGT := fileConverter
APPL_SRCS := main.cpp
APPL_OBJS := $(APPL_SRCS:%.cpp=%.o)

#SRCS := module.cpp
OBJS := $(SRCS:%.cpp=%.o)

TEST_TRGT := utest
TEST_SRCS := iostream_test.cpp
TEST_OBJS := $(TEST_SRCS:%.cpp=%.o)

DOXY_FILE := doc.doxyfile
DOXY_DIR := doc/

RM := rm -rfv
DOXYGEN ?= doxygen

VALGRIND ?= valgrind


all :  test appl doc


%.o :  %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $^



$(TEST_TRGT) :  $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -pthread -lgmock_main -o $@  $^

test :  $(TEST_TRGT)
test-valgrind :  test
	$(VALGRIND) ./$(TEST_TRGT)
test-run :  test
	./$(TEST_TRGT)
test-clean :
	$(RM)  $(TEST_TRGT)  $(TEST_OBJS)



$(APPL_TRGT) :  $(APPL_OBJS) $(OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

appl :  $(APPL_TRGT)
appl-run :  appl
	./$(APPL_TRGT)
appl-clean :
	$(RM)  $(APPL_TRGT)  $(APPL_OBJS)  $(OBJS)



doc :
	$(DOXYGEN)  $(DOXY_FILE)
doc-clean :
	$(RM)  $(DOXY_DIR)



clean :  test-clean appl-clean doc-clean
	$(RM) *.o *.exe

