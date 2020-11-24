
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <stdbool.h>
#include <time.h> //time function
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>

#define SHMKEY ((key_t) 5632)
#define MAX_CARDS 18
#define MAX_SYMBOLS 9
#define MAX_BUFFER 256
#define MAX_PLAYERS 2
#define TEST_MODE 0 // If TEST_MODE = 1 (True) Will Begin Testing TEST_MODE = 0 (False) Will Skip Testing
// array of pointers to characters for symbols
char symbols[MAX_SYMBOLS] = { '!', '@', '#', '$', '^', '&', '*', '+', '~' };
#define PORTNUM  5016 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/




//===================== Cards =======================//
//structure definition
typedef struct {
    char symbol; // this is the special symbol
    bool isFlipped; // if is flipped is true the card is symbol side up
    bool inPlay; // the card is out of play once it has already been matched 
}Card;




typedef struct
{
    int player_sock[MAX_PLAYERS];
    char buffer[255];
    Card deck[MAX_CARDS];
} shared_mem;
shared_mem* game_data;


//===================== Prototypes / Globals =======================//
void populate_deck();
void print_deck_faceup(); //Replaced by char * faceup_deck_to_buffer
char* faceup_deck_to_buffer(char buffer[]);
void print_deck(); //Replaced by char * facedown_deck_to_buffer
char* facedown_deck_to_buffer(char buffer[]);
void display_welcome_message(); //Replaced in main by printf statement
void shuffle_deck();
int get_random_num(int numBeg, int numEnd);
int char_to_num_convert(char theChar);
void test(void* expected, void* actual, const char* testName); // Testing Prototype
bool validate_input(char userInput);
bool isGameOver();
void play_game(int sock);




int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int status, pid;
    int shmid;
    int i = 0;
    char* shmadd;
    shmadd = (char*)0;

    // shared memory startup
    if ((shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    //printf("Shared memory creation was successful.");

    if ((game_data = (shared_mem*)shmat(shmid, shmadd, 0)) == (shared_mem*)-1)
    {
        perror("shmat");
        exit(0);
    }
    //printf("Shared memory creation was successful.\n");
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);


    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = PORTNUM;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/

    status = bind(sockfd, (struct sockaddr*) & serv_addr, sizeof(serv_addr));

    if (status < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now Server starts listening clients wanting to connect. No 	more than 5 clients allowed */

    listen(sockfd, 2);
    clilen = sizeof(cli_addr);

    srand(time(0));
    // "deck" is a 'struct card' or 'Card' type array
    populate_deck(); // populate deck
    //printf("Deck of cards created");
    print_deck_faceup(); // print

    while (i < MAX_PLAYERS) {
        newsockfd = accept(sockfd, (struct sockaddr*) & cli_addr, &clilen);
        game_data->player_sock[i] = newsockfd;
        i++;
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        /* Create child process */
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(sockfd);
            play_game(newsockfd); // run actual game in do process
            exit(0);
        }
        else {
            close(newsockfd);
        }

    } /* end of while */
    if ((shmctl(shmid, IPC_RMID, (struct shmid_ds*) 0)) == -1)
    {
        perror("shmctl");
        exit(-1);
    }

}


