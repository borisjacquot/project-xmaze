#include "libsockets.h"

#define PORT 1337

int main(){
	server_t serv = udpEcoute(PORT);
	printf("Le port de la partie choisie est : %d\n",serv.portTCP);
	return 0;
}
