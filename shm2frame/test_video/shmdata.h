#ifndef __shmdata_h__
#define __shmdata_h__

#ifndef __cplusplus
#include <stdint.h>
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif

#define SHMDATASIZE (1080 * 1920 * 3) // this is the video size [HACK]

typedef struct {
  int width;
  int height;
  uint8_t data[SHMDATASIZE];
} shmdata;

#ifdef __cplusplus
}
#endif

#endif
