#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <iostream>
#include <cstdlib>
#define checkCudaErrors(val) __check( (val), #val, __FILE__, __LINE__)

template<typename T>
void __check(T err, const char* const func, const char* const file, const int line) {
  if (err != cudaSuccess) {
    std::cerr << "CUDA error at: " << file << ":" << line << std::endl;
    std::cerr << cudaGetErrorString(err) << " " << func << std::endl;
    exit(1);
  }
}

int main() {
	int *data;
	checkCudaErrors(cudaMalloc((void **)&data, 4));
	return 0;
}
