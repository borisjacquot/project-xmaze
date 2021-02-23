#include "libthreads.h"

typedef struct {
  void (*function)(void *);
  void *argument;
} _internal_type;

static void *_launchThread(void *arg){
  _internal_type *it=arg;
  it->function(it->argument);
  free(it->argument);
  free(it);
  return NULL;
}

void launchThread(void (*function)(void *),void *argument,int size){
  _internal_type *arg;
  arg=malloc(sizeof(_internal_type));
  arg->argument=malloc(size);
  arg->function=function;
  memcpy(arg->argument,argument,size);
  pthread_t tid;
  pthread_create(&tid, NULL, _launchThread, (void *)arg);
  pthread_detach(tid);
}
