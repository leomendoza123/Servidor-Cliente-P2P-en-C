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

char const *PORT ="51004";
char  *HOSTNAME = "localhost";
char *CURRENTSERVERPATH ;
char *CURRENTCLIENTPATH ;


// ClientSIde
int Client(int action, char *parameter,  int sockfd);
int ClientConect(char *parameter);

void ClientConsole();


int autenticacionOpen(char *parametros){
    char *user;
    char *password;
    char *directory;
    char *string;
    char s[1] = "$";

    int i;
    i = 1;

    string = strtok(parametros, s);
    while(string != NULL){
        if(i == 1){
            user = string;
        }
        else if(i == 2){
            password = string;
        }
        else if(i == 3){
            directory = string;
        }
        string = strtok(NULL, s);
        i++;
    }

    char fileBuffer[1024] = "";

    FILE *archivo;
    archivo = fopen("cuentas.txt", "r");

    while(fscanf(archivo, "%s", fileBuffer) != EOF){

        char *fileUser;
        char *filePW;

        int j;
        j = 0;
        char *data;
        data = strtok(fileBuffer, s);
        while(data != NULL){
            if(j == 0){
                fileUser = data;
            }
            else{
                filePW = data;
            }
            data = strtok(NULL, s);
            j++;
        }

        if(!strcmp(fileUser, user) && !strcmp(filePW, password)){
            fclose(archivo);

            CURRENTSERVERPATH = calloc(strlen (directory), sizeof(char *));
            strcpy(CURRENTSERVERPATH, directory);

            return 1;
        }
    }
    fclose(archivo);
    return 0;
}


void sendMsgToServer(int sockfd, char buffer[], char *parameter);

void reciveMsgFromServer( int sockfd,  char buffer[]);

///ServerSide
void * Server();
void dostuff(int);
void error(const char *msg);
char * getdir(char *dir_localitation);
int dirExistens(char *dir_localitation);


void sendMsg(int sock, char buffer[], char *parameter);

void reciveMsg(int sock, char buffer[]);

int main(int argc, char *argv[]){
    CURRENTSERVERPATH = "/share";
    CURRENTCLIENTPATH = "/home/leoanardo/Desktop";
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
    int sockfd = 0;
    while ("true") {


        printf("*** Welcome to P2P file manager ***\n");
        char action[256];
        char parameter[256];
        bzero(action, 256);
        bzero(parameter, 256);
        fgets(action, 255, stdin);
        fgets(parameter, 255, stdin);
        action[strlen(action) - 1] = '\0';
        parameter[strlen(parameter) - 1] = '\0';


        if (!strcmp(action, "open")) {
            sockfd = ClientConect(parameter);
        }
        else if (sockfd!=0) {

            if (!strcmp(action, "close")) {
                Client(-1, NULL, sockfd);
                close(sockfd);
                sockfd = 0;

            }
            else if (!strcmp(action, "cd")) {
                Client(1, parameter, sockfd);
            }
            else if (!strcmp(action, "quit")) {
                Client(-1, NULL, sockfd);
                close(sockfd);
                return;
            }
            else if (!strcmp(action, "get")) {
                Client(2, parameter, sockfd);

            }
            else if (!strcmp(action, "lcd")) {

                char *Nuevodirectorio = parameter;
                if (dirExistens(Nuevodirectorio) == 1) {

                    CURRENTCLIENTPATH = calloc(strlen(Nuevodirectorio), sizeof(char *));
                    strcpy(CURRENTCLIENTPATH, Nuevodirectorio);
                    printf("Nuevo directorio loca: %s", CURRENTCLIENTPATH);
                }
                else {
                    printf("Directorio loca: %s, no existe", parameter);
                }

            }
            else if (!strcmp(action, "ls")) {
                Client(4, NULL, sockfd);

            }
            else if (!strcmp(action, "put")) {
                Client(3, parameter, sockfd);

            }
            else {
                //TODO: devolver la forma de uso de los argument
                printf("Argumento invalido: \n");
            }
        }
    else {
                //TODO: devolver la forma de uso de los argument
                printf("Argumento invalido: \n");
            }
    }
}



