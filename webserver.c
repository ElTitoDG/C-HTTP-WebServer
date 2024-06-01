#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080 // Puerto predeterminado
#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 524288000 // 500Mb

int setup_server()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("webserver (socket)");
        exit(1);
    }
    printf("Socket creado con éxito\n");

    // Creamos la dirección para vincularla al socket
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Vincular socket a la dirección
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
    {
        perror("webserver (bind)");
        exit(1);
    }
    printf("Socket enlazado con éxito a la dirección\n");

    // Escuchando conexiones entrantes
    if (listen(sockfd, SOMAXCONN) != 0)
    {
        perror("webserver (listen)");
        exit(1);
    }
    printf("Servidor a la espera de conexiones en http://%s:%u/\n\n", inet_ntoa(host_addr.sin_addr),
           ntohs(host_addr.sin_port));

    return sockfd;
}

void handle_request(int newsockfd, char *file_content, struct sockaddr_in *client_addr, int *client_addrlen)
{
    const int buffer_size = BUFFER_SIZE;
    char buffer[buffer_size];

    // Sacamos la direccion del cliente
    int sockn = getsockname(newsockfd, (struct sockaddr *)client_addr,
                            (socklen_t *)client_addrlen);
    if (sockn < 0)
    {
        perror("webserver (getsockname)");
        exit(1);
    }

    // Leemos del socket
    int valread = read(newsockfd, buffer, buffer_size);
    if (valread < 0)
    {
        perror("webserver (read)");
        exit(1);
    }

    // Leemos la petición
    char method[buffer_size], uri[buffer_size], version[buffer_size];
    sscanf(buffer, "%s %s %s", method, uri, version);
    printf("[%s:%u]\n Método: %s\n URI: %s\n Version: %s\n", inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port), method, uri, version);

    if (strcmp(uri, "/") == 0)
    {
        FILE *file = fopen("../docs/index.html", "r");
        if (file == NULL)
        {
            perror("webserver (fopen)");
            exit(1);
        }

        // Obtenemos el tamaño del archivo
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Comprobar si el tamaño del archivo es mayor que MAX_FILE_SIZE
        if (file_size > MAX_FILE_SIZE)
        {
            fprintf(stderr, "Error: index.html es demasiado grande\n");
            fclose(file);
            exit(1);
        }

        // Leer el archivo en el buffer
        size_t nread = fread(file_content, 1, file_size, file);
        if (nread != file_size)
        {
            perror("webserver (fread)");
            exit(1);
        }
        fclose(file);
    }
}

void send_response(int newsockfd, char *file_content)
{
    // Asignar memoria dinámicamente para resp
    char *resp = malloc(strlen("HTTP/1.0 200 OK\r\n"
                               "Server: webserver-c\r\n"
                               "Content-type: text/html\r\n"
                               "Content-Length: %lu\r\n\r\n"
                               "%s") +
                        strlen(file_content));
    if (!resp)
    {
        perror("webserver (malloc)");
        exit(1);
    }

    // Respuesta al cliente
    sprintf(resp, "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n"
                  "Content-Length: %lu\r\n\r\n"
                  "%s",
            strlen(file_content), file_content);

    // Escribir en el socket
    int valwrite = write(newsockfd, resp, strlen(resp));
    if (valwrite < 0)
    {
        perror("webserver (write)");
        free(resp);
        exit(1);
    }

    free(resp);
}

int main()
{
    const int max_file_size = MAX_FILE_SIZE;
    char *file_content = malloc(max_file_size);
    if (!file_content)
    {
        perror("webserver (malloc)");
        exit(1);
    }

    int sockfd = setup_server();

    // Configuración para sacar la dirección del cliente
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Bucle infinito para aceptar conexiones
    for (;;)
    {
        // Aceptamos conexiones entrantes
        int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addrlen);

        if (newsockfd < 0)
        {
            perror("webserver (accept)");
            free(file_content);
            close(sockfd);
            exit(1);
        }
        printf("Conexión aceptada\n");

        handle_request(newsockfd, file_content, &client_addr, &client_addrlen);
        send_response(newsockfd, file_content);

        close(newsockfd);
    }

    //free(file_content);
    //close(sockfd);

    //return 0;
}
