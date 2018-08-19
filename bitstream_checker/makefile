UNAME := $(shell uname)

CPPFLAGS += 
CXXFLAGS += -Wall -Wextra -std=gnu++11
# -g

CC := gcc
CFLAGS += -Wall -Wextra -std=gnu11


APPL_TRGT := bitstream_checker
APPL_SRCS := main.c
APPL_OBJS := $(APPL_SRCS:%.cpp=%.o)
APPL_OBJS := $(APPL_OBJS:%.c=%.o)

SRCS := bitStreamChecker.c \
		tripleBitStreamChecker.c \
		vectorContainer.c \
		trivialBitStreamChecker.c
OBJS := $(SRCS:%.c=%.o)
OBJS := $(OBJS:%.cpp=%.o)

TEST_TRGT := utest
TEST_SRCS := bitStreamChecker_test.cpp \
			 setContainer_test.cpp \
			 trivialBitStreamChecker_test.cpp
TEST_OBJS := $(TEST_SRCS:%.cpp=%.o)
TEST_OBJS := $(TEST_OBJS:%.c=%.o)

RM := rm -rfv


all :  test appl


%.o :  %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $^

%.o :  %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $^


$(TEST_TRGT) :  $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -pthread -lgmock_main -o $@  $^

test :  $(TEST_TRGT)
test-run :  test
	./$(TEST_TRGT)
test-clean :
	$(RM)  $(TEST_TRGT)  $(TEST_OBJS)



$(APPL_TRGT) :  $(APPL_OBJS) $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

appl :  $(APPL_TRGT)
appl-run :  appl
	./$(APPL_TRGT)
appl-clean :
	$(RM)  $(APPL_TRGT)  $(APPL_OBJS)  $(OBJS)

clean :  test-clean appl-clean
ifeq ($(UNAME), Linux)
	$(RM) *.o 
else
	$(RM) *.o {$(APPL_TRGT),$(TEST_TRGT)}.exe
endif