int Client(int action, char *parameter,  int sockfd) {
    char buffer[1024];

    if (action==-1) {
        //Solicita funcion 4 (ls) al server
        bzero(buffer,1024);
        strcat(buffer, "-1");
        write(sockfd, buffer, strlen(buffer));

    }


    if (action==4) {
        //Solicita funcion 4 (ls) al server
        bzero(buffer,1024);
        strcat(buffer, "4");
        write(sockfd, buffer, strlen(buffer));
        bzero(buffer, 1024);
        // Resive datos de solivitud
        read(sockfd, buffer, 1024);
        printf("Server: %s \n", buffer);

    }

    if (action==1) {
        //Solicita funcion 1 (cd) al server
        bzero(buffer,1024);
        strcat(buffer, "1");
        write(sockfd, buffer, strlen(buffer));
        bzero(buffer, 1024);
        // Resive que el server lo escuche
        read(sockfd, buffer, 1024);
        // Envia direicion de directorio
        bzero(buffer,1024);
        strcat(buffer, parameter);
        write(sockfd, buffer, strlen(buffer));
        // Imprime resultado de servidor
        bzero(buffer,1024);
        read(sockfd, buffer, 1024);
        printf("Server: %s", buffer);

    }

    if (action==2) {
        //Accion get
        sendMsgToServer(sockfd, buffer, "2");
        //Espera a estar siendo escuchado
        reciveMsgFromServer(sockfd, buffer);
        // Envia nombre del archivo
        sendMsgToServer(sockfd, buffer, parameter);
        // resive resultado de exitencia
        reciveMsgFromServer(sockfd, buffer);

        FILE *file;

        //TODO que el archivo no se cree si no existe en el server
        char filepath [1024];
        strcpy(filepath, CURRENTCLIENTPATH);
        strcat(filepath, "/");
        strcat(filepath, parameter);



        int tamanoTotal = 0;

        int tamanoDepieza;

        while (strcmp(buffer, "0")) {
            tamanoDepieza = atoi(buffer);
            file = fopen(filepath, "a");
            // Pide siguiente pedaso
            sendMsgToServer(sockfd, buffer, "11");
            // Resive pedaso
            reciveMsgFromServer(sockfd, buffer);


            //printf("Server: %s", buffer);
            printf ("\n Recibido %d",  tamanoDepieza);
            tamanoTotal += tamanoDepieza;

            fwrite(buffer, 1, tamanoDepieza, file);

            // Pregunta si falta
            sendMsgToServer(sockfd, buffer, "12");
            // Resive respuesta
            reciveMsgFromServer(sockfd, buffer);
            fclose (file);
        }
        printf ("Tamaño recibido %d", tamanoTotal);
        bzero (filepath,1024);




    }
    if (action==3){
        // Abre archivo
        FILE *file;
        char filepath [1024];
        bzero(filepath, 1024);
        strcat(filepath, CURRENTCLIENTPATH);
        strcat(filepath, "/");
        strcat(filepath, parameter);
        file = fopen(filepath, "rb");
        printf ("Sending file: %s", filepath);

        if (file!=0) {

            //Envia funcion put
            sendMsg(sockfd, buffer, "3");
            //Recibe coneccion lista1
            reciveMsg(sockfd, buffer);
            //Envia nombre de archivo2
            sendMsg(sockfd, buffer, parameter);
            //Recibe coneccion lista3
            reciveMsg(sockfd, buffer);



            int bytesLeidos;
            char filebuf[1024];
            int totalsize = 0;
            while((bytesLeidos = fread(filebuf, 1, 1000, file))){
                printf ("\n Archivo exite leyendo %d bytes", bytesLeidos);

                char sizeString[10];
                snprintf(sizeString, 10, "%d", bytesLeidos);
                // Envia tamaño de pieza4
                sendMsg(sockfd, buffer, sizeString);
                // Recive cliente listo para recivir archivo
                reciveMsg(sockfd, buffer);
                // Envia pieza de archivo
                write(sockfd,filebuf, bytesLeidos);
                bzero(filebuf,1024);
                // Resive pregunta si falta
                reciveMsg(sockfd, buffer);
                totalsize+= bytesLeidos;

            }
            printf ("\n Tamaño total %d bites", totalsize);
            fclose(file);
        }

        // Envia arhibo no abierto o terminado
        sendMsg(sockfd, buffer, "0");

    }







    return 0;


}


