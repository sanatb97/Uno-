#include "uno_header.h"
#include <stdio.h>
#include <stdlib.h>
#include<time.h>
#include<string.h>
struct Cardname getcardname( Card *card)
{
	struct Cardname cardname;
    /* set color string */
    switch(card->col) {
    case RED:
        strcpy(cardname.col,"Red ");
        break;
    case BLUE:
        strcpy(cardname.col ,"Blue ");
        break;
    case GREEN:
        strcpy(cardname.col , "Green ");
        break;
    case YELLOW:
        strcpy(cardname.col ,"Yellow ");
        break;
    
    }


    /* ...and now name string */
    switch(card->num) {
    
    case SKIP:
        strcpy(cardname.num.s , "Skip");cardname.num.c ='\0';
        break;
    case DRAWTWO:
        strcpy(cardname.num.s , "Draw Two");cardname.num.c ='\0';
        break;
    
    default:
        cardname.num.c = card->num;strcpy(cardname.num.s , "\0");
        break;
    }

    return cardname;
}


/* clean *
 * Frees up a linked list from tail to head end, then sets head to NULL */
void clean(Handcard *head)
{
    if(head == NULL) /* don't try cleaning a null list */
        return;

     Handcard *p;
    while(head->link != NULL) {
        for(p=head; p->link->link != NULL; p = p->link)
            ;
        free(p->link);
        p->link = NULL;
    }
    free(head);
    head = NULL;
}


/* length *
 * Get the length of the hand passed to it */
int length(Handcard *head)
{
    Handcard *current;
    int count = 0;

    if(head == NULL)
        return 0;
    for(current = head; current != NULL; current = current->link)
        count++;

    return count;
}
void shuffle(struct Handcard *head)
{
    int swap, i;
    struct Handcard *p, *swapcard;
    Card const *temp;

    if(head == NULL)
        return;

	srand(time(NULL));    
//printf("%d",rand());
    for(p = head; p ->link!= NULL; p = p->link) 
    {
    	
        swap = (rand() % (length(head)));   /* index of swap card */
        temp = p->card;
        //if swap index is 0,it is not practically feasible to shuffle.Or,shiffling cannot be done.
        /*if(swap==0)

        { struct Handcard *t=(struct Handcard*)malloc(sizeof(struct Handcard));

            for(p=head;p->link!=NULL;p=p->link);
                *t=*p;


            for(p=head;p->link->link!=NULL;p=p->link)
            {
                *(p->link)=*p;

            }
            *head=*t;
            free(t);
        }*/
        for(i=0, swapcard=head; i<swap; i++)    /* walk to swapcard */
            swapcard = swapcard->link;
        p->card = swapcard->card;               /* swap    */
        swapcard->card = temp;                  /* addresses  */
    }
}


int draw(struct Handcard **pl, struct Handcard **pile, struct Handcard **discard)
{
    struct Handcard *p, *d, *temp;

    /* No more cards in pile, need to shuffle */
    if(*pile == NULL) {

        /* If discard is also just one card, leave function without drawing
         * or shuffling */
        if((*discard)->link == NULL)
            return 0;

        

        /* Reassign discard pile (minus top card)
         * to draw pile before shuffling */
        *pile = (struct Handcard *) malloc(sizeof(struct Handcard));
        for(d = (*discard)->link, p = *pile; d != NULL; p = p->link) 
        {
            p->card = d->card;
            if(d->link != NULL)
                p->link = (struct Handcard *) malloc(sizeof(struct Handcard));
            else
                p->link = NULL;
            temp = d;
            d = d->link;
            free(temp);
        }
        (*discard)->link = NULL;
        shuffle(*pile);
    }

    /* Go to end of player's hand */
    for(p = *pl; p->link != NULL; p = p->link);
        
    p->link = (struct Handcard *) malloc(sizeof(struct Handcard));
    p = p->link;
    p->card = (*pile)->card;
    p->link = NULL;

    p = *pile;
    if((*pile)->link == NULL) { /* No more cards in the draw pile */
        free(*pile);
        *pile = NULL;
    } else {
        *pile = (*pile)->link;
        free(p);
    }

    return 1;
}

/*void sort(int a[],int size)
{	int t;
	for(int i=0;i<size-1;i++)
	{
		for(int j=0;j<size-1;j++)
		{
				if(a[j]>a[j+1])
				{
					t=a[j];
					a[j]=a[j+1];
					a[j+1]=t;
				}
			}
	}
	printf("SCORES:\n");
for(int i=0;i<size;i++)
{
	printf("Player %d : %d\n",(i+1),a[i]);
}
}*/