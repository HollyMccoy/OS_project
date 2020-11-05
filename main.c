#include <stdlib.h>
#include <stdio.h>
#include <time.h> //time function
#include <string.h>

#define MAX_CARDS 18
#define MAX_SYMBOLS 9


//===================== Cards =======================//
//structure definition
struct card{ 
  char *symbol; // this is the special symbol
};
typedef struct card Card;

//===================== Prototypes / Globals =======================//
void populate_deck(Card []);
void display_cards_faceUp(const Card deck[]);
void display_welcome_message();
void shuffle_deck();
int get_random_num(int numBeg, int numEnd);

// array of pointers to characters for symbols
char *symbols[MAX_SYMBOLS] = {"!", "@", "#", "$", "^", "&", "*", "+", "x"};



//===================== Main Begins... =======================//
int main () {

    display_welcome_message();

    srand(time(0)); 
    // "deck" is an array of 18 cards
    Card deck[MAX_CARDS];

    populate_deck(deck);

    printf("Printing unshuffled deck:\n");
    display_cards_faceUp(deck);

    printf("Shuffling deck\n");
    shuffle_deck(deck);

    printf("Printing shuffled deck:\n");
    display_cards_faceUp(deck);
    return 0;
}


//===================== Functions / Methods =======================//
// fills a deck with cards
void populate_deck(Card deck[]){
  int i = 0;
  for(i=0;i<MAX_CARDS;i++){
    deck[i].symbol = symbols[i%MAX_SYMBOLS];
  }
}

// Prints deck of cards
void display_cards_faceUp(const Card deck[]){
  int i = 0;
  for(i=0;i<MAX_CARDS;i++){
    printf("[%s] ", deck[i].symbol);
    
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

