#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// #include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() 
{
  char buffer[BUFFER_SIZE];
  char resp[] = "HTTP/1.0 200 OK\r\n"
                "Server: webserver-c\r\n"
                "Content-type: text/html\r\n\r\n"
                "<html>hello, world</html>\r\n"
                "<html>From C WebServer</html>\r\n";

  // Creamos el socket || sockfd == socket file descriptor
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) 
  {
    perror("webserver (socket)");
    return 1;
  }
  printf("Socket created successfully\n");

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
  printf("Socket successfully bound to address\n");

  // Escuchando conexiones entrantes
  if (listen(sockfd, SOMAXCONN) != 0) 
  {
    perror("webserver (listen)");
    return 1;
  }
  printf("Server listening for connections\n");

  // Bucle infinito para aceptar conexiones
  for (;;) {
    // Aceptamos conexiones entrantes
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
                           (socklen_t *)&host_addrlen);

    if (newsockfd < 0) 
    {
      perror("webserver (accept)");
      return 1;
    }
    printf("Connection accepted\n");

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
    printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port), method, uri, version);

    // Escribir en el socket
    int valwrite = write(newsockfd, resp, strlen(resp));
    if (valwrite < 0) 
    {
      perror("webserver (write)");
      continue;
    }

    close(newsockfd);
  }

  return 0;
}
