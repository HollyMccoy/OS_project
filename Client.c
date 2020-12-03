
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <stdbool.h>
#define PORTNUM  5016 /* the port number that the server is listening to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
void Play(int socketid);
void take_card_input(int socketid);
void read_socket(int socketid);

int main()


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

    /* before connecting the socket we need to set up the right values in the different fields of the structure server_addr
    you can check the definition of this structure on your own*/

    server = gethostbyname("osnode11"); //osnode10

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

    printf("%s", welcome);

    Play(socketid);


    /* this closes the socket*/
    close(socketid);

    return 0;
}

/*
Play Function
*/
void Play(int socketid)
{
    int status;
    bool stillPlaying = true; // Game status
    bool my_turn = true;
    bool checkInput = false;
    bool waiting = true;
    char buffer[256];


    //take first answer
    bzero(buffer, 256); // clear buffer
    fgets(buffer, 255, stdin); // place input into buffer
    // printf("buffer: %s", buffer); Testing Purposes

    status = write(socketid, buffer, strlen(buffer)); // Writes buffer to server, returns status
    bzero(buffer, 256); // clears buffer

    if (status < 0) // Checking for error
    {
        printf("error while sending client message to server\n");
    }




    while (stillPlaying) // check if still playing. still playing should not adjust and game should continue without end
    {
        if (my_turn)
        {
            int status;
            char buffer[256];
            bzero(buffer, 256); // clearing buffer
            status = read(socketid, buffer, 255); // reads buffer from server, returns status, -1 returns error
            printf("\n%s \n", buffer); //***Might need two buffers--one each for read and write--to better manage state checking messages

            if (status < 0) // Checking for error
            {
                perror("error while reading message from server");
                exit(1);
            }

            //Should probably create a char code, so we can use one bzero
            //char code = buffer[0]; //But declared before the loop

            /*State checking and management code here - Chase */
            if (buffer[0] == '0') {
                my_turn = false;
                break;
            }
            else if (buffer[0] == '1') {
                //Skipping user input after checking for card match
                bzero(buffer, 256);
                strcpy(buffer, "\nReceived '1'\n");
            }
            else {
                if (buffer[0] == '9') //Server has sent game over message with option to restart
                    checkInput = true;
                
                //User enters input into buffer
                bzero(buffer, 256); // clear buffer
                fgets(buffer, 255, stdin); // place input into buffer

                if (checkInput && (buffer[0] != 'y'))
                    stillPlaying = false;
            }

            status = write(socketid, buffer, strlen(buffer));
            if (status < 0)
            {
                printf("error while sending client message to server\n");
            }

            //Comment out the code below
            /*
            status = write(socketid, buffer, 255);
            //read card layout at the moment

            read_socket(socketid);
            read_socket(socketid);

            //pick card 1

            take_card_input(socketid);
            //pick card 2

            take_card_input(socketid);

            //print score after round

            read_socket(socketid);
            */


            /*
            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);
            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);
            status = read(socketid, buffer, 255);
            printf("%s \n", buffer);
            */

        }
    }

}

// void read_socket(int socketid)
// {
//     int status;
//     char buffer[256];
//     bzero(buffer, 256);
//     status = read(socketid, buffer, 255);
//     printf("\n%s \n", buffer); // Testing

//     if (status < 0)
//     {
//         perror("error while reading message from server");
//         exit(1);
//     }
// }

// void take_card_input(int socketid)
// {
//     int status;
//     char buffer[256];

    
//     /*
    

//      write card choice to server - Chase

//     */
//     bzero(buffer, 256); // clear buffer
//     fgets(buffer, 255, stdin); // place input into buffer
//     //printf("\n%s\n", buffer); // Testing
//     status = write(socketid, buffer, strlen(buffer));
//     if (status < 0)
//     {
//         printf("error while sending client message to server\n");
//     }
//     bzero(buffer, 256); // clear buffer
//     /*

    

//      read validation input - Chase

//     */
//     status = read(socketid, buffer, 255);
//     //printf("\n%s\n", buffer); // Testing


    

//     while (buffer[0] != '1')
//     {
//         bzero(buffer, 256); // clear buffer
//         fgets(buffer, 255, stdin); // place input into buffer
//         // printf("\n%s\n", buffer); // Testing

//         status = write(socketid, buffer, strlen(buffer));
//         if (status < 0)
//         {
//             printf("error while sending client message to server\n");
//         }
//         bzero(buffer, 256);

//         status = read(socketid, buffer, 255);

//     }
//     /*


//      read card layout - Chase


//     */
//     bzero(buffer, 256); // clear buffer
//     status = read(socketid, buffer, 255);
//     printf("\n%s \n", buffer);

//     if (status < 0) {
//         perror("error while reading message from server");
//         exit(1);
//     }




// }

