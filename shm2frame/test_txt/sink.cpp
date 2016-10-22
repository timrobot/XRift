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
  } else {
    print_debug("[SINK] found character stream");
  }

  /** DO STUFF HERE **/

  while (true) {
    uint8_t d0 = data->data[0];
    uint8_t d1 = data->data[1];
    printf("[SINK] found data: %d %d\n", d0, d1);
    sleep(1);
  }

  /** END DO STUFF **/

  // detach the shm from this process
  if (shmdt(data) == -1) {
    print_error("[SINK] failed to detach");
  } else {
    print_debug("[SINK] finished");
  }

  return 0;
}
