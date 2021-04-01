#include "libthreads.h"

static void *_launchThread(void *arg){
  _internal_type *it=arg;
  it->function(it->argument);
  if (it->argument != NULL)
    free(it->argument);
  free(it);
  return NULL;
}

void launchThread(void (*function)(void *),void *argument,int size){
  _internal_type *arg;
  arg=malloc(sizeof(_internal_type));
  if (argument != NULL) {
    arg->argument=malloc(size);
    memcpy(arg->argument,argument,size);
  } else {
    arg->argument=NULL;
  }
  arg->function=function;
  pthread_t tid;
  pthread_create(&tid, NULL, _launchThread, (void *)arg);
  pthread_detach(tid);
}
