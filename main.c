#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <stdlib.h>
#define MAX_ARGUMENTS 5
#define MAX_HISTORY 500

enum builtinCommands{PWD, CD, EXIT, SET, UNSET, ECHO, HELP, HISTORY};

// history open
// global variables
    int his_index = 0;
    int key_index = 0;
    int quit = 1;
    int Tigran_Mec = 1;


// DECLARATION
    int is_builtin(char* command);
    void my_pwd(char* corrnet_pwd);
    int my_cd(char** corrent_pwd, char* directory, long int* pwd_size);
    void my_exit();
    void my_set(char** tokens, int count, char** keys_data, char** values_data);
    void my_unset(char** tokens, int count, char** keys_data, char** values_data);
    void my_echo(char** tokens, int count);
    void my_help(char** tokens, int count);
    void my_history(int fd, char** history_data, int* his_index);
    void resize_pwd(char** corrent_pwd, long int* pwd_size, long int needed_size);

int main(){
    char* corrent_pwd = strdup("/Users/myShell");
    long int pwd_size = strlen(corrent_pwd) + 1;
    
    // history , keys, values 
    
    int history_fd = open("history.txt", O_RDWR | O_CREAT | O_SYNC);
    if(history_fd == -1){
        perror("open error.");
    }
    

    char** history_data = malloc(sizeof(char*) * MAX_HISTORY); 
    if(history_data == NULL){
        printf("malloc for history failed.");
        return 1;
    }
    char** keys_data = (char**)malloc(sizeof(char*) * 100);
    if(keys_data == NULL){
        printf("malloc for keys failed");
        return 1;
    }
    
    char** values_data = (char**)malloc(sizeof(char*) * 100);
    if(values_data == NULL){
        printf("malloc for values failed");
        return 1;}
    



    // SHELL START

    while(quit){
        printf("\n-->>");
        
        char input[50];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
    
       
        history_data[his_index] = strdup(input);
        his_index++;

        if(his_index == 500){
            for(int x = 0; x < 500; x++){
                if(history_data[x]){
                    write(history_fd, history_data[x], strlen(history_data[x]));
                    write(history_fd,"\n", 1);
                    free(history_data[x]);
                    history_data[x] = NULL;
                }
            }
            his_index = 0;
        }        

        //TOKENIZING & PARSING
        
        char* tokens[MAX_ARGUMENTS];
        memset(tokens, 0, sizeof(tokens));

        char* one_token = strtok(input, " ");
        int i = 0;
        while(one_token != NULL && i < MAX_ARGUMENTS){
            tokens[i++] = one_token;
            one_token = strtok(NULL, " ");
        }

        bool is_var = false;
        for(int j = 0; j < i; j++){
            if(strchr(tokens[j],'=') != NULL){
                int key_flag = 0;
                char* symbol = strchr(tokens[j],'=');
                *symbol = '\0';
                char* key = tokens[j];
                char* value = symbol + 1;
                if(key && value){
                    for(int i = 0; i < key_index; i++){
                        if(strcmp(key, keys_data[i]) == 0){
                            free(values_data[i]);
                            values_data[i] = strdup(value);
                            key_flag = 1;
                            break;
                        }
                    }
                    if(key_flag == 0){
                        keys_data[key_index] = strdup(key);
                        values_data[key_index] = strdup(value);
                        key_index++;
                    }
                }
                is_var = true;
            }
            else if(tokens[j][0] == '$'){
                int variable_available = 0;
                char* variable = tokens[j] + 1;
                for(int i = 0; i < key_index; i++){
                    if(strcmp(variable, keys_data[i]) == 0){
                        tokens[j] = values_data[i];
                        variable_available = 1;
                    }
                }
                if(variable_available == 0){
                    printf("Variable is not correct.");
                    tokens[j] = "";
                } 
            }
        }
             if(is_var){
                continue;
             }   
            int builtin = is_builtin(tokens[0]);
                if(builtin != -1){
                    switch(builtin){
                        case PWD:
                            my_pwd(corrent_pwd);
                            break;
                        case CD:
                            my_cd(&corrent_pwd,tokens[1], &pwd_size);
                            break;
                        case EXIT:
                            printf("Good Bye!\n");
                            my_exit(); 
                            break;
                        case SET:
                            my_set(tokens, i, keys_data,values_data);
                            break;
                        case UNSET:
                            my_unset(tokens, i, keys_data,values_data);
                            break;
                        case ECHO:
                            my_echo(tokens, i);
                            break;
                        case HELP:
                            my_help(tokens,i);
                            break;
                        case HISTORY:
                            my_history(history_fd, history_data,&his_index);
                            break;
                        default:
                            continue;
                    }
                }
                else{
                    pid_t pid = fork();
                    if(pid == 0){
                        execvp(tokens[0], tokens);
                        perror("Command not found.");
                        exit(1);
                    }
                    else if(pid > 0){
                        int status;
                        waitpid(pid, &status, 0);
                    }
                    else{
                        perror("Fork failed.");
                    }
                

        }
       
    }
    
    for(int i = 0; i < key_index; i++){
        free(keys_data[i]);
        free(values_data[i]);
    }
    free(keys_data);
    free(values_data);

    for(int i = 0; i < his_index; i++ ){
        free(history_data[i]);
    }
    free(history_data);
    close(history_fd);
    return 0;
}


