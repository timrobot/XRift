#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "highgui.h"
#include "imgproc.h"
#include "ovr.h"
#include "gcube.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <signal.h>
#include "shmdata.h"
#include "color.h"
#include "gcube.h"
#include "gpu_util.h"

using namespace std;

void preprocessSHM(int *img, gcube &bgr);
void subplace(gcube &subimage, gcube &orig, int left, int top);

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
  shmdata *data = NULL;
  data = (shmdata *)shmat(shmid, data, shmflg);
  if (data == (shmdata *)-1) {
    print_error("[SINK] failed to attach");
    return 1;
  } else {
    print_debug("[SINK] found character stream");
  }
  // END SHM

  // Copy into gpu
  int *img;
  checkCudaErrors(cudaMalloc((void **)&img, SHMDATASIZE));

  /** this operation should do:
   *  1) restrict range
   *  2) convert the color
   *  3) possibly flip depending on if dk1 or dk2
   */

#if DK2
  gcube bgr(data->height, data->width - 1920, 3);
#else
  gcube bgr(data->height, data->width - 1280, 3); // removes the excess width from DK1
#endif

  /** get the subimages
   */
  size_t new_width = 500;
  size_t new_height = new_width * bgr.n_rows / bgr.n_cols;
  size_t crop = 20;//new_width / 20;
  size_t subimage_width = new_width - crop;
  size_t subimage_height = new_height;

#if DK2
  size_t xoffset = 960 - subimage_width;
  size_t yoffset = 540 - (subimage_height / 2) - 25;

  gcube limg(1080, 960, 3, gfill::zeros);
  gcube rimg(1080, 960, 3, gfill::zeros);
  gcube combined(1080, 1920, 3, gfill::zeros);

#else

  size_t xoffset = 640 - subimage_width;
  size_t yoffset = 400 - (subimage_height / 2) - 25; // hacked 25 extra offset up

  gcube limg(800, 640, 3, gfill::zeros);
  gcube rimg(800, 640, 3, gfill::zeros);
  gcube combined(800, 1280, 3, gfill::zeros);
#endif

  double offset = 0.15;

  while (!stopsig) {
    // copy to gpu over from the shm
    checkCudaErrors(cudaMemcpy(img, data->data, SHMDATASIZE, cudaMemcpyHostToDevice));

    // do triple operation
    preprocessSHM(img, bgr);

    // resize the gcube
    gcube resized = gpu_imresize2(bgr, new_height, new_width);

    // after resize, then place the resized image onto two subimages
    subplace(limg, resized, xoffset, yoffset);
    subplace(rimg, resized, -crop, yoffset);

    combined = ovr_image(limg, rimg, offset);
    cv::Mat out = combined.cv_img();
    cv::imshow("hud", out);
    if (cv::waitKey(30) >= 0) {
      continue;
    }

    // reset the cuda buffer
    checkCudaErrors(cudaMemset(limg.d_pixels, 0, limg.n_elem * sizeof(float)));
    checkCudaErrors(cudaMemset(rimg.d_pixels, 0, rimg.n_elem * sizeof(float)));
    checkCudaErrors(cudaMemset(combined.d_pixels, 0, combined.n_elem * sizeof(float)));
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
  bgr[IJK2C(i, j, 0, n_rows, dst_cols)] = (float)img[IJK2C(3-0-1, j, i, 4, src_cols)] / 255.0;
  bgr[IJK2C(i, j, 1, n_rows, dst_cols)] = (float)img[IJK2C(3-1-1, j, i, 4, src_cols)] / 255.0;
  bgr[IJK2C(i, j, 2, n_rows, dst_cols)] = (float)img[IJK2C(3-2-1, j, i, 4, src_cols)] / 255.0;
}

void preprocessSHM(int *img, gcube &bgr) {
  dim3 blockSize(16, 16, 1);
  dim3 gridSize((bgr.n_rows-1)/16+1, (bgr.n_cols-1)/16+1, 1); // range restricted, adds back excess for calculation
#if DK2
  size_t OVRCOLS = 1920;
#else
  size_t OVRCOLS = 1280;
#endif
  GPU_preprocess<<<gridSize, blockSize>>>(bgr.d_pixels, (uint8_t *)img, bgr.n_rows, bgr.n_cols + OVRCOLS, bgr.n_cols);
  checkCudaErrors(cudaGetLastError());
}

__global__ void GPU_subplace(float *dst, float *src, int left, int top, int dw, int dh, int sw, int sh) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
  if (top + i < 0 || top + i >= dh || left + j < 0 || left + j >= dw || i >= sh || j >= sw) {
    return;
  }
  dst[IJK2C(top + i, left + j, 0, dh, dw)] = src[IJK2C(i, j, 0, sh, sw)];
  dst[IJK2C(top + i, left + j, 1, dh, dw)] = src[IJK2C(i, j, 1, sh, sw)];
  dst[IJK2C(top + i, left + j, 2, dh, dw)] = src[IJK2C(i, j, 2, sh, sw)];
}

void subplace(gcube &subimage, gcube &orig, int left, int top) {
  dim3 blockSize(16, 16, 1);
  dim3 gridSize((orig.n_rows-1)/16+1, (orig.n_cols-1)/16+1, 1);
  GPU_subplace<<<gridSize, blockSize>>>(subimage.d_pixels, orig.d_pixels, left, top, subimage.n_cols, subimage.n_rows, orig.n_cols, orig.n_rows);
  checkCudaErrors(cudaGetLastError());
}
