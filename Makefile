# Put [program].hh [program].cc test_[program].cc triplet names here:
PROGRAMS=cities
# The makefile will automatically handle things, more or less.
# If your program has dependencies, do something like this:
#hforest: htree.o

ALLOBJS = test_generic.o $(addsuffix .o,$(PROGRAMS)) $(addprefix test_,$(addsuffix .o,$(PROGRAMS))) catch_main.o

CFLAGS = -g -Wall -Wextra -pedantic #-Werror
CXXFLAGS = -std=c++17

all:	cities

$(PROGRAMS) : % : test_generic.o test_%.o %.o catch_main.o
	$(CXX) -o $@ $^

test:	$(OBJSALL)
	$(CXX) -o $@ $^

%.o:	%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	%.cc
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr $(PROGRAMS) $(OBJSALL) test
