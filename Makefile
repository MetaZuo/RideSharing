GUROBI = /Library/gurobi801/mac64
INC = $(GUROBI)/include
CPPLIB = -L$(GUROBI)/lib -lgurobi_c++ -lgurobi80
all:
	g++ -o Main Main.cpp -l metis -I$(INC) $(CPPLIB)
