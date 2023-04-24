#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {

    char *ip = "127.0.0.1"; //local. fazer global depois (prov na mesma rede)
    int port = 5555; //arbitrario

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Erro ao criar socket do cliente");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = port;

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Erro ao conectar ao servidor");
        return 1;
    }

    char buffer[1024];
    bzero(buffer, 1024);
    int read_size = 0;

    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        printf("%s", buffer);
        send(client_socket, buffer, strlen(buffer), 0);

        read_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (read_size <= 0) {
            break;
        }

        buffer[read_size] = '\0';
        printf("%s", buffer);
    }

    close(client_socket);

    return 0;
}
