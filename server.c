            // Profile p;
            // strcpy(p.email, cJSON_GetObjectItem(json, "email")->valuestring);
            // strcpy(p.nome, cJSON_GetObjectItem(json, "nome")->valuestring);
            // strcpy(p.sobrenome, cJSON_GetObjectItem(json, "sobrenome")->valuestring);
            // strcpy(p.cidade, cJSON_GetObjectItem(json, "cidade")->valuestring);
            // strcpy(p.formacao, cJSON_GetObjectItem(json, "formacao")->valuestring);
            // p.ano_formatura = cJSON_GetObjectItem(json, "ano_formatura")->valueint;
            // strcpy(p.habilidades, cJSON_GetObjectItem(json, "habilidades")->valuestring);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "cJSON.h"

    
typedef struct { //struct contendo as infos de perfil
    char email[50];
    char nome[50];
    char sobrenome[50];
    char cidade[50];
    char formacao[50];
    int ano_formatura;
    char habilidades[100];
} Profile;

//struct profile profiles[1000]; //criação do vetor de perfis (estrutura de dados é uma lista de structs)
int num_profiles = 0;

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;

    int read_size;    
    char buffer[1024];
    bzero(buffer, 1024);
    while (1) {
        read_size = recv(client_socket, buffer, sizeof(buffer),0);
        if (read_size > 0){
            buffer[read_size] = '\0';
            cJSON *jsonPayload = cJSON_Parse(buffer);
            if (jsonPayload == NULL) {
                printf("Erro ao fazer o parse do JSON.\n");
                break;
            }

            cJSON *action = cJSON_GetObjectItem(jsonPayload, "action");
            cJSON *message = cJSON_GetObjectItem(jsonPayload, "message");
            cJSON *inputEmail = cJSON_GetObjectItem(message, "email");
            FILE *fp = fopen("data.json", "r");

            char fileBuffer[1024];
            fread(fileBuffer, 1, 1024, fp);
            fclose(fp);
            
            if (strcmp(action->valuestring, "register") == 0){ // register
                int existeId = 0;
                cJSON *data_json = cJSON_Parse(fileBuffer);
                cJSON *profiles_array = cJSON_GetObjectItem(data_json, "profiles");
                for(int i = 0; i < num_profiles; i++){ //verificando se ja tem
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, inputEmail->valuestring) == 0){ //ja existe o email
                        existeId = 1;
                        break;  
                    } 
                }
                if(existeId == 0){
                    cJSON_AddItemToArray(profiles_array, message);
                    fp = fopen("data.json", "w");
                    fprintf(fp, "%s", cJSON_PrintUnformatted(data_json));
                    fclose(fp);
                    bzero(buffer, 1024);
                    strcpy(buffer, "usuário cadastrado com sucesso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
                else{
                    bzero(buffer, 1024);
                    strcpy(buffer, "Falha ao cadastrar usuário. Email em uso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
            }
                    cJSON_Delete(jsonPayload);
            }
            printf("read_size: %d\n", read_size);
            printf("buffer: %s\n", buffer);
        }
    }


int main() {

    char *ip = "127.0.0.1"; //local. fazer global depois (prov na mesma rede)
    int port = 5561; //arbitrario
    socklen_t addr_size;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0); //IPv4, TCP
    if (server_socket < 0) { //padrão para verificar erros
        perror("Erro ao criar socket do servidor");
        exit(1);
    }

    struct sockaddr_in server_address, client_address; //sockaddr_in é uma estrutura que armazena o endereço e a porta
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = port;

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind error");
        exit(1);
    } //vincula o socket ao endereço. se retorna -1 deu erro

    
    listen(server_socket, 10);

    printf("Servidor escutando\n");
    int client_socket;

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        printf("client accepted\n");
        pthread_t tid; // precisa de thread pra atender em simultaneo
        pthread_create(&tid, NULL, handle_client, (void*)&client_socket);
        pthread_detach(tid);
    }
    
    close(client_socket);

    return 0;
}

