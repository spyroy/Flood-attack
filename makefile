
all: Q4 Q5
	
Q4: tcp_udp_flood_ipv4.o
	gcc tcp_udp_flood_ipv4.o -o Q4 
	
Q5: udp_ipv6.o
	gcc -pthread udp_ipv6.o -o Q5
	
tcp_udp_flood_ipv4.o: tcp_udp_flood_ipv4.c
	gcc -c tcp_udp_flood_ipv4.c
	
udp_flood_ipv6.o: udp_ipv6.c
	gcc -c udp_ipv6.c


clean:
	rm *.o Q4 Q5
