CXXFLAGS += -O3 

%.o : %.cpp *.h
	$(CXX) $(CXXFLAGS) $< -c -o $@;
	
.PHONY: all
all: fusion

fusion: ref.o 
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f ref.o fusion
