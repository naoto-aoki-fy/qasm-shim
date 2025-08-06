QCS ?= qcs

.PHONY: all
all: userqasm.so main

userqasm.so: src/userqasm.cpp include/qasm/qasm.hpp $(QCS)/include/qcs/qcs.hpp
	$(CXX) -I./include -I./qcs/include -fPIC -shared -std=c++11 $< -o $@

main: src/main.cpp $(QCS)/src/qcs.cpp src/qasm.cpp include/qasm/qasm.hpp $(QCS)/include/qcs/qcs.hpp
	$(CXX) -I./include -I./qcs/include -std=c++11 -rdynamic src/main.cpp $(QCS)/src/qcs.cpp src/qasm.cpp -o main

.PHONY: run
run: all
	./main

.PHONY: clean
clean:
	$(RM) main userqasm.so
