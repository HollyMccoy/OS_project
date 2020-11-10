#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h> //time function

#define MAX_CARDS 18
#define MAX_SYMBOLS 9

//===================== Cards =======================//
//structure definition
struct card{ 
  char *symbol; // this is the special symbol
  bool isFlipped; // if is flipped is true the card is symbol side up
};
typedef struct card Card;

//===================== Prototypes / Globals =======================//
void populate_deck(Card []);
void display_cards_faceUp(const Card deck[]);
void display_cards_faceDown();
void display_welcome_message();
void shuffle_deck();
int get_random_num(int numBeg, int numEnd);
int char_to_num_convert(char theChar);

// array of pointers to characters for symbols
char *symbols[MAX_SYMBOLS] = {"!", "@", "#", "$", "^", "&", "*", "+", "~"};

//===================== Main Begins... =======================//
int main () {

    display_welcome_message();

    srand(time(0)); 
    // "deck" is a 'struct card' or 'Card' type array
    Card deck[MAX_CARDS];

    //filling deck with cards
    populate_deck(deck);

    //printf("Printing unshuffled deck:\n");
    //display_cards_faceUp(deck);

    //printf("Shuffling deck\n...\n\n");
    //shuffle_deck(deck);

    //printf("Printing shuffled deck:\n");
    //display_cards_faceUp(deck);

    //printf("Printing deck face down:\n");
    //display_cards_faceDown(deck);

    char input;
    int cardLocation, cardLocation2;
    Card firstCard, secondCard; //these will be compared
    bool stillPlaying = true;

    // Game Loop
    while(stillPlaying){
      display_cards_faceDown(deck);
     
     fflush(stdin);
      //userSelection = 'b';
      printf("Enter a letter a --> r: ");
      scanf("%c", &input);
      
      //Validating Proper Input
      while(!(input >= 'a' && input <= 'r')){
        fflush(stdin); // fixes double printing the statement below
        printf("please enter a valid selection: ");
        scanf("%c", &input);
      }

      //converting char to array index loc
      cardLocation = char_to_num_convert(input);
      printf("%d\n\n", cardLocation);

      //locating card in array
      firstCard = deck[cardLocation];

      deck[cardLocation].isFlipped = true;

      //reveal card on board
      display_cards_faceDown(deck);
      
      //Getting second card
      fflush(stdin);
      //userSelection = 'b';
      printf("Enter a letter a --> r: ");
      scanf("%c", &input);
      
      //Validating Proper Input
      while(!(input >= 'a' && input <= 'r')){
        fflush(stdin); // fixes double printing the statement below
        printf("please enter a valid selection: ");
        scanf("%c", &input);
      }

      //converting char to array index loc
      cardLocation2 = char_to_num_convert(input);
      printf("%d\n\n", cardLocation2);

      //locating card in array
      secondCard = deck[cardLocation2];

      deck[cardLocation2].isFlipped = true;

      //reveal card on board
      display_cards_faceDown(deck);
    

      //if both card's symbols match
      if (deck[cardLocation].symbol == deck[cardLocation2].symbol){
        printf("Match!\n");
        
      }
      //Flipping cards back over if they don't match
      else{
        printf("Try again\n");
        deck[cardLocation].isFlipped = false;  
        deck[cardLocation2].isFlipped = false;
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
    deck[i].isFlipped = false;
  }

  for(i=0;i<MAX_CARDS;i++){
    printf("[%s] ", deck[i].symbol);
    
    if((i == 5) || (i == 11)){
        printf("\n");
    }
  }
  printf("\n\n");
}

// Prints deck of cards symbol-side down,
// unless the card's isFlipped is true
void display_cards_faceDown(const Card deck[]){
  int i = 0;
  char letter = 97; // Using ASCII to print alphabet
  for(i=0;i<MAX_CARDS;i++){
    if(deck[i].isFlipped){
      printf("[%s] ", deck[i].symbol);
    }
    else{
      printf("[%c] ", letter);
    }
    letter++; // incrementing to next ASCII
    
    if((i == 5) || (i == 11)){
        printf("\n");
    }
  }
  printf("\n\n");
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