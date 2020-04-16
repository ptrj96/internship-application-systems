/*
Because of the use of Raw sockets this program must be 
run with root (sudo) user priveleges.
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PING_PKT_SIZE  64

struct packet
{
    struct icmphdr icmp_hdr;
    char buf[PING_PKT_SIZE - sizeof(struct icmphdr)];
};

void sigint_handler(int sig);
void ping(int sockfd, char* host);
unsigned short checksum(void *b, int len);

int keep_pinging = 1;

int main(int argc, char**argv)
{
    struct addrinfo hints, *result, *addr;
    int addr_ret;
    int socketfd;

    //Opening raw socket requires root privelege
    if(geteuid() != 0)
    {
        printf("Must run as root\n");
        exit(1);
    }

    if (argc < 2)
    {
        printf("Usage: %s [hostname/ip address]\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; //ipv4 or ipv6
    hints.ai_socktype = SOCK_RAW; //raw socket for icmp
    hints.ai_protocol = IPPROTO_ICMP; //protocol for icmp

    addr_ret = getaddrinfo(argv[1], NULL, &hints, &addr);
    if (addr_ret != 0)
    {
        fprintf(stderr, "Error with getaddrinfo\n");
        exit(1);
    }


    //Loop through list to find valid address
    for (result = addr; result != NULL; result=result->ai_next)
    {
        socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if (socketfd == -1)
        {
            continue;
        }
        else if (connect(socketfd, result->ai_addr, result->ai_addrlen) != -1)
        {
            break;
        }
        close(socketfd);
    }

    //if result is null no addresses were valid
    if (result == NULL)
    {
        fprintf(stderr, "Could not connect to %s\n", argv[1]);
        exit(1);
    }

    struct timeval timeout;      
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    //Set timeout options for sockets
    if (setsockopt (socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        fprintf(stderr, "setsockopt failed\n");
    }
    if (setsockopt (socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        fprintf(stderr, "setsockopt failed\n");
    }

    ping(socketfd, argv[1]);

    return 0;
}

void ping(int sockfd, char* host)
{
    printf("PING %s\n", host);

    int num_sent = 0, num_passed = 0, i, success;
    struct timespec s;
    double time_start, time_sent, time_received, time_finished, time_rtt, time_total,
            rtt_min = INFINITY, rtt_max = -INFINITY, rtt_total = 0, rtt_avg;
    struct packet pkt;

    clock_gettime(CLOCK_MONOTONIC, &s);
    time_start = (double)(s.tv_sec);

    while (keep_pinging)
    {
        success = 1;

        bzero(&pkt, sizeof(pkt));
        pkt.icmp_hdr.type = ICMP_ECHO;
        pkt.icmp_hdr.un.echo.id = getpid();
        pkt.icmp_hdr.un.echo.sequence = ++num_sent;

        for (i = 0; i < sizeof(pkt.buf); ++i)
        {
            pkt.buf[i] = i+'0';
        }
        pkt.buf[i] = 0;

        pkt.icmp_hdr.checksum = checksum(&pkt, sizeof(pkt));
        
        clock_gettime(CLOCK_MONOTONIC, &s);
        time_sent = (double)(s.tv_nsec / 1.0e6);

        if (send(sockfd, &pkt, sizeof(pkt), 0) <= 0)
        {
            printf("Failed send\n");
            success = 0;
        }

        if (recv(sockfd, &pkt, sizeof(pkt), 0) <= 0)
        {
            printf("Failed recv\n");
            success = 0;
        }
        
        //Code must be 0 and type of 69(echo) for valid response
        if (success && !(pkt.icmp_hdr.code == 0) && !(pkt.icmp_hdr.type == 69))
        {
            success = 0;
        }

        clock_gettime(CLOCK_MONOTONIC, &s);
        time_received = (double)(s.tv_nsec / 1.0e6);
        time_rtt = time_received - time_sent;
        num_passed+=success;
        if (success)
        {
            printf("%ld bytes from %s: time=%f\n", sizeof(pkt), host, time_rtt);
        }

        if (time_rtt > rtt_max)
        {
            rtt_max = time_rtt;
        }
        else if (time_rtt < rtt_min)
        {
            rtt_min = time_rtt;
        }

        rtt_total += time_rtt;

        sleep(1);
    }

    clock_gettime(CLOCK_MONOTONIC, &s);
    time_finished = (double)(s.tv_sec);
    time_total = (time_finished - time_start) * 1000;
    float percent_loss = (((float)num_sent-(float)num_passed)/(float)num_sent) * 100.0f;
    rtt_avg = rtt_total/num_sent;

    printf("\n--- %s ping statistics ---\n", host);
    printf("%d packets trasnmitted, %d received, %f%% packet loss, time %fms\n", 
        num_sent, num_passed, percent_loss, time_total);
    printf("rtt min/avg/max = %f/%f/%f\n", rtt_min, rtt_avg, rtt_max);
    
    close(sockfd);

}

//Gracefully handle interupt signal 
void sigint_handler(int sig)
{
    keep_pinging = 0;
}

//perform checksum for icmp
unsigned short checksum(void *b, int len) 
{    
    unsigned short *buf = b; 
    unsigned int sum=0; 
    unsigned short result; 
  
    for ( sum = 0; len > 1; len -= 2 )
    {
        sum += *buf++;
    }
    if ( len == 1 )
    { 
        sum += *(unsigned char*)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF); 
    sum += (sum >> 16); 
    result = ~sum; 
    return result; 
}
