#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define PORT 1337

int keepRunning = 1;

void hand(int sig){
	if(sig==SIGINT){
		keepRunning=0;
	}
}

/* Fonction d'ecoute du broadcast UDP sur le port 1337 */
int udpEcoute(){
	struct sockaddr_in mysocket, other_socket;
	int s;
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int broadcast=1;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		    &broadcast, sizeof broadcast);

	memset(&mysocket, 0, sizeof(mysocket));
	mysocket.sin_family = AF_INET;
	mysocket.sin_port = htons(PORT);
	mysocket.sin_addr.s_addr = INADDR_ANY;

	bind(s, (struct sockaddr *)&mysocket, sizeof(struct sockaddr));
	printf("Lecture du port %d ...\n",PORT);
	int game_port;
	
	/* === A MODIFIER L'ARRET DE LA BOUCLE DE RECEPTION === */
	struct sigaction action;
	action.sa_handler=hand;
	sigaction(SIGINT,&action,NULL);
	/* =================================================== */

	char tab[50][50];
	int k=0;
	while(keepRunning){
		char buffer[50];
		memset(buffer,0,sizeof(buffer));
		unsigned sock_len=sizeof(struct sockaddr);
		recvfrom(s, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&other_socket, &sock_len);
		int flag=0;
		if(k!=0){
			for(int i=0; i<=k;i++){
				if(strcmp(buffer,tab[i])==0){
					flag=1;
				}
			}
		}
		if(flag==0){
			strcpy(tab[k],buffer);
			k++;	
			game_port = buffer[0] + (buffer[1]<<8);	
			for(long unsigned int i=0;i<sizeof(buffer)-1;i++){
				buffer[i]=buffer[i+2];
			}
			printf("%d) La partie de jeu %s est sur le port : %d\n",k,buffer,game_port);
		}
	}
	close(s);
	printf("Entrer le numero du serveur souhaite : ");
	int choix;
	scanf("%d",&choix);
	game_port = tab[choix-1][0]+(tab[choix-1][1]<<8);
	return game_port;
}

int main(){
	int game_port = udpEcoute();
	printf("Le port de la partie est : %d\n",game_port);
	return 0;
}
