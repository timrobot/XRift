#ifndef __shmdata_h__
#define __shmdata_h__

#ifndef __cplusplus
#include <stdint.h>
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif

#define DK2 1

#if DK2
#define SHMDATASIZE ((1920 + 1920) * 1080 * 4)
#else
#define SHMDATASIZE ((1920 + 1280) * 1080 * 4) // this is the video size [HACK]
#endif

typedef struct {
  int width;
  int height;
  uint8_t data[SHMDATASIZE];
} shmdata;

#ifdef __cplusplus
}
#endif

#endif