void play_game(int sock) {
    int status;
    char buffer[256];
    char score1[12];
    char score2[12];
    char turn[12];
    bzero(buffer, 256); // empty buffer
    char input;
    int cardLocation, cardLocation2;
    Card firstCard, secondCard; //these will be compared
    bool stillPlaying = true;

    bool isTakeTurns = true;
    int playerTurn = 0; //Tracks which player's turn it is; starts at 0
    int numOfPlayers = 2;
    int playerScores[MAX_PLAYERS] = { 0 }; //Tracks each player's score

    //strcpy(buffer, facedown_deck_to_buffer(deck, buffer)); // copy result of facedown_deck_to_buffer into buffer
    //status = write(sock, buffer, 210); // send buffer to client
    status = read(sock, buffer, 255); // read any input
    printf(buffer);

    if (status < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }



    //------------------------------- Game Loop Begins ------------------------------------
    // this game mode is for when the players take turns

    if (isTakeTurns) {
        while (stillPlaying) {
            /*

             print player's turn

            */

            //print_deck(deck);
            /*

             write card set up and player turn to clients

            */
            printf("player %d turn: \n", (playerTurn + 1));
            sprintf(turn, "%d", (playerTurn + 1));

            bzero(buffer, 256); // clear buffer
            strcpy(buffer, "\nPlayer ");
            strcat(buffer, turn);
            strcat(buffer, "'s turn: \n");
            status = write(sock, buffer, 255);
            bzero(buffer, 256);
            strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
            strcat(buffer, "\nplease enter a selection a-->r\n");
            status = write(sock, buffer, 255);

            //printf("%s", buffer);
            // userSelection = 'b';
            bzero(buffer, 256);


            if (status < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }


            status = read(sock, buffer, 255);
            //printf("%s", buffer);
            // checking user input
            bool isValid = validate_input(buffer[0]);
            if (isValid)
            {
                //printf("true");
            }
            else
            {
                //printf("false");
            }
            while (!(isValid))
            {
                fflush(stdin); // fixes double printing the statement below
                status = write(sock, buffer, 255);
                status = read(sock, buffer, 255);
                isValid = validate_input(buffer[0]);
            }

            // converting user's input to a number representing a location in the array of cards
            cardLocation = char_to_num_convert(buffer[0]);
            //printf("%d\n\n", cardLocation);

            // locating card in array
            firstCard = game_data->deck[cardLocation];

            // flipping the card face up
            game_data->deck[cardLocation].isFlipped = true;

            bzero(buffer, 256); // clear buffer
            status = write(sock, "11111", 255);
            //printf("\n\nwrote 1 to buffer\n\n");

            bzero(buffer, 256); // clear buffer
            strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
            strcat(buffer, "\nplease enter a selection AH a-->r\n");
            status = write(sock, buffer, 255);
            printf("%s", buffer);

            /*

            take in the second card

            */
            status = read(sock, buffer, 255);
            //printf("\n%s\n", buffer);
            // checking user input
            isValid = validate_input(buffer[0]);
            while (!(isValid))
            {
                status = write(sock, "wrong", 255);
                status = read(sock, buffer, 255);
                isValid = validate_input(buffer[0]);
            }
            status = write(sock, "1", 255);
            // converting user's input to a number representing a location in the array of cards
            cardLocation2 = char_to_num_convert(buffer[0]);
            //printf("%d\n\n", cardLocation2);

            // locating card in array
            secondCard = game_data->deck[cardLocation2];

            // flipping card
            game_data->deck[cardLocation2].isFlipped = true;

            //reveal card on board
            //print_deck(deck);
            bzero(buffer, 256); // clear buffer
            strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
            status = write(sock, buffer, 255);

            //if both card's symbols match
            if (game_data->deck[cardLocation].symbol == game_data->deck[cardLocation2].symbol) {
                // status = write(sock, "\nMatch!\n", 255);
                strcpy(buffer, "\nMatch!\n");
                //taking the cards out of play
                game_data->deck[cardLocation].inPlay = false;
                game_data->deck[cardLocation2].inPlay = false;

                //Adding point to player
                playerScores[playerTurn]++;

                // function checks to see if all cards are face up, if so game over
                if (isGameOver()) {
                    strcat(buffer, "\n\nGame Over\n\nFinal Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n");
                    printf("\n\nGame Over\n\n");
                    printf("Final Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", playerScores[0], playerScores[1]);
                    break;
                }

            }
            //If cards do not match, then we will be flipping cards back over
            else {
                strcpy(buffer, "Try again\n");
                //printf("Try again\n");
                game_data->deck[cardLocation].isFlipped = false;
                game_data->deck[cardLocation2].isFlipped = false;
            }
            printf("Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", playerScores[0], playerScores[1]);
            bzero(buffer, 256); // clear buffer
            sprintf(score1, "%d", playerScores[0]);
            sprintf(score2, "%d", playerScores[1]);
            strcat(buffer, "Scores\n\nPlayer 1: ");
            strcat(buffer, score1);
            strcat(buffer, "\nPlayer 2: ");
            strcat(buffer, score2);
            strcat(buffer, "\n\n");


            status = write(sock, buffer, 255);

            //Rotate player turn
            if (++playerTurn == numOfPlayers)
                playerTurn = 0;
        }
    }





}


// fills a deck with cards
void populate_deck() {
    int i = 0;
    for (i = 0; i < MAX_CARDS; i++) {
        game_data->deck[i].symbol = symbols[i % MAX_SYMBOLS];
        game_data->deck[i].isFlipped = false; // all cards start out face down
        game_data->deck[i].inPlay = true;    // all cards start in play
    }
}

