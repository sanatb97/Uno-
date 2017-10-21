#include <stdio.h>

#include <stdlib.h>

#include "uno_header.h"



void main()
{
Handcard *pile = NULL;       /* head of draw pile (linked list) */
     Handcard *discard = NULL;    /* head of discard pile (linked list) */


     Handcard *plcard = NULL; /* selected card in player's hand */
     Handcard *p = NULL;      /* the generic Handcard pointer */

    struct Cardname cardname;

    /* Defaults for parameters */
    int multdraw = 0;
    int noskip = 0;
    int winscore = 500;
    int i;
	printf("Hello there,Welcome to UNO!\n");
	
    printf("For UNO rules, visit www.unorules.com\n");
	
    printf("How many players?(2-10)\n");

int plrs;
scanf("%d",&plrs);

struct Handcard *pl[plrs];      /* heads of players' hands (linked lists) */
    char *plname[plrs];             /* players' names */
    int plscore[plrs];              /* Players' scores */
    int uno[plrs]; 
    for(i=0; i<plrs; i++) 
    {
        plscore[i] = 0;
        plname[i] = (char *) calloc(13, sizeof(char));
       
        printf("Player %d, enter your name (12 chars max): \n",i+1);
        scanf("%s",(plname[i]));

}


for(i=0;i<plrs;i++)
{
	printf("Player %d:%s\n",i+1,plname[i]);
}

printf("LET'S GO!\n");

}
