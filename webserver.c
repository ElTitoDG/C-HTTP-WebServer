// C Web Server that only server the file index.html

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080 // Puerto predeterminado
#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 524288000 // 500Mb

int main()
{
  char buffer[BUFFER_SIZE];
  char *resp;
  char *file_content = malloc(MAX_FILE_SIZE);
  if (!file_content)
  {
      perror("webserver (malloc)");
      return 1;
  }

  // Creamos el socket || sockfd == socket file descriptor
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    perror("webserver (socket)");
    return 1;
  }
  printf("Socket creado con éxito\n");

  // Creamos la dirección para vincularla al socket
  struct sockaddr_in host_addr;
  int host_addrlen = sizeof(host_addr);

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(PORT);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Configuracion para sacar la direccion del cliente
  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  // Vincular socket a la dirección
  if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
  {
    perror("webserver (bind)");
    return 1;
  }
  printf("Socket enlazado con éxito a la dirección\n");

  // Escuchando conexiones entrantes
  if (listen(sockfd, SOMAXCONN) != 0)
  {
    perror("webserver (listen)");
    return 1;
  }
  printf("Servidor a la espera de conexiones en http://%s:%u/\n\n", inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  // Bucle infinito para aceptar conexiones
  for (;;) {

    // Aceptamos conexiones entrantes
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
                           (socklen_t *)&host_addrlen);

    if (newsockfd < 0)
    {
      perror("webserver (accept)");
      continue;
    }
    printf("Conexión aceptada\n");

    // Sacamos la direccion del cliente
    int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr,
                            (socklen_t *)&client_addrlen);
    if (sockn < 0)
    {
      perror("webserver (getsockname)");
      continue;
    }

    // Leemos del socket
    int valread = read(newsockfd, buffer, BUFFER_SIZE);
    if (valread < 0)
    {
      perror("webserver (read)");
      continue;
    }

    // Leemos la peticion
    char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
    sscanf(buffer, "%s %s %s", method, uri, version);
    printf("[%s:%u]\n Metodo: %s\n URI: %s\n Version: %s\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port), method, uri, version);

    if (strcmp(uri, "/") == 0)
    {
        FILE *file = fopen("../docs/index.html", "r");
        if (file == NULL)
        {
            perror("webserver (fopen)");
            continue;
        }

        //Obtenemos el tamaño del archivo
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Comprobar si el tamaño del archivo es mayor que MAX_FILE_SIZE
        if (file_size > MAX_FILE_SIZE)
        {
            fprintf(stderr, "Error: index.html es demasiado grande\n");
            fclose(file);
            continue;
        }

        // Leer el archivo en el buffer
        size_t nread = fread(file_content, 1, file_size, file);
        if (nread != file_size)
        {
            perror("webserver (fread)");
            continue;
        }
        fclose(file);

        // Asignar memoria dinámicamente para resp
        resp = malloc(strlen("HTTP/1.0 200 OK\r\n"
                              "Server: webserver-c\r\n"
                              "Content-type: text/html\r\n"
                              "Content-Length: %lu\r\n\r\n"
                              "%s") + file_size);
        if (!resp)
        {
            perror("webserver (malloc)");
            continue;
        }

        //Respuesta al cliente
        sprintf(resp, "HTTP/1.0 200 OK\r\n"
                        "Server: webserver-c\r\n"
                        "Content-type: text/html\r\n"
                        "Content-Length: %lu\r\n\r\n"
                        "%s", strlen(file_content), file_content);
    }

    // Escribir en el socket
    int valwrite = write(newsockfd, resp, strlen(resp));
    if (valwrite < 0)
    {
      perror("webserver (write)");
      continue;
    }

    close(newsockfd);
  }

  free(file_content);
  free(resp);

  return 0;
}
