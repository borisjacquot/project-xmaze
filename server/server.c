#include <stdint.h>
#include "common.h"
#include "libsockets.h"
#include "libthreads.h"

#define UDP_PORT "1337"

typedef struct {
  struct broadReturn br;
  balise_t b;
} beaconPack;

int keepRunning = 1;

void hand(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}

void beacon(void *pack){
    beaconPack *p=pack;
    struct broadReturn *br=&p->br;
    balise_t *b=&p->b;
    while(keepRunning) {
        sendBroadcast(br->sfd, br->broad, (void *)b, sizeof(balise_t));
        sleep(5);
    }
    close(br->sfd);
}

int main(void) {
    /* --- BROADCAST UDP --- */
    struct sigaction action = {0};

    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);

    beaconPack pack;

    pack.b.port = atoi("1330");
    strcpy(pack.b.name, "Xmazing");

    pack.br = setBroadcast(UDP_PORT);
    //beacon(&pack);
    /* envoi du message */
    launchThread(beacon,&pack,sizeof(pack));
    /* --- FIN BROADCAST UDP --- */

    printf("\nBelle journée\n");
    return 0;
}
