QCS ?= qcs
SM_VER ?= $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader | awk '{print $$1*10;}')
CXX = g++
NVCC = nvcc
NVCCFLAGS = $(shell ./get_nvccopts.sh) -Xcompiler -Wformat=2 -I./include -lcurand -lnccl -lssl -lcrypto --cudart=shared -g -O3 -Xcompiler -fopenmp -Xlinker --export-dynamic -std=c++11 -rdc=true -Wno-deprecated-gpu-targets -gencode=arch=compute_$(SM_VER),code=sm_$(SM_VER)
OBJDIR = obj
QCS_O ?= $(OBJDIR)/qcs.o

.PHONY: all
all: userqasm.so main

userqasm.so: src/userqasm_ghz.cpp include/qasm/qasm.hpp
	$(CXX) -I./include -fPIC -shared -std=c++11 $< -o $@

$(OBJDIR)/main.o: src/main.cpp include/qasm/qasm.hpp qcs/include/qcs/qcs.hpp
	$(CXX) -c -I./include -I./qcs/include/ -std=c++11 $< -o $@

$(OBJDIR)/qasm.o: src/qasm.cpp include/qasm/qasm.hpp $(QCS)/include/qcs/qcs.hpp
	$(CXX) -c -I./include -I./qcs/include -std=c++11 $< -o $@

$(OBJDIR)/qcs.o: qcs/src/qcs.cpp qcs/include/qcs/qcs.hpp
	$(CXX) -c -I./include -I./qcs/include/ -std=c++11 $< -o $@

main: $(OBJDIR)/main.o $(OBJDIR)/qasm.o $(QCS_O)
	$(NVCC) $(NVCCFLAGS) $^ -o $@


.PHONY: run
run: all
	./main

.PHONY: clean
clean:
	$(RM) main userqasm.so $(OBJDIR)/main.o $(OBJDIR)/qasm.o $(OBJDIR)/qcs.o
