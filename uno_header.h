
#define DECKSIZE 108
#define INITHAND 7      /* initial hand dealt (7 in standard Uno rules) */
#define CARDTOP 6
#define CARDLINES 7
#define CARDLEFT 2

/* Constants for each of the card values, which also correspond to their
 * char values to be displayed */
enum Number { ZERO = 48, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
       REVERSE = 82, SKIP, DRAWTWO = 68, WILD = 87, PLUSWILD = 43 };

/* Color constants
 * DEF is the standard dialog color pair
 * UNO is used for the Uno tag in the player stats
 * WHITE is for Wilds, and a couple other things */
enum Color { RED = 1, BLUE, GREEN, YELLOW, WHITE, DEF, UNO };

typedef struct {
    char num;
    int col;
} Card;


typedef struct Cardname {
    union {
        char *s;
        char c;
    } num;
    char *col;
}Cardname;

/* players' hands
 * Each element contains a pointer to a card,
 * as well as a link to the next card in the player's hand. */
typedef struct Handcard {
    Card const *card;
    struct Handcard *link;
}Handcard;



/* Card constants in a constant array  */
 /*const Card deck[] = {
    { ZERO, RED },
    { ONE, RED },
    { ONE, RED },
    { TWO, RED },
    { TWO, RED },
    { THREE, RED },
    { THREE, RED },
    { FOUR, RED },
    { FOUR, RED },
    { FIVE, RED },
    { FIVE, RED },
    { SIX, RED },
    { SIX, RED },
    { SEVEN, RED },
    { SEVEN, RED },
    { EIGHT, RED },
    { EIGHT, RED },
    { NINE, RED },
    { NINE, RED },
    { REVERSE, RED },
    { REVERSE, RED },
    { SKIP, RED },
    { SKIP, RED },
    { DRAWTWO, RED },
    { DRAWTWO, RED },
    { ZERO, BLUE },
    { ONE, BLUE },
    { ONE, BLUE },
    { TWO, BLUE },
    { TWO, BLUE },
    { THREE, BLUE },
    { THREE, BLUE },
    { FOUR, BLUE },
    { FOUR, BLUE },
    { FIVE, BLUE },
    { FIVE, BLUE },
    { SIX, BLUE },
    { SIX, BLUE },
    { SEVEN, BLUE },
    { SEVEN, BLUE },
    { EIGHT, BLUE },
    { EIGHT, BLUE },
    { NINE, BLUE },
    { NINE, BLUE },
    { REVERSE, BLUE },
    { REVERSE, BLUE },
    { SKIP, BLUE },
    { SKIP, BLUE },
    { DRAWTWO, BLUE },
    { DRAWTWO, BLUE },
    { ZERO, GREEN },
    { ONE, GREEN },
    { ONE, GREEN },
    { TWO, GREEN },
    { TWO, GREEN },
    { THREE, GREEN },
    { THREE, GREEN },
    { FOUR, GREEN },
    { FOUR, GREEN },
    { FIVE, GREEN },
    { FIVE, GREEN },
    { SIX, GREEN },
    { SIX, GREEN },
    { SEVEN, GREEN },
    { SEVEN, GREEN },
    { EIGHT, GREEN },
    { EIGHT, GREEN },
    { NINE, GREEN },
    { NINE, GREEN },
    { REVERSE, GREEN },
    { REVERSE, GREEN },
    { SKIP, GREEN },
    { SKIP, GREEN },
    { DRAWTWO, GREEN },
    { DRAWTWO, GREEN },
    { ZERO, YELLOW },
    { ONE, YELLOW },
    { ONE, YELLOW },
    { TWO, YELLOW },
    { TWO, YELLOW },
    { THREE, YELLOW },
    { THREE, YELLOW },
    { FOUR, YELLOW },
    { FOUR, YELLOW },
    { FIVE, YELLOW },
    { FIVE, YELLOW },
    { SIX, YELLOW },
    { SIX, YELLOW },
    { SEVEN, YELLOW },
    { SEVEN, YELLOW },
    { EIGHT, YELLOW },
    { EIGHT, YELLOW },
    { NINE, YELLOW },
    { NINE, YELLOW },
    { REVERSE, YELLOW },
    { REVERSE, YELLOW },
    { SKIP, YELLOW },
    { SKIP, YELLOW },
    { DRAWTWO, YELLOW },
    { DRAWTWO, YELLOW },
    { WILD, WHITE },
    { WILD, WHITE },
    { WILD, WHITE },
    { WILD, WHITE },
    { PLUSWILD, WHITE },
    { PLUSWILD, WHITE },
    { PLUSWILD, WHITE },
    { PLUSWILD, WHITE }
};
*/

void clean(Handcard *); /* Frees up a linked list from tail to head end, then sets head to NULL */
void shuffle(Handcard *); /* Shuffle whatever pile has been passed to it, starting at head */
int length(Handcard *);  /* Get the length of the hand passed to it */
Cardname getcardname(Cardname, Card const *);  /* Gets the name of the card as strings */
int draw(Handcard **pl,Handcard **draw,Handcard **discard); 
/* draw *
 * A more complicated function. Draws a card from the draw pile into
 * the player's hand, and also handles reshuffling deck if draw pile is empty.
 * Returns 1 if a card is successfully drawn, 0 if not */
 