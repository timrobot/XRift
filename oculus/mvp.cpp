#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "highgui.h"
#include "ovr.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "shmdata.h"
#include "color.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv; // this might break something

int main(int argc, const char** argv){
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
  // END COPYING

  Mat img(data->width, data->height, CV_8UC4);

  int height = img.rows;
  int width = img.cols;
  int crop = width/20;

  printf("Image dimensions: height=%d width=%d crop=%d\n", height, width, crop);

  while(true){

    img.rows = data->height;
    img.cols = data->width;
    img.data = data->data;
    memcpy(data->data, img.data, SHMDATASIZE);

    cv::Mat subImage1(img, cv::Range::all(), cv::Range(0, width-crop)); // Mat dst(Mat src, yrange, xrange)
    cv::Mat subImage2(img, cv::Range::all(), cv::Range(crop, width));

    // find the new dimensions to match up to the oculus rift ratio
    int new_width = 500; // 9 / 10 of the original width
    int new_height = new_width * subImage1.rows / subImage1.cols;

    // resize both images
    cv::Mat resizedLeft, resizedRight;
    cv::resize(subImage1, resizedLeft, cv::Size(new_width, new_height));
    cv::resize(subImage2, resizedRight, cv::Size(new_width, new_height));

    // create the left image
    cv::Mat leftImage(800, 640, CV_8UC3);
    int xoffset = 640 - new_width;          // width = 1280
    int yoffset = 400 - (new_height / 2);   // height = 800

    resizedLeft.copyTo(leftImage(cv::Rect(xoffset, yoffset, new_width, new_height)));

    // create the right image
    cv::Mat rightImage(800, 640, CV_8UC3);
    resizedRight.copyTo(rightImage(cv::Rect(0, yoffset, new_width, new_height)));

    arma::cube limg = cvt_opencv2arma(leftImage) / 255.0;
    arma::cube rimg = cvt_opencv2arma(rightImage) / 255.0;
    arma::cube combined;

    double offset = 0.15;

    combined = ovr_image(limg, rimg, offset);
    disp_image("hud", combined);

    if(disp_keyPressed() >= 0)
    {
      continue;
    }
  }

  // for(;;){
  //   combined = ovr_image(limg, rimg, offset);
  //   disp_image("hud", combined);
  //   if(disp_keyPressed() >= 0)
  //   {
  //     break;
  //   }
  // }
  return 0;
}

