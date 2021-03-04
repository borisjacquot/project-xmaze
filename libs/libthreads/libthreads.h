#ifndef LIBTHREADS_H
#define LIBTHREADS_H

/* INCLUDES  */
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
/* DEFINE  */

/* STRUCTURES */
typedef struct {
  void (*function)(void *);
  void *argument;
} _internal_type;

/* PROTOTYPES */
static void *_launchThread(void *arg);
void launchThread(void (*function)(void *),void *argument,int size);

#endif //LIBTHREADS_H
