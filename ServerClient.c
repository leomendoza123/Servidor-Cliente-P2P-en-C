#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

char const *PORT ="8893";
char  *HOSTNAME = "localhost";
char *DIRECTORIOACTUAL = "/share";


// ClientSIde
int Client(int action, char *parameter);
void ClientConsole();

///ServerSide
void * Server();
void dostuff(int);
void error(const char *msg);
void printdir(char *dir, int depth);


void client_scanArgs(char **parametros);

int main(int argc, char *argv[]){
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    pthread_t server;

    // Se inicia el hilo de el servidor
    pthread_create(&server, NULL, Server, NULL);
    ClientConsole();

    return (0);


}



///________________________________________________________________________________________________
////___________________________________CLIENT_SIDE_________________________________________________



void ClientConsole() {
    while ("true") {


        printf("*** Welcome to P2P file manager ***\n");
        char action[256];
        char parameter[256];
        bzero(action,256);
        bzero(parameter,256);
        fgets(action,255,stdin);
        fgets(parameter,255,stdin);
        action[strlen(action)-1] = '\0';
        parameter[strlen(parameter)-1] = '\0';





        if (!strcmp(action, "open")) {
            //HOSTNAME = parameter;
            Client(0, NULL);
            //TODO hacer proceso de validacion
            // Este puede ser un string usuario y contraseña antes de cada llamada
        }
        else if (!strcmp(action, "close")) {
            //TODO
            // Borra los datos de validacion

        }
        else if (!strcmp(action, "quit")) {
            return;

        }
        else if (!strcmp(action, "get")) {

        }
        else if (!strcmp(action, "lcd")) {

        }
        else if (!strcmp(action, "ls")) {

        }
        else if (!strcmp(action, "put")) {

        }
        else {

            //TODO: devolver la forma de uso de los argument
            printf("Argumento invalido: \n");
        }
        sleep (1);
    }
}


int Client(int action, char *parameter)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    portno = atoi(PORT);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(HOSTNAME);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");


    //_________________________________Conection PIN__________________

    if (action == 0) {
            strcat(buffer, "0");
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");
            bzero(buffer, 256);
            n = read(sockfd, buffer, 255);
            if (n < 0)
                error("ERROR reading from socket");
            printf("You are conencted!");
            close(sockfd);
            return 0;
    }
    if (action == 1) {
        strcat(buffer, "1");
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
            error("ERROR reading from socket");
        printf("You are conencted!");
        close(sockfd);
        return 0;
    }
    return 1;


}




///________________________________________________________________________________________________
////___________________________________SERVER_SIDE_________________________________________________



void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *Server() {
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(PORT);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) {
        printf ("********************************************************************\n");
        printf ("*************** Port in use, ServerSide will not run ***************\n");
        return 0;
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    printf ("********************************************************************\n");
    printf("**********************Server online at port %s  ********************\n", PORT);
    while (1) {
        newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");

            close(sockfd);
            dostuff(newsockfd);
            exit(0);

         close(newsockfd);
    } /* end of while */
    close(sockfd);
    return  0;
}

/******** DOSTUFF() *********************
There is a separate instance of this function
for each connection.  It handles all communication
once a connnection has been established.
*****************************************/

void dostuff (int sock)
{
    int n;
    char buffer[256];

    bzero(buffer,256);


    n = read(sock,buffer,255);
    if (n < 0) error("ERROR reading from socket");

    /// El cliente hace un pin
    if (!strcmp(buffer, "0"))
    {
        printf("Client conected");
        n = write(sock, "1", 1);
        if (n < 0) error("ERROR writing to socket");
    }
    if (!strcmp(buffer, "1"))
    {
        printf("Client conected");
        n = write(sock, "1", 1);
        if (n < 0) error("ERROR writing to socket");
    }
}



void printdir(char *dir, int depth) {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 ||
                    strcmp("..",entry->d_name) == 0)
                continue;
            printf("%*s%s/\n",depth,"",entry->d_name);
            /* Recurse at a new indent level */
            printdir(entry->d_name,depth+4);
        }
        else printf("%*s%s\n",depth,"",entry->d_name);
    }
    chdir("..");
    closedir(dp);
}
