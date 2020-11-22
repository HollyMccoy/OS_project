#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h> //time function

#define MAX_CARDS 18
#define MAX_SYMBOLS 9
#define MAX_BUFFER 256
#define MAX_PLAYERS 5
#define TEST_MODE 0 // If TEST_MODE = 1 (True) Will Begin Testing TEST_MODE = 0 (False) Will Skip Testing

//===================== Cards =======================//
//structure definition
struct card{ 
  char *symbol; // this is the special symbol
  bool isFlipped; // if is flipped is true the card is symbol side up
  bool inPlay; // the card is out of play once it has already been matched 
};
typedef struct card Card;

//===================== Prototypes / Globals =======================//
void populate_deck(Card []);
void print_deck_faceup( Card deck[]); //Replaced by char * faceup_deck_to_buffer
char * faceup_deck_to_buffer(Card deck[], char buffer[]);
void print_deck(); //Replaced by char * facedown_deck_to_buffer
char * facedown_deck_to_buffer(const Card *deck, char buffer[]);
void display_welcome_message(); //Replaced in main by printf statement
void shuffle_deck();
int get_random_num(int numBeg, int numEnd);
int char_to_num_convert(char theChar);
void test(void* expected, void* actual, const char* testName); // Testing Prototype
bool validate_input(char userInput, Card thedeck[MAX_CARDS]);
bool isGameOver(Card deck[]);

// array of pointers to characters for symbols
char *symbols[MAX_SYMBOLS] = {"!", "@", "#", "$", "^", "&", "*", "+", "~"};

//===================== Main Begins... =======================//
int main () {

    if(TEST_MODE == 1){

      printf("TESTING MODE");

      srand(time(0)); 
      // "deck" is a 'struct card' or 'Card' type array
      Card deck[MAX_CARDS];


    } else {
    //Game welcome message
    char * welcome = "\n"
    "    +------------------------------+\n"
    "    |                              |\n"
    "    |      MATCHING CARD GAME      |\n"
    "    |                              |\n"
    "    +------------------------------+\n\n";
    printf(welcome);

    srand(time(0)); 
    // "deck" is a 'struct card' or 'Card' type array
    Card deck[MAX_CARDS];

    //"buffer" is an array containing messages to be sent to clients
    char buffer[MAX_BUFFER];

    //filling deck with cards
    populate_deck(deck);
    shuffle_deck(deck);
    //print_deck_faceup(deck);
    printf(faceup_deck_to_buffer(deck, buffer));

    //printf("Printing unshuffled deck:\n");
    //print_deck_faceup(deck);

    //printf("Shuffling deck\n...\n\n");
    //shuffle_deck(deck);

    //printf("Printing shuffled deck:\n");
    //print_deck_faceup(deck);

    //printf("Printing deck face down:\n");
    //print_deck(deck);

    char input;
    int cardLocation, cardLocation2;
    Card firstCard, secondCard; //these will be compared
    bool stillPlaying = true;

    bool isTakeTurns = true;
    int playerTurn = 0; //Tracks which player's turn it is; starts at 0
    int numOfPlayers = 2;
    int playerScores[MAX_PLAYERS] = {0}; //Tracks each player's score

    //------------------------------- Game Loop Begins ------------------------------------
    // this game mode is for when the players take turns
    if (isTakeTurns){
        while(stillPlaying){
            //print_deck(deck);
            printf(facedown_deck_to_buffer(deck, buffer));
            
            fflush(stdin);
              // userSelection = 'b';
              printf("Enter a letter a --> r: ");
              scanf("%c", &input);
              
              // checking user input
              bool isValid = validate_input(input, deck);
              while(!(isValid)){
                fflush(stdin); // fixes double printing the statement below
                printf("please enter a valid selection: ");
                scanf("%c", &input);
                isValid = validate_input(input, deck);
              }

              // converting user's input to a number representing a location in the array of cards
              cardLocation = char_to_num_convert(input);
              printf("%d\n\n", cardLocation);

              // locating card in array
              firstCard = deck[cardLocation];

              // flipping the card face up
              deck[cardLocation].isFlipped = true;

              //print_deck(deck);
              printf(facedown_deck_to_buffer(deck, buffer));
              
              fflush(stdin);
              // userSelection = 'b';
              printf("Enter a letter a --> r: ");
              scanf("%c", &input);
              
              // checking user input
              isValid = validate_input(input, deck);
              while(!(isValid)){
                fflush(stdin); // fixes double printing the statement below
                printf("please enter a valid selection: ");
                scanf("%c", &input);
                isValid = validate_input(input, deck);
              }

              // converting user's input to a number representing a location in the array of cards
              cardLocation2 = char_to_num_convert(input);
              printf("%d\n\n", cardLocation2);

              // locating card in array
              secondCard = deck[cardLocation2];

              // flipping card
              deck[cardLocation2].isFlipped = true;

              //reveal card on board
              //print_deck(deck);
              printf(facedown_deck_to_buffer(deck, buffer));
            
              //if both card's symbols match
              if (deck[cardLocation].symbol == deck[cardLocation2].symbol){
                printf("Match!\n");
                //taking the cards out of play
                deck[cardLocation].inPlay = false;
                deck[cardLocation2].inPlay = false;

                //Adding point to player
                playerScores[playerTurn]++;

                // function checks to see if all cards are face up, if so game over
                if(isGameOver(deck)){
                  printf("\n\nGame Over\n\n");
                  printf("Final Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", playerScores[0], playerScores[1]);
                  break;
                }
                
              }
              //If cards do not match, then we will be flipping cards back over
              else{
                printf("Try again\n");
                deck[cardLocation].isFlipped = false;  
                deck[cardLocation2].isFlipped = false;
              }
              printf("Scores\n\nPlayer 1: %d\nPlayer 2: %d\n\n", playerScores[0], playerScores[1]);

              //Rotate player turn
              if(++playerTurn == numOfPlayers)
                playerTurn = 0;
          }
    }

    }
    return 0;
}

