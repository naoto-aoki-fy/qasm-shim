.PHONY: all
all: userqasm.so main

userqasm.so: userqasm.cpp qasm.hpp qcs.hpp
	$(CXX) -fPIC -shared -std=c++11 userqasm.cpp -o userqasm.so

main: main.cpp qcs.cpp qasm.cpp qasm.hpp qcs.hpp
	$(CXX) -std=c++11 -rdynamic main.cpp qcs.cpp qasm.cpp -o main

.PHONY: run
run: all
	./main

.PHONY: clean
clean:
	$(RM) main userqasm.so