// DEFINITIONS 

int is_builtin(char* command){
    if(strcmp(command, "pwd") == 0){
        return PWD;
    }      
    if(strcmp(command, "cd") == 0){
        return CD;
    }
    if(strcmp(command, "exit") == 0){
        return EXIT;
    }
    if(strcmp(command, "set") == 0){
        return SET;
    }
    if(strcmp(command, "unset") == 0){
        return UNSET;
    }
    if(strcmp(command, "echo") == 0){
        return ECHO;
    }
    if(strcmp(command, "help") == 0){
        return HELP;
    }
    if(strcmp(command, "history") == 0){
        return HISTORY;
    }
    return -1;
}


void my_pwd(char* corrent_pwd){ 
    printf("Corrent directory: %s\n", corrent_pwd);
}


int my_cd(char** corrent_pwd, char* directory, long int* pwd_size){
    if(directory == NULL){
        return 0;
    }
    if(strcmp(directory ,"..") == 0){
        int len = strlen(*corrent_pwd);
        if(len == 0){ return 0;}
        
        if(len > 1 && (*corrent_pwd)[len - 1] == '/'){
            (*corrent_pwd)[len - 1] = '\0';
            len--;
        }

        char* slesh = strrchr((*corrent_pwd), '/');
        if(slesh != NULL){
            if(slesh == (*corrent_pwd)){
                (*corrent_pwd)[1] = '\0';
            }
            else {
                *slesh = '\0';
            }
        }
        return 0;
    }
    int dir_fd = open(directory, O_RDONLY);
    if(dir_fd == -1){
        printf("Directory not found.");
        return -1;
    }

    long int needed_size = strlen(*corrent_pwd) + 1 + strlen(directory) + 1;
    resize_pwd(corrent_pwd, pwd_size, needed_size);

    strcat((*corrent_pwd), "/");
    strcat((*corrent_pwd),directory);
    return 0;
}

void my_exit(){
    quit = 0;
}