//===================== Functions / Methods =======================//

// fills a deck with cards
void populate_deck(Card deck[]){
  int i = 0;
  for(i=0;i<MAX_CARDS;i++){
    deck[i].symbol = symbols[i%MAX_SYMBOLS];
    deck[i].isFlipped = false; // all cards start out face down
    deck[i].inPlay = true;    // all cards start in play
  }
}

// Prints deck of cards symbol-side down,
// unless the card's isFlipped is true
void print_deck(const Card deck[]){
  int i = 0;
  char letter = 97; // Using ASCII to print alphabet
  printf("               ");
  for(i=0;i<MAX_CARDS;i++){
    if(deck[i].isFlipped){
      printf(" %s  ", deck[i].symbol);
    }
    else{
      printf("[%c] ", letter);
    }
    letter++; // incrementing to next ASCII
    
    if((i == 5) || (i == 11)){
        printf("\n               ");
    }
  }
  printf("\n\n");
}

//Populates buffer deck of cards symbol-side down, unless the card's isFlipped
//is true; returns pointer to buffer for use with print statements and message sending
char * facedown_deck_to_buffer(const Card *deck, char buffer[]){
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int j = 0;
    int i;
    char letter = 97; //Using ASCII to print alphabet
    buffer[j++] = '\t';
    for( i=0; i<MAX_CARDS; i++, j++){
        if(deck[i].isFlipped){
            buffer[j++] = ' ';
            buffer[j++] = *deck[i].symbol;
            buffer[j++] = ' ';
            buffer[j] = ' ';
        }
        else{
            buffer[j++] = '[';
            buffer[j++] = letter;
            buffer[j++] = ']';
            buffer[j] = ' ';
        }
        letter++;

        if((i == 5) || (i == 11)){
            buffer[j++] = '\n';
            buffer[j] = '\t';
        }
    }
    buffer[j++] = '\n';
    buffer[j++] = '\n';
    buffer[j] = '\0';
    return buffer;
}

void print_deck_faceup( Card deck[]){
    int i;
  printf("Solution:\n               ");
  for(i=0;i<MAX_CARDS;i++){
    printf("[%s] ", deck[i].symbol);
    
    if((i == 5) || (i == 11)){
        printf("\n               ");
    }
  }
  printf("\n\n");
} 

//Populates buffer with faceup deck; returns pointer to buffer for
//use with print statements and message sending
char * faceup_deck_to_buffer(Card deck[], char buffer[]){
    //To-do: add error checks for buffer size (more important for non-deck buffer functions)
    int i = 0,j;
    buffer[i++] = '\t';
    for(j=0;j<MAX_CARDS;j++, i++){
        buffer[i++] = '[';
        buffer[i++] = *deck[j].symbol;
        buffer[i++] = ']';
        buffer[i] = ' ';

        if ((j == 5) || (j==11)){
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
bool validate_input(char userInput, Card thedeck[MAX_CARDS]){
  if(!(userInput >= 'a' && userInput <= 'r')){
        return false;
  }
  // converting user's input to a number representing a location in the array of cards
  int location = char_to_num_convert(userInput);

  // if the card has already been flipped then that's an invalid selection
  if(!(thedeck[location].inPlay)){
    return false;
  }

  if(thedeck[location].isFlipped==true){
    return false;
  }

  return true;
}

// Iterates through deck, if there is a card still face down we return false,
// if all cards are face up then we return true and the game is over
bool isGameOver(Card deck[]){
    int i;
  for(i=0;i<MAX_CARDS;++i){
    if(deck[i].isFlipped == false){
      return false;
    }
  }
  return true;
}

void display_welcome_message(){
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
int char_to_num_convert(char theChar){
  return theChar - 97;
}

// Testing Function
void test(void* expected, void* actual, const char* testName){
  if(expected == actual){
    printf("\n%s PASSED", testName);
  } else {
    printf("\n%s FAILED expected: %d actual: %d", testName, expected, actual);
  }
}

/* TODO:

decide how to display and design the board look and how cards
and locations information are sent

Associate a “value” with each location 
which has been set by the server at the beginning of the game 
(perhaps based on the difficulty of remembering the card) and unknown to the clients.

The server has the option to decide if the game should 
    1. force alternative turns for the players 
    2. or if each player plays as fast as desired.
        - If the latter option is selected the initial board will be send to both 
          clients at the same time. If both clients choose the same locations and the symbols 
          are matches, then the client who picked the second location first 
          will receive the points.

*/


//POSSIBLE JOBS
/*

Developer - game functionality
Developer - client / server
Tester

*/