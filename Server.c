
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
#include<pthread.h>
#define SHMKEY ((key_t) 5632)
#define MAX_CARDS 18
#define MAX_SYMBOLS 9
#define MAX_BUFFER 256
#define MAX_PLAYERS 6
#define TEST_MODE 0 // If TEST_MODE = 1 (True) Will Begin Testing TEST_MODE = 0 (False) Will Skip Testing
// array of pointers to characters for symbols
char symbols[MAX_SYMBOLS] = { '!', '@', '#', '$', '^', '&', '*', '+', '~' };
#define PORTNUM  5019 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
pthread_mutex_t mutex;



//===================== Cards =======================//
//structure definition
typedef struct {
    char symbol; // this is the special symbol
    bool isFlipped; // if is flipped is true the card is symbol side up
    bool inPlay; // the card is out of play once it has already been matched 
}Card;




typedef struct
{
    //shared players, set by first person
    // number of players currently connected
    int playerScores[MAX_PLAYERS]; // = { 0 }
    int player_sock[MAX_PLAYERS];
    int expPlayers; // = 0; Number of players expected to join
    int numOfPlayers; // = 0; Number of players currently connected (Note: need to delete declaration in play game function)
    int playerTurn;
    bool isTakeTurns; 
    char buffer[MAX_BUFFER];
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
void reset_deck(); //Reset deck after game ends
int get_random_num(int numBeg, int numEnd);
int char_to_num_convert(char theChar);
void test(void* expected, void* actual, const char* testName); // Testing Prototype
bool validate_input(char userInput);
bool isGameOver();
bool play_game(int sock);




int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int status, pid;
    int shmid;
    int i = 0;
    char* shmadd;
    shmadd = (char*)0;
    pthread_mutex_init(&mutex, NULL);


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
    printf("How many people will be playing?\n");
    scanf("%d", &game_data->expPlayers);
    while ((getchar()) != '\n'); 
    char choice;
    printf("Enter \"y\" to force turns (press enter for free play): ");
    while ((getchar()) != '\n'); 
    scanf("%c",choice);
    if (choice == 'y')
        game_data->isTakeTurns = true;
    //Initialize shared memory (game_data) fields to default values
    game_data->numOfPlayers = 0;

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

    // shuffle_deck(); // Shuffling Deck

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
            bool playAgain = true;
            while(playAgain)
                playAgain = play_game(newsockfd); // run actual game in do process
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


bool play_game(int sock) {
    int status;
    char buffer[256];
    char lazyBuffer[100];
    char tempString[50]; //Used with sprintf and strcat to append text to buffer
    char turn[12];
    bzero(buffer, 256); // empty buffer
    char input;
    int cardLocation, cardLocation2;
    int currPlayer; //The player number for the given client connection
    int i = 0; //Loop counter
    Card firstCard, secondCard; //these will be compared
    bool stillPlaying = true;
    bool playAgain = false;
    bool isTakeTurns = false;
    int playerTurn = 0; //Tracks which player's turn it is; starts at 0
    pthread_mutex_init(&mutex, NULL);

    game_data->playerTurn = 0;
    for (i =0; i < MAX_PLAYERS; i++) //Initialize scores; ***will move later***
        game_data->playerScores[i] = 0; 
    


    //strcpy(buffer, facedown_deck_to_buffer(deck, buffer)); // copy result of facedown_deck_to_buffer into buffer
    //status = write(sock, buffer, 210); // send buffer to client
    status = read(sock, buffer, 255); // read any input
    printf(buffer);
    pthread_mutex_lock(&mutex);
    currPlayer = game_data->numOfPlayers++;
    pthread_mutex_unlock(&mutex);

    while (game_data->expPlayers != game_data->numOfPlayers) { //State: Game start OR Game restart
        //Ensure at least 2 clients connected and that all expected players have entered "ready"
        
        strcpy(buffer, "2\nWaiting on other players...");
        sprintf(tempString, "(%d/", game_data->numOfPlayers);
        strcat(buffer, tempString);
        sprintf(tempString, "%d) players have joined... press enter to refresh\n\n", game_data->expPlayers);
        strcat(buffer, tempString);
        status = write(sock, buffer, 255);
        bzero(buffer, 256);
        status = read(sock, buffer, 255);
        
    }

    if (status < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }



    //------------------------------- Game Loop Begins ------------------------------------
    // this game mode is for when the players take turns

    if (false && isTakeTurns) { //We may be able to consolidate both modes into one loop
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
            bzero(buffer, 256);


            if (status < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }

            // userSelection = 'b';
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
            strcat(buffer, "\nplease enter a selection a-->r\n");
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
                game_data->playerScores[playerTurn]++;

                // function checks to see if all cards are face up, if so game over
                if (isGameOver()) {
                    strcat(buffer, "\n\nGame Over\n\nFinal Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n");
                    printf("\n\nGame Over\n\n");
                    printf("Final Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", game_data->playerScores[0], game_data->playerScores[1]);
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
            printf("Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", game_data->playerScores[0], game_data->playerScores[1]);
            bzero(buffer, 256); // clear buffer
            sprintf(tempString, "%d", game_data->playerScores[0]);
            strcat(buffer, "Scores\n\nPlayer 1: ");
            strcat(buffer, tempString);
            sprintf(tempString, "%d", game_data->playerScores[1]);
            strcat(buffer, "\nPlayer 2: ");
            strcat(buffer, tempString);
            strcat(buffer, "\n\n");


            status = write(sock, buffer, 255);

            //Rotate player turn
            if (++playerTurn == game_data->numOfPlayers)
                playerTurn = 0;
        }
    }
    else { //Non-turn mode
        int cardsSelected = 0; //Number of cards currently selected by client
        /*numOfPlayers should be placed in shared memory and track the number of connected players
         *Also, only allow new client game connections while numOfPlayers < 5*/
        bool isValid = false;
        while (true) {
            if (!stillPlaying) {
                reset_deck(); //Placed here in anticipation for an option to change game modes
                if (buffer[0] == 'y') { //Replay this game mode
                    playAgain = true;
                    strcpy(buffer, "\nPress enter to continue...\n");
                    status = write(sock, buffer, 255);
                    game_data->numOfPlayers--;
                    break; //Skips write and read for this loop and starts game reset
                    //Reset game conditions
                    //Will need to alter code to offer ability to switch game modes
                    //==> Use a code to pick between a) Play this game mode again b) Switch modes, or c) Quit (or "Enter any other key to quit")
                }
                else //Client response was to end game
                    break;
            }
            else if (isGameOver()) {
                //Create game over buffer with final scores
                //Also, include prompt to play another game
                stillPlaying = false;
                bzero(buffer, MAX_BUFFER);
                buffer[0] = '9'; //Code that server has sent game over message
                strcat(buffer, "\n----GAME OVER----\nFinal Scores: \n");
                //Adding each player's score to buffer, adjusted for number of players
                for (i = 0; i < game_data->numOfPlayers; i++) {
                    sprintf(tempString, "Player %d: %d\n", i+1, game_data->playerScores[i]);
                    strcat(buffer, tempString);
                }
                strcat(buffer, "\n");
                strcat(buffer, "Would you like to play again?\n if so, enter \"yes\"\n");
                // write buffer, receive response, change continuePlaying to true if continue playing is selected or leave continuePlaying as is 
            }
           //else if (game_data->expPlayers != game_data->numOfPlayers) { //State: Game start OR Game restart
           //    //Ensure at least 2 clients connected and that all expected players have entered "ready"
           //    
           //    //Possible issues on client side code: what do they write back to the above message to continue?
           //    //==> Could use a non-user message as an automatic client response, but this could lead to many quick print statements
           //    //==> Or, could simply add to the above message "...Press Enter to refresh."
           //    //==> what if we remove the idea of a ready message and just dont send out the deck of cards till all players have connected?
           //    // message to buffer example: "Waiting on other players...(3/5) players have joined..."

           //    strcpy(buffer, "\nWaiting on other players...");
           //    sprintf(tempString, "(%d/", game_data->numOfPlayers);
           //    strcat(buffer, tempString);
           //    sprintf(tempString, "%d) players have joined...\n\n", game_data->expPlayers);
           //    strcat(buffer, tempString);
           //    continue; //stub
           //}
            else if (isTakeTurns && (currPlayer != playerTurn)) {
                printf("\nShouldn't be here yet--In development\n"); //Delete later
                break; //Delete later
                //Will send code (e.g., '2') telling client to wait for its turn
            }
            else if (game_data->isTakeTurns && (currPlayer != game_data->playerTurn)) {
                bzero(buffer, MAX_BUFFER);
                buffer[0] = '2';
                strcat(buffer, "\nWaiting for your turn...\n");
                //Will send code (e.g., '2') telling client to wait for its turn
            }
            else if (cardsSelected == 0) {
                //Create buffer that prompts for card 1
                cardsSelected++;
                bzero(buffer, 256); // clear buffer
                //strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
                if (game_data->isTakeTurns) {
                    strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
                    sprintf(tempString, "Player %d", currPlayer+1);
                    strcat(buffer, tempString);
                }
                strcat(buffer, "\nPlease enter first selection a-->r\n");
            }
            else if (cardsSelected == 1) {
                isValid = validate_input(buffer[0]);
                if (!isValid) {
                    bzero(buffer, 256); // clear buffer
                    strcpy(buffer, "You have entered an invalid response please try again");
                    //strcat(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
                    strcat(buffer, "\nPlease enter first selection a-->r\n");
                }
                else { //First card is valid
                    cardLocation = char_to_num_convert(buffer[0]);
                    game_data->deck[cardLocation].isFlipped = true;
                    //Create buffer that prompts for card 2
                    cardsSelected++;
                    bzero(buffer, 256); // clear buffer
                    strcpy(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
                    strcat(buffer, "\nPlease enter second selection a-->r\n");
                }
            }
            else if (cardsSelected == 2) {
                isValid = validate_input(buffer[0]);
                if (!isValid) {
                    //Create buffer that re-prompts for card 2
                    bzero(buffer, 256); // clear buffer
                    strcpy(buffer, "You have entered an invalid response please try again");
                    //strcat(buffer, facedown_deck_to_buffer(buffer)); // copy into buffer again
                    strcat(buffer, "\nPlease enter second selection a-->r\n");
                }
                else { //Second card is valid
                    cardLocation2 = char_to_num_convert(buffer[0]);
                    game_data->deck[cardLocation2].isFlipped = true;
                    cardsSelected = 0; //Reset state
                    
                    game_data->playerTurn++; //Prepare for next turn
                    if (game_data->playerTurn >= game_data->numOfPlayers)
                        game_data->playerTurn = 0;

                    //Check for match and create corresponding buffer message
                    /*If cards match, then updateScores(currPlayer);
                     *updateScores function represents critical section*/
                    pthread_mutex_lock(&mutex);
                    bzero(buffer, 256);
                    buffer[0] = '1'; //Code for client to send dummy reply that skips user input
                    strcat(buffer, facedown_deck_to_buffer(lazyBuffer));
                    if (game_data->deck[cardLocation].symbol == game_data->deck[cardLocation2].symbol) {
                        strcat(buffer, "\nMatch!\n");
                        //taking the cards out of play
                        game_data->deck[cardLocation].inPlay = false;
                        game_data->deck[cardLocation2].inPlay = false;

                        //Adding point to player
                        game_data->playerScores[currPlayer]++;

                    }
                    //If cards do not match, then we will be flipping cards back over
                    else {
                        strcat(buffer, "\nTry again\n");
                        //printf("Try again\n");
                        game_data->deck[cardLocation].isFlipped = false;
                        game_data->deck[cardLocation2].isFlipped = false;
                    }
                    pthread_mutex_unlock(&mutex);
                }
            }
            else {
                /* Unexpected State: Print state information to server console and
                 * inform client of the error (include program end code)*/
                bzero(buffer, 256);
                buffer[0] = '0'; //Code to end client game loop; concatenate rest of message
                strcat(buffer, "Unexpected State\n");
                status = write(sock, buffer, 255); //Bad message
                break;
            }
            status = write(sock, buffer, 255);
            status = read(sock, buffer, 255);
        }
    }




    return playAgain;
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

void reset_deck() {
    int i;
    for (i = 0; i < MAX_CARDS; ++i) {
        game_data->deck[i].inPlay = true;
        game_data->deck[i].isFlipped = false;
    }
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
