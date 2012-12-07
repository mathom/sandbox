BINARY=sandbox
SOURCE=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))

CXXFLAGS=`fltk-config --cxxflags` -I/usr/local/include
LDFLAGS=`fltk-config --use-gl --ldstaticflags` -lchipmunk

CXX=g++

all: $(BINARY)

$(BINARY): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^ 

clean:
	$(RM) $(OBJECTS) $(BINARY) 
