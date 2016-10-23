#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "highgui.h"
#include "ovr.h"
#include "gcube.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "shmdata.h"
#include "color.h"
#include "gcube.h"
//#include "gpu_util.h"

using namespace std;

void preprocessSHM(int *img, gcube &bgr);
void subplace(gcube &subimage, gcube &orig, int left, int top, int width, int height);

static int stopsig;
void stopme(int signo) {
  stopsig = 1;
}

int main(int argc, const char *argv[]) {
  signal(SIGINT, stopme);

  key_t key;
  int shmflg;
  int shmid;
  int size;

  key = 9000;
  size = sizeof(shmdata);
  shmflg = 0666;

  // get an shm
  if ((shmid = shmget(key, size, shmflg)) == -1) {
    print_error("[SINK] shmget failed");
    return 1;
  } else {
    print_debug("[SINK] Got an shm!");
  }

  // attach the shm to this process
  shmdata *data = (shmdata *)shmat(shmid, data, shmflg);
  if (data == (shmdata *)-1) {
    print_error("[SINK] failed to attach");
    return 1;
  } else {
    print_debug("[SINK] found character stream");
  }
  // END SHM

  // Copy into gpu
  int *img;
  checkCudaErrors(cudaMalloc(&img, SHMDATASIZE));

  /** this operation should do:
   *  1) restrict range
   *  2) convert the color
   *  3) possibly flip depending on if dk1 or dk2
   */
  gcube bgr(data->height, data->width - 1280, 3); // removes the excess width from DK1

  /** get the subimages
   */
  size_t new_width = 500;
  size_t new_height = new_width * bgr.n_rows / bgr.n_cols;
  size_t crop = new_width / 20;
  size_t subimage_width = new_width - crop;
  size_t subimage_height = new_height;

  size_t xoffset = 640 - subimage_width;
  size_t yoffset = 400 - (subimage_height / 2);

  gcube limg(800, 640, 3, fill::zeros);
  gcube rimg(800, 640, 3, fill::zeros);
  gcube combined(800, 1280, 3, fill::zeros);

  while (!stopsig) {
    // copy to gpu over from the shm
    checkCudaErrors(cudaMemcpy(img.d_pixels, data->data, SHMDATASIZE, cudaMemcpyHostToDevice));

    // do triple operation
    preprocessSHM(img, bgr);

    // resize the gcube
    gcube resized = gpu_imresize2(bgr, new_width, new_height);

    // after resize, then place the resized image onto two subimages
    subplace(limg, resized, xoffset, yoffset, subimage_width, subimage_height);
    subplace(rimg, resized, 0, yoffset, subimage_width, subimage_height);

    combined = ovr_image(limg, rimg, offset);
    cv::Mat out = combined.cv_img();
    cv::imshow("hud", out);
    if (cv::waitKey(30) >- 0) {
      continue;
    }
  }

  return 0;
}

__global__ void GPU_preprocess(float *bgr, uint8_t *img, int n_rows, int src_cols, int dst_cols) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
  if (i >= n_rows || j >= dst_cols) {
    return;
  }
  // row-wise
  int src_idx = 4 * (i * src_cols + j);
  int dst_idx = 3 * (i * dst_cols + j);
  bgr[dst_idx + 0] = float(img[src_idx + 0]) / 255.0;
  bgr[dst_idx + 1] = float(img[src_idx + 1]) / 255.0;
  bgr[dst_idx + 2] = float(img[src_idx + 2]) / 255.0;
}

void preprocessSHM(int *img, gcube &bgr) {
  dim3 blockSize(16, 16, 1);
  dim3 gridSize((bgr.n_rows-1)/16+1, (bgr.n_cols-1)/16+1, 1); // range restricted
  GPU_preprocess<<<gridSize, blockSize>>>(bgr.d_pixels, (uint8_t *)img, bgr.n_rows, bgr.n_cols, bgr.n_cols - 1280);
  checkCudaErrors(cudaGetLastErrror());
}

__global__ void GPU_subplace(float *dst, float *src, int left, int top, int sw, int sh, int width, int height) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
  if (i >= sh || j >= sw) {
    return;
  }
  // row-wise
  int src_idx = 3 * (i * width + j);
  int dst_idx = 3 * ((top + i) * sw + (left + j));
  dst[dst_idx + 0] = src[src_idx + 0];
  dst[dst_idx + 1] = src[src_idx + 1];
  dst[dst_idx + 2] = src[src_idx + 2];
}

void subplace(gcube &subimage, gcube &orig, int left, int top, int width, int height) {
  dim3 blockSize(16, 16, 1);
  dim3 gridSize((height-1)/16+1, (width-1)/16+1, 1);
  GPU_subplace<<<gridSize, blockSize>>>(subimage.d_pixels, orig.d_pixels, left, top, subimage.n_cols, subimage.n_rows, width, height);
  checkCudaErrors(cudaGetLastError());
}