// Prints deck of cards symbol-side down,
// unless the card's isFlipped is true
void print_deck() {
    int i = 0;
    char letter = 97; // Using ASCII to print alphabet
    printf("               ");
    for (i = 0; i < MAX_CARDS; i++) {
        if (game_data->deck[i].isFlipped) {
            printf(" %c  ", game_data->deck[i].symbol);
        }
        else {
            printf("[%c] ", letter);
        }
        letter++; // incrementing to next ASCII

        if ((i == 5) || (i == 11)) {
            printf("\n               ");
        }
    }
    printf("\n\n");
}

//Populates buffer deck of cards symbol-side down, unless the card's isFlipped
//is true; returns pointer to buffer for use with print statements and message sending
char* facedown_deck_to_buffer(char buffer[]) {
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int j = 0;
    int i;
    char letter = 97; //Using ASCII to print alphabet

    buffer[j++] = '\t';
    for (i = 0; i < MAX_CARDS; i++, j++) {
        if (game_data->deck[i].isFlipped) {
            buffer[j++] = ' ';
            buffer[j++] = game_data->deck[i].symbol;
            buffer[j++] = ' ';
            buffer[j] = ' ';
        }
        else {
            buffer[j++] = '[';
            buffer[j++] = letter;
            buffer[j++] = ']';
            buffer[j] = ' ';
        }
        letter++;

        if ((i == 5) || (i == 11)) {
            buffer[j++] = '\n';
            buffer[j] = '\t';
        }
    }
    buffer[j++] = '\n';
    buffer[j++] = '\n';
    buffer[j] = '\0';
    return buffer;
}

void print_deck_faceup() {
    int i;
    printf("Solution:\n               ");
    for (i = 0; i < MAX_CARDS; i++) {
        printf("[%c] ", game_data->deck[i].symbol);

        if ((i == 5) || (i == 11)) {
            printf("\n               ");
        }
    }
    printf("\n\n");
}

//Populates buffer with faceup deck; returns pointer to buffer for
//use with print statements and message sending
char* faceup_deck_to_buffer(char buffer[]) {
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int i = 0, j;
    buffer[i++] = '\t';
    for (j = 0; j < MAX_CARDS; j++, i++) {
        buffer[i++] = '[';
        buffer[i++] = game_data->deck[j].symbol;
        buffer[i++] = ']';
        buffer[i] = ' ';

        if ((j == 5) || (j == 11)) {
            buffer[i++] = '\n';
            buffer[i] = '\t';
        }
    }
    buffer[i++] = '\n';
    buffer[i++] = '\n';
    buffer[i] = '\0';
    return buffer;
}

// Shuffles cards using Fisher Yates Shuffle Algorithm
void shuffle_deck() {
    int i, j;
    Card temp;
    for (i = 0; i < MAX_CARDS - 1; ++i) {
        j = i + (get_random_num(0, MAX_CARDS - i));
        //swapping
        temp = game_data->deck[i];
        game_data->deck[i] = game_data->deck[j];
        game_data->deck[j] = temp;

    }
}

// Returns a random number according to specifications
int get_random_num(int numBeg, int numEnd) {
    int ranNum = rand() % numEnd + numBeg;
    return ranNum;
}

//this ensures the user enters a valid input
bool validate_input(char userInput) {
    if (!(userInput >= 'a' && userInput <= 'r')) {
        return false;
    }
    // converting user's input to a number representing a location in the array of cards
    int location = char_to_num_convert(userInput);

    // if the card has already been flipped then that's an invalid selection
    if (!(game_data->deck[location].inPlay)) {
        return false;
    }

    if (game_data->deck[location].isFlipped == true) {
        return false;
    }

    return true;
}

// Iterates through deck, if there is a card still face down we return false,
// if all cards are face up then we return true and the game is over
bool isGameOver()
{
    int i;
    for (i = 0; i < MAX_CARDS; ++i) {
        if (game_data->deck[i].isFlipped == false) {
            return false;
        }
    }
    return true;
}

void display_welcome_message() {
    printf("\n");
    printf("    +------------------------------+\n");
    printf("    |                              |\n");
    printf("    |      MATCHING CARD GAME      |\n");
    printf("    |                              |\n");
    printf("    +------------------------------+\n\n");
}

// lowercase 'a' in ASCII is 97,
// so this function uses the formula to returns a number 
// represents index of letter in array
int char_to_num_convert(char theChar) {
    return theChar - 97;
}

// Testing Function
void test(void* expected, void* actual, const char* testName) {
    if (expected == actual) {
        printf("\n%s PASSED", testName);
    }
    else {
        printf("\n%s FAILED expected: %d actual: %d", testName, expected, actual);
    }
}
