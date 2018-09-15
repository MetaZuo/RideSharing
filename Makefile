GUROBI = /Library/gurobi801/mac64
INC = $(GUROBI)/include
CPPLIB = -L$(GUROBI)/lib -lgurobi_c++ -lgurobi80
all:
	g++ -o Main Main.cpp -O3 -l metis -I$(INC) $(CPPLIB)
