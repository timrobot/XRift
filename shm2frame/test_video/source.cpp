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
  shmflg = 0666 | IPC_CREAT;

  // get an shm
  if ((shmid = shmget(key, size, shmflg)) == -1) {
    print_error("[SOURCE] shmget failed, trying again");
    struct shmid_ds sds;
    shmctl(shmid, IPC_RMID, &sds);
    if ((shmid = shmget(key, size, shmflg)) == -1) {
      print_error("[SOURCE] shmget failed again, failing");
      return 1;
    } else {
      print_debug("[SOURCE] Got an shm!");
    }
  } else {
    print_debug("[SOURCE] Got an shm!");
  }

  // attach the shm to this process
  shmdata *data = (shmdata *)shmat(shmid, data, shmflg);
  if (data == (shmdata *)-1) {
    print_error("[SOURCE] failed to attach");
    return 1;
  } else {
    print_debug("[SOURCE] found character stream");
  }

  /** DO STUFF HERE **/

  VideoCapture cam(0);
  if (cam.isOpened()) {
    print_debug("Camera opened!");
  } else {
    print_error("Camera not found");
    return 1;
  }

  Mat img;
  //namedWindow("source_test", CV_WINDOW_AUTOSIZE);
  
  data->width = 640;
  data->height = 480;

  while (true) {
    cam >> img;
    // huge operation
    memcpy(data->data, img.data, SHMDATASIZE);
    //printf("%d %d %d\n", img.rows, img.cols, img.channels());
    //imshow("source_test", img);
    //waitKey(30);
  }

  /** END DO STUFF **/

  // detach the shm from this process
  if (shmdt(data) == -1) {
    print_error("[SOURCE] failed to detach");
    return 1;
  } else {
    print_debug("[SOURCE] finished");
  }

  // remove shmget (remember command in shell is: $ ipcrm -M [key]
  struct shmid_ds shmid_ds;
  if (shmctl(shmid, IPC_RMID, &shmid_ds) == -1) {
    print_error("[SOURCE] failed to remove shmid");
    return 1;
  } else {
    print_debug("[SOURCE] removed shmid");
  }

  return 0;
}
