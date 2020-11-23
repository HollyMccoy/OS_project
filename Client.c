
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <stdbool.h>
#define PORTNUM  5002 /* the port number that the server is listening to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
void Play(int socketid);
void take_card_input(int socketid);

void main()


{
    int  port;
    int  socketid;      /*will hold the id of the socket created*/
    int  status;        /* error status holder*/
     
    struct sockaddr_in serv_addr;
    struct hostent* server;

    /* this creates the socket*/
    socketid = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (socketid < 0)
    {
        printf("error in creating client socket\n");
        exit(-1);
    }

    printf("created client socket successfully\n");

    /* before connecting the socket we need to set up the right     	values in the different fields of the structure server_addr
    you can check the definition of this structure on your own*/

    server = gethostbyname("osnode10");

    if (server == NULL)
    {
        printf(" error trying to identify the machine where the 	server is running\n");
        exit(0);
    }

    port = PORTNUM;
    /*This function is used to initialize the socket structures with null values. */
    bzero((char*)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr,
        (char*)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(port);

    /* connecting the client socket to the server socket */

    status = connect(socketid, (struct sockaddr*) & serv_addr,
        sizeof(serv_addr));
    if (status < 0)
    {
        printf(" error in connecting client socket with server	\n");
        exit(-1);
    }

    printf("connected client socket to the server socket \n");

    /* now lets send a message to the server. the message will be
      whatever the user wants to write to the server.*/

    char* welcome = "\n"
        "    +------------------------------+\n"
        "    |                              |\n"
        "    |      MATCHING CARD GAME      |\n"
        "    |                              |\n"
        "    +------------------------------+\n\n"
        "    Enter \"ready\" to begin playing\n\n";
    printf(welcome);
   
    Play(socketid);


    /* this closes the socket*/
    close(socketid);


}

void Play(int socketid)
{
    int status;
    bool stillPlaying = true;
    bool my_turn = true;
    bool waiting = true;
    char buffer[256];

    while (stillPlaying) // check if still playing. still playing should not adjust and game should continue without end
    {
        if (my_turn)
        {
            //take first answer
            bzero(buffer, 256); // clear buffer
            fgets(buffer, 255, stdin); // place input into buffer
            printf("buffer: %s", buffer);

            status = write(socketid, buffer, strlen(buffer));
            bzero(buffer, 256);
            
            if (status < 0)
            {
                printf("error while sending client message to server\n");
            }


            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);
           
            if (status < 0) {
                perror("error while reading message from server");
                exit(1);
            }



            /* 
            
            pick card 1
            
            */

            take_card_input(socketid);

            /*
            
            pick card 2
            
            */


            take_card_input(socketid);


            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);

            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);

            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);

        }
    }

}



void take_card_input(int socketid)
{
    int status;
    char buffer[256];

    bzero(buffer, 256); // clear buffer
    fgets(buffer, 255, stdin); // place input into buffer
    printf("buffer: %s", buffer);

    status = write(socketid, buffer, strlen(buffer));
    if (status < 0)
    {
        printf("error while sending client message to server\n");
    }
    bzero(buffer, 256);

    status = read(socketid, buffer, 255);
    printf("%s", buffer);
    while (buffer[0] != '1')
    {
        bzero(buffer, 256); // clear buffer
        fgets(buffer, 255, stdin); // place input into buffer
        printf("buffer: %s", buffer);

        status = write(socketid, buffer, strlen(buffer));
        if (status < 0)
        {
            printf("error while sending client message to server\n");
        }
        bzero(buffer, 256);

        status = read(socketid, buffer, 255);
    }

    status = read(socketid, buffer, 255);
    printf("%s \n", buffer);

    if (status < 0) {
        perror("error while reading message from server");
        exit(1);
    }


}

