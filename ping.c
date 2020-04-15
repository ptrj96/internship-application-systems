#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

void sigint_handler(int sig);
void ping(int sockfd, char* host);

int keep_pinging = 1;

int main(int argc, char**argv)
{
    struct addrinfo hints, *result, *addr;
    int error;
    int socketfd;

    if (argc < 2)
    {
        printf("Usage: %s <hostname/ip address>\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; //ipv4 or ipv6
    hints.ai_socktype = SOCK_RAW; //raw socket for icmp
    hints.ai_protocol = IPPROTO_ICMP;

    error = getaddrinfo(argv[1], NULL, &hints, &addr);
    if (error != 0)
    {
        fprintf(stderr, "Error with getaddrinfo\n");
        exit(1);
    }


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

    if (result == NULL)
    {
        fprintf(stderr, "Could not connect to %s\n", argv[1]);
        exit(1);
    }

    ping(socketfd, argv[1]);

    return 0;
}

void ping(int sockfd, char* host)
{
    printf("PING %s\n", host);

    int num_sent = 0, *num_failed = 0;
    time_t time_start, time_sent, time_received, time_finished;

    time_start = time(NULL);

    while (keep_pinging)
    {

    }

    time_finished = time(NULL);

    printf("--- %s ping statistics ---\n", host);
    // printf("%d packets trasnmitted, %d received, %f%% packet loss, time %dms\n");
    // printf("rtt min/avg/max/mdev = %f/%f/%f/%f ms\n");
    
    close(sockfd);

}

void sigint_handler(int sig)
{
    keep_pinging = 0;
}