


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

#define MAX_CARDS 18
#define MAX_SYMBOLS 9
#define MAX_BUFFER 256
#define MAX_PLAYERS 5
#define TEST_MODE 0 // If TEST_MODE = 1 (True) Will Begin Testing TEST_MODE = 0 (False) Will Skip Testing

//===================== Cards =======================//
//structure definition
struct card {
    char* symbol; // this is the special symbol
    bool isFlipped; // if is flipped is true the card is symbol side up
    bool inPlay; // the card is out of play once it has already been matched 
};
typedef struct card Card;
Card deck[MAX_CARDS];
//===================== Prototypes / Globals =======================//
void populate_deck(Card[]);
void print_deck_faceup(Card deck[]); //Replaced by char * faceup_deck_to_buffer
char* faceup_deck_to_buffer(Card deck[], char buffer[]);
void print_deck(); //Replaced by char * facedown_deck_to_buffer
char* facedown_deck_to_buffer(const Card* deck, char buffer[]);
void display_welcome_message(); //Replaced in main by printf statement
void shuffle_deck();
int get_random_num(int numBeg, int numEnd);
int char_to_num_convert(char theChar);
void test(void* expected, void* actual, const char* testName); // Testing Prototype
bool validate_input(char userInput, Card thedeck[MAX_CARDS]);
bool isGameOver(Card deck[]);
void doprocessing (int sock);

// array of pointers to characters for symbols
char* symbols[MAX_SYMBOLS] = { "!", "@", "#", "$", "^", "&", "*", "+", "~" };
#define PORTNUM  5001 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/




int main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int status, pid;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM,DEFAULT_PROTOCOL );

   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = PORTNUM;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/

   status =  bind(sockfd, (struct sockaddr *) &serv_addr, sizeof	(serv_addr)); 

   if (status < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   /* Now Server starts listening clients wanting to connect. No 	more than 5 clients allowed */
   
   listen(sockfd,2);
   clilen = sizeof(cli_addr);

   srand(time(0));
   // "deck" is a 'struct card' or 'Card' type array
   populate_deck(deck); // populate deck
   printf("Deck of cards created");
   print_deck_faceup(deck); // print

   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, 	&clilen);
		
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
         doprocessing(newsockfd); // run actual game in do process
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}


void doprocessing (int sock) {
   int status;
   char buffer[256];
   bzero(buffer,256); // empty buffer
   strcpy(buffer, facedown_deck_to_buffer(deck, buffer)); // copy result of facedown_deck_to_buffer into buffer
   status = write(sock, buffer, 210); // send buffer to client
   status= read(sock,buffer,255); // read any input
   

   if (status < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   printf("You entered: %s\n",buffer); //  should print what input the server recieved
   bool isValid = validate_input(buffer[0], deck); // check if input is valid
   while (!(isValid)) {
       bzero(buffer, 256); // fixes double printing the statement below
       status = write(sock, "please enter a valid selection: ", 210); // ask for valid input
       status = read(sock, buffer, 255);// read input
       isValid = validate_input(buffer[0], deck); // adjust is valid again
   }
   strcpy(buffer, facedown_deck_to_buffer(deck,buffer)); // copy into buffer again
   status = write(sock, buffer, 210); // writeto socket
   bzero(buffer, 256); // clear buffer
       
   
   
   

   if (status < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
	
}


// fills a deck with cards
void populate_deck(Card deck[]) {
    int i = 0;
    for (i = 0; i < MAX_CARDS; i++) {
        deck[i].symbol = symbols[i % MAX_SYMBOLS];
        deck[i].isFlipped = false; // all cards start out face down
        deck[i].inPlay = true;    // all cards start in play
    }
}

// Prints deck of cards symbol-side down,
// unless the card's isFlipped is true
void print_deck(const Card deck[]) {
    int i = 0;
    char letter = 97; // Using ASCII to print alphabet
    printf("               ");
    for (i = 0; i < MAX_CARDS; i++) {
        if (deck[i].isFlipped) {
            printf(" %s  ", deck[i].symbol);
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
char* facedown_deck_to_buffer(const Card* deck, char buffer[]) {
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int j = 0;
    int i;
    char letter = 97; //Using ASCII to print alphabet
    
    buffer[j++] = '\t';
    for (i = 0; i < MAX_CARDS; i++, j++) {
        if (deck[i].isFlipped) {
            buffer[j++] = ' ';
            buffer[j++] = *deck[i].symbol;
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

void print_deck_faceup(Card deck[]) {
    int i;
    printf("Solution:\n               ");
    for (i = 0; i < MAX_CARDS; i++) {
        printf("[%s] ", deck[i].symbol);

        if ((i == 5) || (i == 11)) {
            printf("\n               ");
        }
    }
    printf("\n\n");
}

//Populates buffer with faceup deck; returns pointer to buffer for
//use with print statements and message sending
char* faceup_deck_to_buffer(Card deck[], char buffer[]) {
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int i = 0, j;
    buffer[i++] = '\t';
    for (j = 0; j < MAX_CARDS; j++, i++) {
        buffer[i++] = '[';
        buffer[i++] = *deck[j].symbol;
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
void shuffle_deck(Card deck[]) {
    int i, j;
    Card temp;
    for (i = 0; i < MAX_CARDS - 1; ++i) {
        j = i + (get_random_num(0, MAX_CARDS - i));
        //swapping
        temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;

    }
}

// Returns a random number according to specifications
int get_random_num(int numBeg, int numEnd) {
    int ranNum = rand() % numEnd + numBeg;
    return ranNum;
}

//this ensures the user enters a valid input
bool validate_input(char userInput, Card thedeck[]) {
    if (!(userInput >= 'a' && userInput <= 'r')) {
        return false;
    }
    // converting user's input to a number representing a location in the array of cards
    int location = char_to_num_convert(userInput);

    // if the card has already been flipped then that's an invalid selection
    if (!(thedeck[location].inPlay)) {
        return false;
    }

    if (thedeck[location].isFlipped == true) {
        return false;
    }

    return true;
}

// Iterates through deck, if there is a card still face down we return false,
// if all cards are face up then we return true and the game is over
bool isGameOver(Card deck[]) {
    int i;
    for (i = 0; i < MAX_CARDS; ++i) {
        if (deck[i].isFlipped == false) {
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
