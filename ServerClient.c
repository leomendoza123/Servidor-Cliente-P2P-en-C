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
#include <errno.h>

char const *PORT ="51705";
char  *HOSTNAME = "localhost";
char *CURRENTSERVERPATH ;


// ClientSIde
int Client(int action, char *parameter);
void ClientConsole();

///ServerSide
void * Server();
void dostuff(int);
void error(const char *msg);
char * getdir(char *dir_localitation);
int dirExistens(char *dir_localitation);







int main(int argc, char *argv[]){
    CURRENTSERVERPATH = malloc(sizeof("/share"));
    CURRENTSERVERPATH = "/share";
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
        else if (!strcmp(action, "cd")) {
            Client(1, parameter);
        }
        else if (!strcmp(action, "quit")) {
            return;
        }
        else if (!strcmp(action, "get")) {

        }
        else if (!strcmp(action, "lcd")) {

        }
        else if (!strcmp(action, "ls")) {
            Client(4, NULL);

        }
        else if (!strcmp(action, "put")) {

        }
        else {

            //TODO: devolver la forma de uso de los argument
            printf("Argumento invalido: \n");
        }
    }
}


int Client(int action, char *parameter)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1024];

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


    if (action==0) {
        //Solicita funcion 0 (Pin)al server
        bzero(buffer,1024);
        strcat(buffer, "0");
        n = write(sockfd, buffer, strlen(buffer));
        if (n<0){
            error("Error: writing on server");
        }
        bzero(buffer, 1024);
        // Resive datos de solivitud
        n = read(sockfd, buffer, 1024);
        if (n<0){
            error("Error: reading on server");
        }
        if (!strcmp (buffer,"0") ){
            printf("Server: Conencted! \n");
            }
        }

    if (action==4) {
        //Solicita funcion 4 (ls) al server
        bzero(buffer,1024);
        strcat(buffer, "4");
        n = write(sockfd, buffer, strlen(buffer));
        bzero(buffer, 1024);
        // Resive datos de solivitud
        n = read(sockfd, buffer, 1024);
        printf("Server: %s \n", buffer);

    }

    if (action==1) {
        //Solicita funcion 1 (cd) al server
        bzero(buffer,1024);
        strcat(buffer, "1");
        n = write(sockfd, buffer, strlen(buffer));
        bzero(buffer, 1024);
        // Resive que el server lo escuche
        n = read(sockfd, buffer, 1024);
        // Envia direicion de directorio
        bzero(buffer,1024);
        strcat(buffer, parameter);
        n = write(sockfd, buffer, strlen(buffer));
        // Imprime resultado de servidor
        bzero(buffer,1024);
        n = read(sockfd, buffer, 1024);
        printf("Server: %s", buffer);

    }
    close(sockfd);
    return 0;


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
        //close(sockfd);
        return 0;
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {
        printf ("%s", CURRENTSERVERPATH);
        newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

/******** DOSTUFF() *********************
There is a separate instance of this function
for each connection.  It handles all communication
once a connnection has been established.
*****************************************/

void dostuff (int sock)
{

    int n;
    char buffer[1024];

    bzero(buffer,1024);
    n = read(sock,buffer,1024);
    if (n < 0) error("ERROR reading from socket");

    printf("Client request action #%s\n",buffer);

    // ES UN PIN
    if (!strcmp(buffer, "0")){
        n = write(sock,"0",1);
        if (n < 0)
            error("ERROR writing to socket");

    }

    // ES UN LS
    if (!strcmp(buffer, "4")){
        printf ("%s",CURRENTSERVERPATH);
        char * dirString= getdir(CURRENTSERVERPATH);
        printf ("%s %d \n",dirString, (int)strlen(dirString));
        n = write(sock,dirString,strlen(dirString));
        if (n < 0)
            error("ERROR writing to socket");

    }


    // ES UN DR
    if (!strcmp(buffer, "1")){
        //Solicita el nuevo directorio
        n = write(sock,"0",1);
        bzero(buffer,1024);
        n = read(sock,buffer,255);
        char *Nuevodirectorio = buffer;
        if (dirExistens(Nuevodirectorio)==1){

            printf ("%s \n", Nuevodirectorio);
            CURRENTSERVERPATH = calloc (strlen (Nuevodirectorio), sizeof(char *));
            strcpy (CURRENTSERVERPATH,Nuevodirectorio);
            printf ("%s \n",CURRENTSERVERPATH);
            n = write(sock,"Ok",2);
        }
        else{
            n = write(sock,"Invalid directory",17);
        }



    }
}



char *getdir(char *dir_localitation) {
    //TODO Mostrar de forma diferente cuando se obtiene un archivo o cuando se obtiene un directorio
    char *directoriString="";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dir_localitation)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
           if (!(ent->d_name [0]=='.'))
            {

                char *temp = malloc(strlen(ent->d_name) + 4 + strlen(directoriString));
                strcat(temp, directoriString);
                strcat(temp, ent->d_name);
                strcat(temp, " / ");
                directoriString = temp;
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
    return directoriString;
}


int dirExistens(char *dir_localitation) {
    struct stat s;
    int err = stat(dir_localitation, &s);
    if(-1 == err) {
        if(ENOENT == errno) {
            return 0;
        } else {
            perror("stat");
            exit(1);
        }
    } else {
        if(S_ISDIR(s.st_mode)) {
            return 1;
        } else {
           return 0;
        }
    }
    return -1;
}