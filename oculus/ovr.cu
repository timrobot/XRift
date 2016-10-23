#include "highgui.h"
#include "imgproc.h"
#include "gpu_util.h"

//const float u_distortion[4] = { 1.0, -0.22, -0.24, 0 };
// preprogrammed constants
#define UD0 1.0
#define UD1 0.22
#define UD2 0.24

// This GPU function is used inside the CPU function ovr_image(const gcube &, const gcube &, double)
__global__ void barrel_distort_ovr(float *G, float *F,
    int n_rows, int n_cols, int n_slices,
    float r_x, float r_y, float r_max, float offset_x, int right_image) {

  /** This kernel is like a GPU-parallelized for-loop:

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        // GPU code here
      }
    }

  */

  int j = blockIdx.x * blockDim.x + threadIdx.x;
  int i = blockIdx.y * blockDim.y + threadIdx.y;

  // Everything below is like the block of code inside a parallelized GPU nested for-loop

  if (i >= n_rows || j >= n_cols) {
    return;
  }
  float x = (float)(j-r_x) / r_max + offset_x;
  float y = (float)(i-r_y) / r_max;
  // distortionScale
  float rr = x*x+y*y;
  float distortion = UD0+UD1*rr+UD2*rr*rr;
  int _i = (int)roundf(distortion*y*r_max+r_y);
  int _j = (int)roundf((distortion*x-offset_x)*r_max+r_x);
  for (int k = 0; k < n_slices; k++) {
    // since they are stored right next to each other, the slices are the separating factor (k*2+right_image)
    if (_i >= 0 && _i < n_rows && _j >= 0 && _j < n_cols) {
      G[IJK2C(i, j, k*2+right_image, n_rows, n_cols)] = F[IJK2C(_i, _j, k, n_rows, n_cols)];
    }
  }
}

/** This function takes in 2 gcubes (a GPU-enabled image), and outputs another gcube which is the oculus-transformed image
 *  @param left the left image
 *  @param right the right image
 *  @param offset_x the calibration constant for the offset of the rift headset, manually inputted (try 0.15)
 *  @return a new image (oculus-displayable)
 */
gcube ovr_image(const gcube &left, const gcube &right, double offset_x) {
  // assume that the left and right images have equal dimensions
  assert(left.n_rows == right.n_rows && left.n_cols == right.n_cols);

  // find the middle of the left image's width + (1 + |offset_x|)
  double mrx = left.n_cols / 2.0 * (1 + abs(offset_x));

  // find the middle of the right image's width
  double mry = right.n_cols / 2.0;

  // find the distortion radius
  float r_max = (float)sqrt(mrx * mrx + mry * mry);

  // find the middle point for either image
  float r_x = left.n_cols / 2.0f;
  float r_y = left.n_rows / 2.0f;

  // define the dimensions of the blockSize (GPU-only)
  dim3 blockSize(16, 16, 1);

  // define the dimensions of the gridSize (GPU-only)
  dim3 gridSize((left.n_cols-1)/16+1, (left.n_rows-1)/16+1, 1);

  // create a <dst> GCube (this is a custom class that I defined somewhere
  // with height = left.height, width = left.width + right.width, channels = left.n_channels, filled with 0s
  gcube C(left.n_rows, left.n_cols + right.n_cols, left.n_slices, gfill::zeros);

  // use the GPU to barrel distort the source images (left and right) into the destination (gcube C)
  barrel_distort_ovr<<<gridSize, blockSize>>>(C.d_pixels, left.d_pixels,
      left.n_rows, left.n_cols, left.n_slices,
      r_x, r_y, r_max, -offset_x, 0);
  checkCudaErrors(cudaGetLastError());
  barrel_distort_ovr<<<gridSize, blockSize>>>(C.d_pixels, right.d_pixels,
      right.n_rows, right.n_cols, right.n_slices,
      r_x, r_y, r_max, offset_x, 1);
  checkCudaErrors(cudaGetLastError());

  // resize the image just in case and return
  return gpu_imresize2(C, 800, 1280); // note: resizing is SUPER SLOW
}
