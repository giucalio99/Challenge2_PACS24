CXX      ?= g++
CXXFLAGS ?= -std=c++20
#the only thing you need to change is the location of json include in your computer
CPPFLAGS ?= -O3 -Wall -I./include -Wno-conversion-null -Wno-deprecated-declarations 
SOURCE_DIR=./src/
VPATH=$(SOURCE_DIR)
SRCS=$(join $(dir $(SOURCE_DIR)),$(notdir *.cpp))
OBJS= $(SRCS:%.cpp=%.o)

EXEC= main
.phony= clean 
.DEFAULT_GOAL = all
all: $(EXEC)

$(OBJS): $(SRCS) 
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(SRCS)
	mv *.o ./src
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC)

clean:
	$(RM) *.o
	$(RM) $(OBJS)
	$(RM) $(EXEC)
	$(RM) -r ./doc/html ./doc/latex