int ClientConect(char *HostUserPassPath) {
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1024];

    portno = atoi(PORT);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Conecta al servidor solicitado
    char *string;

    char s[1] = "@";
    string = strtok(HostUserPassPath, s);
    char *host = string;

    char * UserPassPath = strtok(NULL,s);


    server = gethostbyname(host);
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



        //Solicita funcion 0 (Pin)al server
        bzero(buffer, 1024);
        strcat(buffer, "0");
        write(sockfd, buffer, strlen(buffer));

        bzero(buffer, 1024);
        read(sockfd, buffer, 1024);


        bzero(buffer, 1024);
        strcat(buffer, UserPassPath);
        write(sockfd, buffer, strlen(buffer));


        bzero(buffer, 1024);
        read(sockfd, buffer, 1024);

        //TODO compara el output resivido, segun eso devulve o no el socket o lo cierra
        // close(sockfd);

        printf("%s\n", buffer);
        return sockfd;



}

void reciveMsgFromServer(int sockfd, char buffer[]) {
    int n;
    bzero(buffer, 1024);
    n = read(sockfd, buffer, 1024);
    if (n < 0)
        error("ERROR writing to socket");
}

void sendMsgToServer(int sockfd, char buffer[], char *parameter ) {
    int n;
    bzero(buffer,1024);
    strcat(buffer, parameter);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
}




///________________________________________________________________________________________________
////___________________________________SERVER_SIDE_________________________________________________



