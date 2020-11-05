#include <stdlib.h>
#include <stdio.h>
#include <time.h> //time function
#include <string.h>

#define MAX_CARDS 18
#define MAX_SYMBOLS 9


//===================== Cards =======================//
//structure definition
struct card{ 
  char *symbol;    
};
typedef struct card Card;

//===================== Prototypes / Globals =======================//
void populate_deck(Card []);
void display_cards(const Card deck[]);

// array of pointers to characters for symbols
char *symbols[MAX_SYMBOLS] = {"!", "@", "#", "$", "^", "&", "*", "+", "x"};

//===================== Main Begins... =======================//
int main () {

    // "deck" is an array of 18 cards
    Card deck[MAX_CARDS];

    populate_deck(deck);

    printf("Printing unshuffled deck:\n");
    display_cards(deck);
    return 0;
}

// populate_decks deck of cards
void populate_deck(Card deck[]){
  int i = 0;
  for(i=0;i<MAX_CARDS;i++){
    deck[i].symbol = symbols[i%MAX_SYMBOLS];
  }
}

// Prints deck of cards
void display_cards(const Card deck[]){
  int i = 0;
  for(i=0;i<MAX_CARDS;i++){
    printf("%s", deck[i].symbol);
    printf("\n");
    }
  }

