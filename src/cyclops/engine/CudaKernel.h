#include <vector>

template <class T>
class CudaKernel {

public:

    // Allocate device arrays
    float* d_in;
    float* d_out;
    
    // Allocate temporary storage
    void *d_temp_storage = NULL;
    size_t temp_storage_bytes = 0;

    CudaKernel(T* h_in, int num_items);
    ~CudaKernel();

    void CubScanMalloc(int num_items);
    void CubScan(int num_items);
};