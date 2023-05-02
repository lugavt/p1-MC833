#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"

void printResponse(cJSON *profiles_array, char* print_type){

    int num_profiles = cJSON_GetArraySize(profiles_array);

    for(int i = 0; i < num_profiles; i++){

        if (i > 0){
            printf("\n-------------------------------------------------------------\n")
        }

        cJSON *profile = cJSON_GetArrayItem(profiles_array, i);

        cJSON *name = cJSON_GetObjectItem(profile, "nome");
        cJSON *email = cJSON_GetObjectItem(profile, "email");

        printf("NOME: %s\n", name->valuestring);
        printf("EMAIL: %s\n", email->valuestring);


        if (strcmp(print_type, "year") == 0) {
            
            cJSON *course = cJSON_GetObjectItem(profile, "formacao");
            printf("FORMAÇÃO: %s\n", course->valuestring);


        } else if (strcmp(print_type, "all") == 0) {

            cJSON *course = cJSON_GetObjectItem(profile, "formacao");
            cJSON *sobrenome = cJSON_GetObjectItem(profile, "sobrenome");
            cJSON *cidade = cJSON_GetObjectItem(profile, "cidade");
            cJSON *ano_formatura = cJSON_GetObjectItem(profile, "ano_formatura");

            printf("SOBRENOME: %s\n", sobrenome->valuestring);
            printf("CIDADE: %s\n", cidade->valuestring);
            printf("FORMAÇÃO: %s\n", course->valuestring);
            printf("ANO DE FORMAÇÃO: %d\n", ano_formatura->valueint);
            printf("HABILIDADES:\n");

            cJSON *skills_array = cJSON_GetObjectItem(profile, "habilidades");
            int num_skills = cJSON_GetArraySize(skills_array);
            
            for(int j = 0; j < num_skills; j++) {
                cJSON *skill = cJSON_GetArrayItem(skills_array, j);
                printf("    %s", skill->valuestring);
        }
    }
    
    
}
}

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
    int port = 5562; //arbitrario
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

        printf("\n-----------------------------------------------\n");
        printf("Escolha uma opcao:\n");
        printf("1 - Cadastro\n");
        printf("2 - Coletar perfis através do curso\n");
        printf("3 - Coletar perfis através das habilidades\n");
        printf("4 - Coletar perfis através do ano de formação\n");
        printf("5 - Coletar todos os perfis\n");
        printf("6 - Coletar informações de um perfil\n");
        printf("7 - Remover perfil\n");
        printf("8 - Desconectar e encerrar\n");

        printf("Digite a sua escolha (1-8): ");
        scanf("%d", &opcao);

        Payload payload;

        switch(opcao) {
            case 1:
                Profile profile;
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

                char input_message[1000];
                printf("Digite as suas habilidades separados por vírgula: ");
                scanf("%s", input_message);
                
                char *current_skill;
                char skills[1000] = "[";

                current_skill = strtok(input_message, ",");
                int skill_counter = 0;

                while (current_skill != NULL) {

                    if (skill_counter > 0){
                        strcat(skills, ",");
                    }
                    strcat(skills, "\"");
                    strcat(skills, current_skill);
                    strcat(skills, "\"");

                    current_skill = strtok(NULL, ",");
                    skill_counter++;
                }

                strcat(skills, "]");
                strcpy(profile.habilidades, skills);

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
                
                strcpy(payload.action, "getAllProfilesByCourse");
                printf("Digite o curso selecionado: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                cJSON *jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                cJSON *profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "course");

                // printf("%s", buffer);
                break;

            case 3:
                
                strcpy(payload.action, "getAllProfilesBySkill");

                printf("Digite a habilidade selecionada: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                cJSON *jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                cJSON *profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "skill");

                break;

            case 4:
                
                strcpy(payload.action, "getAllProfilesByYear");

                printf("Digite o ano de formatura selecionado: ");
                scanf("%d", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %d}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                cJSON *jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                cJSON *profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "year");
                break;

            case 5:
                
                strcpy(payload.action, "getAllProfiles");
                strcpy(payload.message, "");

                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": \"%s\"}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                cJSON *jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                cJSON *profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "all");

                break;

            case 6:
                
                strcpy(payload.action, "getProfile");

                printf("Digite o email do perfil desejado: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                cJSON *jsonPayload = cJSON_Parse(buffer);
                if (jsonPayload == NULL) {
                    printf("Erro ao fazer o parse do JSON.\n");
                    break;
                }

                cJSON *profiles_array = cJSON_GetObjectItem(jsonPayload, "profiles");

                printResponse(profiles_array, "all");
                break;

            case 7:
                
                strcpy(payload.action, "removeProfile");

                printf("Digite o email do perfil a ser removido: ");
                scanf("%s", payload.message);
                
                // SEND
                sprintf(buffer, "{\"action\": \"%s\", \"message\": %s}",
                payload.action, payload.message);
                printf("buffer: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer),0);

                // RESPONSE
                read_size = recv(client_socket, buffer, sizeof(buffer),0);

                if (read_size <= 0) {
                    break;
                }

                buffer[read_size] = '\0';
                printf("%s", buffer);
                break;

            case 8:

                close(client_socket);
                printf("Você foi desconectado. Volte sempre :)\n");
                return 1;

            default:
                printf("Opção inválida.\n");
                break;
        }
 }

    close(client_socket);

    return 0;
}