void my_set(char** tokens,int count,char** keys_data, char** values_data){
    if(count == 1){
        for(int i = 0; i < key_index; i++){
            printf("%d: [%s] = [%s]\n", i, keys_data[i], values_data[i]);
        }
        return;
    }
    for(int j = 1; j < count; j++){
        if(strchr(tokens[j],'=') != NULL){
            int key_flag = 0;
            char* symbol = strchr(tokens[j],'=');
            *symbol = '\0';
            char* key = tokens[j];
            if(strlen(key) == 0){
                printf("Empty variable name for [%d] VAR\n", j+1);
                continue;
            }
            char* value = symbol + 1;
            if(key && value){
                for(int i = 0; i < key_index; i++){
                    if(strcmp(key, keys_data[i]) == 0){
                        free(values_data[i]);
                        values_data[i] = strdup(value);
                        key_flag = 1;
                        break;
                    }
                }
                if(key_flag == 0){
                    keys_data[key_index] = strdup(key);
                    values_data[key_index] = strdup(value);
                    key_index++;
                }
            }  
        }
        else{
           printf("Invalid setting for [%d] VAR\n", j); 
        }
    }
}


void my_unset(char** tokens, int count, char** keys_data, char** values_data){
    if(count == 1){
        printf("Invalid using UNSET : unset VAR1 ... \n");
        return;
    }
    for(int i = 1; i < count; i++){
        int key_found = 0;
        for(int j = 0; j < key_index; j++){
            if(strcmp(tokens[i],keys_data[j]) == 0){
                free(keys_data[j]);
                free(values_data[j]);
                for(int k = j; k < key_index - 1; k++){
                    keys_data[k] = keys_data[k+1];
                    values_data[k] = values_data[k+1];
                }
 
                key_index--;
                key_found = 1;
                break;
            }
        }
        if(key_found == 0){
            printf("variable number[%d] is invalid", i+1);
        }
    }

}

void my_echo(char** tokens, int count){
    if(count == 1){
        printf("\n");
    }
    else{
        for(int i = 1; i < count; i++){
            printf("%s ",tokens[i]);
        }
        printf("\n");
    }
}

void my_help(char** tokens, int count){
    if(count == 1){
        printf("I can give you help only for my builtin commands [pwd,cd,exit,set,unset,echo,history].\n");
        printf("help [command]\n");
    }
    for(int i = 1; i < count; i++){
        int command = is_builtin(tokens[i]);
        switch(command){
            case PWD:
                printf("pwd - Print current directory.\n");        
                break;
            case CD:
               printf("cd [DIR] - Change current directory. Use '..' to go up.\n");
                break;
            case EXIT:
               printf("exit - Exit the shell.\n");
               break;
            case SET:
                printf("set VAR=VAL [...] - Set one or more variables.\nset - Show all variables.\n");
                break;
            case UNSET:
                printf("unset VAR [...] - Remove one or more variables by name.\n");
                break;
            case ECHO:
               printf("echo [ARG ...] - Print all arguments. Use $VAR for variable substitution.\n"); 
               break;
            case HELP:
               printf("help [COMMAND] - Show help message.\n");
               break;
            case HISTORY:
               printf("history - Print all previously entered commands.\n");
               break;
            default:
               printf("Unknown command: %s\n", tokens[i]);
                continue;
        }
    }

}

void my_history(int fd, char** history_data, int* his_index){
    for(int x = 0; x < *his_index; x++){
        if(history_data[x]){
            write(fd, history_data[x], strlen(history_data[x]));
            write(fd,"\n", 1);
            free(history_data[x]);
            history_data[x] = NULL;
        }
    }
    *his_index = 0;

    lseek(fd,0,SEEK_SET);
    char buf[501];
    long int bytes = read(fd,buf,sizeof(buf));
    if(bytes <= 0){
        printf("History is empty.");
        return;
    }
    printf("%s",buf);
        
}

void resize_pwd(char** corrent_pwd, long int* pwd_size, long int needed_size ){
    if(needed_size >= *pwd_size){
        long int new_size = needed_size * 2;
        char* new_buffer = realloc(*corrent_pwd, new_size);
        if(!new_buffer){
            perror("Realloc failed\n");
            exit(1);
        }
        *corrent_pwd = new_buffer;
        *pwd_size = new_size;
    }
}
