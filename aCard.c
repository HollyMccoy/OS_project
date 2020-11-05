#include "aCard.h"

void aCard::SetSymbol(string theSuit) {
    suit = theSuit;
}

string aCard::GetSymbol() {
    return brand;
}

void aCard::Print() {
    printf("%c", symbol);
}