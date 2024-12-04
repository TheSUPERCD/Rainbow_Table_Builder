all: make make gpu filecomp

make:
	g++ -Wall -O3 ./src/sha3_cpu.cpp ./src/util_funcs.cpp ./src/main.cpp -o binary -fopenmp

gpu:
	nvcc -O3 ./src/sha3_gpu.cu ./src/util_funcs.cpp ./src/main_cuda.cpp -o binary_cuda -Xcompiler -Wall

static:
	nvcc -O3 ./src/sha3_gpu.cu ./src/util_funcs.cpp ./src/main_cuda.cpp -o binary_cuda_static -cudart=static -Xcompiler -Wall

dataset:
	python3 ./UUID_Generator_Script/uuid_generator.py

filecomp:
	g++ -Wall -O3 ./src/file_comparator.cpp -o filecomp

clean:
	rm -f binary binary_cuda binary_cuda_static filecomp
