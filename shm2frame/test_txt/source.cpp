#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "shmdata.h"
#include "color.h"

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
  } else {
    print_debug("[SOURCE] found character stream");
  }

  /** DO STUFF HERE **/

  data->data[0] = 14;
  data->data[1] = 32;
  sleep(1);
  data->data[0] = 15;
  data->data[1] = 35;
  sleep(1);
  data->data[0] = 16;
  data->data[1] = 36;
  sleep(1);
  data->data[0] = 17;
  data->data[1] = 37;
  sleep(1);
  data->data[0] = 18;
  data->data[1] = 38;
  sleep(10);

  /** END DO STUFF **/

  // detach the shm from this process
  if (shmdt(data) == -1) {
    print_error("[SOURCE] failed to detach");
  } else {
    print_debug("[SOURCE] finished");
  }

  // remove shmget (remember command in shell is: $ ipcrm -M [key]
  struct shmid_ds shmid_ds;
  if (shmctl(shmid, IPC_RMID, &shmid_ds) == -1) {
    print_error("[SOURCE] failed to remove shmid");
  } else {
    print_debug("[SOURCE] removed shmid");
  }

  return 0;
}
