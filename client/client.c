#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
	struct sockaddr_in si_me, si_other;
	int s;
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int port=1337;
	int broadcast=1;

	setsockopt(s, SOL_SOCKET, SO_BROADCAST,
		    &broadcast, sizeof broadcast);

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = INADDR_ANY;

	bind(s, (struct sockaddr *)&si_me, sizeof(struct sockaddr));

	while(1){
		printf("Lecture du port %d ...\n");
	    char buf[10000];
	    unsigned slen=sizeof(struct sockaddr);
	    recvfrom(s, buf, sizeof(buf)-1, 0, (struct sockaddr *)&si_other, &slen);

	    printf("recv: %s\n", buf);
	}
}
