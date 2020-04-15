#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char**argv)
{
    struct addrinfo hints, *result, *addr;
    int error;
    int socketfd;

    if (argc < 2)
    {
        printf("Usage: %s <hostname/ip address>\n", argv[0]);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; //ipv4 or ipv6
    hints.ai_socktype = SOCK_RAW; //raw socket for icmp
    hints.ai_protocol = IPPROTO_ICMP;

    error = getaddrinfo(argv[1], NULL, &hints, &addr);
    if (error != 0)
    {
        fprintf(stderr, "Error with getaddrinfo\n");
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
    }




    return 0;
}