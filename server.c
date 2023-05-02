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
#include <stdbool.h>

    
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

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;

    int read_size;    
    char buffer[1024];
    while (1) {
        
        bzero(buffer, 1024);
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
            
            FILE *fp = fopen("data.json", "r");

            char fileBuffer[1024];
            fread(fileBuffer, 1, 1024, fp);
            fclose(fp);

            cJSON *data_json = cJSON_Parse(fileBuffer);
            cJSON *profiles_array = cJSON_GetObjectItem(data_json, "profiles");

            int num_profiles = cJSON_GetArraySize(profiles_array);
            
            if (strcmp(action->valuestring, "register") == 0){ // register

                int existeId = 0;
                cJSON *inputEmail = cJSON_GetObjectItem(message, "email");

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
                    strcpy(buffer, "Usuário cadastrado com sucesso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
                else{
                    bzero(buffer, 1024);
                    strcpy(buffer, "Falha ao cadastrar usuário. Email em uso.\n");
                    send(client_socket, buffer, strlen(buffer), 0);
                }
            }

            else if (strcmp(action->valuestring, "getAllProfilesByCourse") == 0){
                
                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){ //verificando se ja tem
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *course = cJSON_GetObjectItem(profile, "formacao");
                    if (strcmp(course->valuestring, message->valuestring) == 0){
                        profilesCounter += 1;
                        cJSON *email = cJSON_GetObjectItem(profile, "email");
                        cJSON *name = cJSON_GetObjectItem(profile, "nome");
                        if (profilesCounter > 1){
                            strcat(payload, ",");
                        }
                        sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\"}",
                        email->valuestring, name->valuestring);
                        strcat(payload, profileJson);
                    } 
                }

                strcat(payload, "]}");
                bzero(buffer, 1024);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(action->valuestring, "getAllProfilesBySkill") == 0){

                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){ //verificando se ja tem

                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *skills_array = cJSON_GetObjectItem(profile, "habilidades");
                    int num_skills = cJSON_GetArraySize(skills_array);

                    for(int j = 0; j < num_skills; j++) {
                        cJSON *skill = cJSON_GetArrayItem(skills_array, j);
                        if (strcmp(skill->valuestring, message->valuestring) == 0){
                            profilesCounter += 1;
                            cJSON *email = cJSON_GetObjectItem(profile, "email");
                            cJSON *name = cJSON_GetObjectItem(profile, "nome");
                            if (profilesCounter > 1){
                                strcat(payload, ",");
                            }
                            sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\"}",
                            email->valuestring, name->valuestring);
                            strcat(payload, profileJson);
                        }
                    }
                }

                strcat(payload, "]}");
                bzero(buffer, 1024);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp(action->valuestring, "getAllProfilesByYear") == 0){

                char payload[10000];
                char profileJson[200];
                strcpy(payload, "{\"profiles\":[");
                int profilesCounter = 0;

                for(int i = 0; i < num_profiles; i++){ //verificando se ja tem
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *year = cJSON_GetObjectItem(profile, "ano_formatura");
                    if (year->valueint == message->valueint){
                        profilesCounter += 1;
                        cJSON *email = cJSON_GetObjectItem(profile, "email");
                        cJSON *name = cJSON_GetObjectItem(profile, "nome");
                        cJSON *course = cJSON_GetObjectItem(profile, "formacao");
                        if (profilesCounter > 1){
                            strcat(payload, ",");
                        }
                        sprintf(profileJson, "{\"email\": \"%s\", \"nome\": \"%s\", \"formacao\": \"%s\"}",
                        email->valuestring, name->valuestring, course->valuestring);
                        strcat(payload, profileJson);
                    } 
                }

                strcat(payload, "]}");
                bzero(buffer, 1024);
                sprintf(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);
            }

            else if (strcmp(action->valuestring, "getAllProfiles") == 0){ //retorna o arquivo em forma de string
                bzero(buffer, 1024);
                char *json_str = cJSON_PrintUnformatted(data_json);
                strcpy(buffer, json_str);
                send(client_socket, buffer, strlen(buffer), 0);
                free(json_str);
            }

            else if (strcmp(action->valuestring, "getProfile") == 0){ //testar
                char payload[10000];
                strcpy(payload, "{\"profiles\":[");

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, message->valuestring) == 0){
                        
                        char *json_str = cJSON_PrintUnformatted(profile);
                        strcat(payload, json_str);
                        free(json_str);  

                    }
                }

                strcat(payload, "]}");
                bzero(buffer, 1024);
                strcpy(buffer, payload);
                send(client_socket, buffer, strlen(buffer), 0);

            }

            else if (strcmp(action->valuestring, "removeProfile") == 0){

                bool profile_found = false;

                for(int i = 0; i < num_profiles; i++){
                    cJSON *profile = cJSON_GetArrayItem(profiles_array, i);
                    cJSON *email = cJSON_GetObjectItem(profile, "email");
                    if (strcmp(email->valuestring, message->valuestring) == 0){

                        profile_found = true;

                        cJSON_DeleteItemFromArray(profiles_array, i);
                        cJSON *removedProfile = cJSON_GetArrayItem(profiles_array, i);

                        fp = fopen("data.json", "w");
                        fprintf(fp, "%s", cJSON_PrintUnformatted(data_json));
                        fclose(fp);
        
                        cJSON_Delete(removedProfile);

                        break;
                    }                
                }
                if (profile_found) {strcpy(buffer, "Perfil removido com sucesso!");} else {strcpy(buffer, "Erro: perfil não encontrado!");}
                send(client_socket, buffer, strlen(buffer), 0);    
            }

            cJSON_Delete(jsonPayload);

            }
        }
    }


int main() {

    char *ip = "127.0.0.1"; //local. fazer global depois (prov na mesma rede)
    int port = 5563; //arbitrario
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

