.Phony: all
all: userqasm.so main

userqasm.so: userqasm.cpp qasm.hpp qcs.hpp math.hpp math_type.hpp
	$(CXX) -fPIC -shared -std=c++11 userqasm.cpp -o userqasm.so

main: main.cpp qcs.cpp qasm.hpp qcs.hpp math.hpp math_type.hpp
	$(CXX) -std=c++11 -rdynamic main.cpp qcs.cpp -o main

.Phony: run
run: all
	./main


.Phony: clean
clean:
	$(RM) main userqasm.so