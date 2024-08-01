CXX = g++
CXXFLAGS = -std=c++11 -Wall


SRCS = main.cpp CellLibraryParser.cpp NetlistParser.cpp GateMapper.cpp NetlistWriter.cpp Optimizer.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = netlist_optimizer

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