void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *Server() {
    int sockfd, newsockfd, portno;//, pid;
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
        newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        //pid = fork();
        //if (pid < 0)
          //  error("ERROR on fork");
        //if (pid == 0)  {
       //     close(sockfd);
            dostuff(newsockfd);
         //   exit(0);
        //}
       // else close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

/******** DOSTUFF() *********************
There is a separate instance of this function
for each connection.  It handles all communication
once a connnection has been established.
*****************************************/

void dostuff (int sock) {

// La conecion con cliente se mantiene viva
    while(1){

        int n;
        char buffer[1024];

        bzero(buffer, 1024);
        n = read(sock, buffer, 1024);
        if (n < 0) error("ERROR reading from socket");

        printf("Client request action #%s\n", buffer);



        //Es fin
         if (!strcmp(buffer, "-1")) {
            close(sock);
            return;

         }

        // ES UN PIN
        if (!strcmp(buffer, "0")) {

            write(sock,"0",1);//ENVIA 2
            bzero(buffer, 1024);
            read(sock, buffer, 1024);//RECIBE 3

            int respuesta = autenticacionOpen(buffer);

            if(respuesta != 1){
                write(sock, "ERROR user doesn't exist", 18);
            }
            else{

                write(sock, "Autentication done successfully", 18);//ENVIA 4
            }


        }

        // ES UN LS
        if (!strcmp(buffer, "4")) {
            printf("%s", CURRENTSERVERPATH);
            char *dirString = getdir(CURRENTSERVERPATH);
            printf("%s %d \n", dirString, (int) strlen(dirString));
            n = write(sock, dirString, strlen(dirString));
            if (n < 0)
                error("ERROR writing to socket");

        }


        // ES UN CD
        if (!strcmp(buffer, "1")) {
            //Solicita el nuevo directorioc
            n = write(sock, "0", 1);
            bzero(buffer, 1024);
            n = read(sock, buffer, 255);
            char *Nuevodirectorio = buffer;
            if (dirExistens(Nuevodirectorio) == 1) {

                printf("%s \n", Nuevodirectorio);
                CURRENTSERVERPATH = calloc(strlen(Nuevodirectorio), sizeof(char *));
                strcpy(CURRENTSERVERPATH, Nuevodirectorio);
                printf("%s \n", CURRENTSERVERPATH);
                n = write(sock, "Ok", 2);
            }
            else {
                n = write(sock, "Invalid directory", 17);
            }


        }

        // ES UN get
        if (!strcmp(buffer, "2")) {
            //Envia Acepta entrada
            sendMsg(sock, buffer, "1");
            //Recive nombre de archivo
            reciveMsg(sock, buffer);
            printf("%s", buffer);
            // Abre archivo
            FILE *file;
            char filepath[1024];
            strcat(filepath, CURRENTSERVERPATH);
            strcat(filepath, "/");
            strcat(filepath, buffer);
            file = fopen(filepath, "rb");
            bzero(filepath, 1024);
            printf("Archivo por abrir %s", filepath);
            if (file != 0) {

                int bytesLeidos;
                char filebuf[1024];
                int totalsize = 0;
                while ((bytesLeidos = fread(filebuf, 1, 1000, file))) {
                    printf("\n Archivo exite leyendo %d bytes", bytesLeidos);
                    // Envia tamaño de pieza
                    char sizeString[10];
                    snprintf(sizeString, 10, "%d", bytesLeidos);
                    sendMsg(sock, buffer, sizeString);
                    // Recive cliente listo para recivir archivo
                    reciveMsg(sock, buffer);
                    // Envia pieza de archivo
                    n = write(sock, filebuf, bytesLeidos);
                    bzero(filebuf, 1024);
                    // Resive pregunta si falta
                    reciveMsg(sock, buffer);
                    totalsize += bytesLeidos;

                }
                printf("\n Tamaño total %d bites", totalsize);
                fclose(file);
            }

            // Envia arhibo no abierto o terminado
            sendMsg(sock, buffer, "0");


        }

        // ES UN put
        if (!strcmp(buffer, "3")) {
            //Envia  listo1
            sendMsg(sock, buffer, "1");
            // resive nombre de archivo2
            reciveMsg(sock, buffer);
            char *filename = buffer;

            //Crea el archivo
            FILE *file;
            char filepath[1024];
            strcpy(filepath, CURRENTSERVERPATH);
            strcat(filepath, "/");
            strcat(filepath, filename);

            //Pide primer pedazo3
            sendMsg(sock, buffer, "1");
            //Resive tamaño de pedazo4
            reciveMsg(sock, buffer);
            printf ("tamaño primer pedaso %s ", buffer);

            int tamanoTotal = 0;
            int tamanoDepieza;

            while (strcmp(buffer, "0")) {
                tamanoDepieza = atoi(buffer);
                file = fopen(filepath, "a");
                // Pide pedazo
                sendMsg(sock, buffer, "11");
                // Resibe pedazo
                reciveMsg(sock, buffer);

                printf("\n Recibido %d", tamanoDepieza);
                tamanoTotal += tamanoDepieza;

                fwrite(buffer, 1, tamanoDepieza, file);

                // Pregunta tamaño siguiente pedazo
                sendMsg(sock, buffer, "12");
                // Resive respuesta
                reciveMsg(sock, buffer);
                fclose(file);
            }
            printf("Tamaño recibido %d", tamanoTotal);
            bzero(filepath, 1024);

        }
    }
}


void reciveMsg(int sock, char buffer[]) {
    int n;
    bzero(buffer,1024);
    n = read(sock,buffer,1024);
    if (n < 0)
        error("ERROR writing to socket");
}

void sendMsg(int sock, char buffer[], char *parameter) {
    int n;
    bzero(buffer,1024);
    strcat(buffer, parameter);
    n = write(sock,buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
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
