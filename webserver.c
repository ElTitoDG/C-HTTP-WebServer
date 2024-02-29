#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <errno.h>

#define PORT 8080

int main()
{
    //Creamos el socket || sockfd == socket file descriptor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("webserver (socket)");
        return 1;
    }
    printf("Socket created successfully\n");

    //Creamos la dirección para vincularla al socket
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Vincular socket a la dirección
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
    {
        perror("webserver (bind)");
        return 1;
    }
    printf("Socket successfully bound to address\n");

    //Escuchando conexiones entrantes
    if (listen(sockfd, SOMAXCONN) != 0)
    {
        perror("webserver (listen)");
        return 1;
    }
    printf("Server listening for connections\n");

    //Bucle infinito para aceptar conexiones
    for (;;)
    {
        //Aceptamos conexiones entrantes
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);

        if (newsockfd < 0)
        {
            perror("webserver (accept)");
            return 1;
        }
        printf("Connection accepted\n");

        close(newsockfd);
    }

    return 0;
}
