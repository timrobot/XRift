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

int main(int argc, char *argv[]) {
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

  /** DO STUFF HERE **/

  Mat img(data->width, data->height, CV_8UC3);
  namedWindow("sink_test", CV_WINDOW_AUTOSIZE);

  while (true) {
    img.rows = data->height;
    img.cols = data->width;
    img.data = data->data;
    memcpy(data->data, img.data, SHMDATASIZE);
    imshow("sink_test", img);
    waitKey(30);
  }

  /** END DO STUFF **/

  // detach the shm from this process
  if (shmdt(data) == -1) {
    print_error("[SINK] failed to detach");
    return 1;
  } else {
    print_debug("[SINK] finished");
  }

  return 0;
}
