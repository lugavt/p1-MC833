#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {

    typedef struct { //struct contendo as infos de perfil
        char email[50];
        char nome[50];
        char sobrenome[50];
        char cidade[50];
        char formacao[50];
        int ano_formatura;
        char habilidades[100];
    } Profile;

    typedef struct { //struct contendo o payload das mensagens a serem enviadas
        char action[50];
        char message[500];
    } Payload;

    char *ip = "127.0.0.1"; //local. fazer global depois (prov na mesma rede)
    int port = 5561; //arbitrario
    struct sockaddr_in client_address;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        perror("Erro ao criar socket do cliente");
        exit(1);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
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

        int opcao;

        printf("Escolha uma opcao:\n");
        printf("1 - Cadastro\n");
        printf("2 - Coletar perfis através do curso\n");
        printf("3 - Coletar perfis através das habilidades\n");
        printf("4 - Coletar perfis através do ano de formação\n");
        printf("5 - Coletar todos os perfis\n");
        printf("6 - Coletar informações de um perfil\n");
        printf("7 - Remover perfil\n");

        printf("Digite a sua escolha (1-7): ");
        scanf("%d", &opcao);

        switch(opcao) {
            case 1:
                Profile profile;
                Payload payload;
                strcpy(payload.action, "register");

                printf("Para realizar o cadastro precisaremos de algumas informações: \n");

                printf("Digite o seu email: ");
                scanf("%s", profile.email);

                printf("Digite o seu nome: ");
                scanf("%s", profile.nome);

                printf("Digite o seu sobrenome: ");
                scanf("%s", profile.sobrenome);

                printf("Digite a sua cidade: ");
                scanf("%s", profile.cidade);

                printf("Digite a sua formacao: ");
                scanf("%s", profile.formacao);

                printf("Digite o ano de formatura: ");
                scanf("%d", &profile.ano_formatura);

                printf("Digite as suas habilidades: ");
                scanf("%s", profile.habilidades);

                sprintf(payload.message, "{\"email\": \"%s\", \"nome\": \"%s\", \"sobrenome\": \"%s\", \"cidade\": \"%s\", \"formacao\": \"%s\", \"ano_formatura\": %d, \"habilidades\": \"%s\"}",
                profile.email, profile.nome, profile.sobrenome, profile.cidade, profile.formacao, profile.ano_formatura, profile.habilidades);

                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                printf("%s", buffer);
                break;

            case 2:
                Payload payload;
                strcpy(payload.action, "getAllProfilesByCourses");
                break;
            case 3:
                Payload payload;
                strcpy(payload.action, "getAllProfilesBySkills");
                break;
            case 4:
                Payload payload;
                strcpy(payload.action, "getAllProfilesByYear");
                break;
            case 5:
                Payload payload;
                strcpy(payload.action, "getAllProfiles");
                break;
            case 6:
                Payload payload;
                strcpy(payload.action, "getProfile");
                break;
            case 7:
                Payload payload;
                strcpy(payload.action, "removeProfile");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }

        //char *json_str = "{\"email\": \"gui@gmail.com\",\"nome\": \"Guilherme\",\"sobrenome\": \"Tezoli\",\"cidade\": \"São Paulo\",\"formacao\": \"Engenharia\",\"ano_formatura\": 2024,\"habilidades\": \"Python, Java, REDES\"}";
    }

    close(client_socket);

    return 0;
}
