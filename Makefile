QCS ?= qcs
QCS_ABS = $(shell realpath -P $(QCS))
SM_VER ?= $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader | awk '{print $$1*10;}')
CXX = g++
OBJDIR = obj
QCS_LIB = $(QCS_ABS)/lib


.PHONY: all
all: userqasm.so main

userqasm.so: src/userqasm_ghz.cpp include/qasm/qasm.hpp
	$(CXX) -I./include -fPIC -shared -std=c++11 $< -o $@

$(OBJDIR)/main.o: src/main.cpp include/qasm/qasm.hpp qcs/include/qcs/qcs.hpp
	$(CXX) -c -I./include -I./qcs/include/ -std=c++11 $< -o $@

$(OBJDIR)/qasm.o: src/qasm.cpp include/qasm/qasm.hpp qcs/include/qcs/qcs.hpp
	$(CXX) -c -I./include -I./qcs/include -std=c++11 $< -o $@

qcs/lib/libqcs.so: qcs/src/qcs.cpp qcs/include/qcs/qcs.hpp
	$(CXX) -fPIC -shared -I./include -I./qcs/include/ -std=c++11 $< -o $@

main: $(OBJDIR)/main.o $(OBJDIR)/qasm.o $(QCS_LIB)/libqcs.so
	$(CXX) -Wformat=2 -I./include -rdynamic -std=c++11 -Wl,-rpath,$(QCS_LIB) -L$(QCS_LIB) -lqcs $(word 1, $^) $(word 2, $^) -o $@


.PHONY: run
run: all
	./main

.PHONY: clean
clean:
	$(RM) main userqasm.so $(OBJDIR)/main.o $(OBJDIR)/qasm.o $(OBJDIR)/qcs.o
