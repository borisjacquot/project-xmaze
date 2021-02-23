#include <stdint.h>
#include "common.h"
#include "libsockets.h"

#define UDP_PORT "1337"

int keepRunning = 1;

void hand(int sig) {
    if (sig == SIGINT) {
        keepRunning = 0;
    }
}

int main(void) {
    /* --- BROADCAST UDP --- */
    struct sigaction action = {0};
    struct broadReturn br;
    balise_t balise;

    balise.port = atoi("1330");
    strcpy(balise.name, "Xmazing");

    br = setBroadcast(UDP_PORT);

    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);

    /* envoi du message */
    while(keepRunning) {
        sendBroadcast(br.sfd, br.broad, (void *) &balise, sizeof(balise));
        sleep(1);
    }
    close(br.sfd);
    /* --- FIN BROADCAST UDP --- */

    printf("Belle journée\n");
    return 0;
}
