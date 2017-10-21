#include "uno_header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 initscr();
    start_color();
    cbreak();
    noecho();
int main(int argc, char *argv[])
{
    int cursy, cursx;
    int maxy, maxx;
    int i, j;
    int prev;       /* used in +4 Wild challenges */
    int lastcol;    /* used in +4 Wild challenges */
    int plrs;       /* total number of players */
    int dir;        /* direction of gameplay, for use with reverse cards.   *
                 * 1 if "clockwise", -1 if "counterclockwise"           */
    int color;      /* Current color in play */
    int key;        /* input character
                 * Needs to be int, to make room for arrow keys */
    char turn;      /* becomes 0 when player turn ends */
    int drawx;      /* If a draw X card is played, how screwed next player is */
    char gtype;     /* Quick or full game */
    int won;        /* Once player wins full game, will be set to their index */
    int round;      /* Round (for full games) */
    int multdraw;   /* player draws until they have a playable card */
    int noskip;     /* Draw Two and Draw Four Wild don't skip */
    int winscore;   /* Score needed to win the game */

    char *cp = NULL;    /* char pointer (for player name dialog) */

     Handcard *pile = NULL;       /* head of draw pile (linked list) */
     Handcard *discard = NULL;    /* head of discard pile (linked list) */

     Handcard *plcard = NULL; /* selected card in player's hand */
     Handcard *p = NULL;      /* the generic Handcard pointer */

    struct Cardname cardname;

    /* Defaults for parameters */
    multdraw = 0;
    noskip = 0;
    winscore = 500;
    /* Read in command line parameters */
    while(--argc > 0 && (*++argv)[0] == '-') {
            switch(*++argv[0]) {
            case 'm':
                multdraw = 1;
                break;
            case 'd':
                noskip = 1;
                break;
            case 's':
                if((*++argv[0]) != '=') {
                    printf("Error: Format should be -s=score\n");
                    return 1;
                }
                winscore = 0;
                while((*++argv[0]))
                    if(!isdigit(*argv[0])) {
                        printf("Error: Numerical parameter needed\n");
                        return 1;
                    } else
                        winscore = 10*winscore + *argv[0] - '0';
                break;
            case 'h': case 'H':
                printf("UNO usage\n"
                    "-d\t\tDraw Two and Draw Four Wilds don't skip\n"
                    "-h, -H\t\tThis help reference\n"
                    "-m\t\tPlayers draw until they pull a playable card\n"
                    "-s=score\tSet winning score for full games (Default = 500)"
                    "\n\n"
                    "For UNO rules, visit www.unorules.com\n");
                return 0;
            default:
                printf("Illegal parameter %c\n", *argv[0]);
                return 1;
            }
    }

    /* NCURSES MODE INITIATES HERE.  TRY TO KEEP DECLARATIONS
     * ABOVE THIS LINE PLS */
   
    keypad(stdscr, TRUE);   /* for arrow keys and stuff */

    getmaxyx(stdscr, maxy, maxx);

    /* quit if terminal screen not big enough */
    if(maxy < 24 || maxx < 80) {
        endwin();
        printf("ERROR: Screen too small! Aborting...\n\n");
        return 1;
    }

    /* Color pair initializations */
    init_pair(RED, COLOR_BLACK, COLOR_RED);
    init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(GREEN, COLOR_BLACK, COLOR_GREEN);
    init_pair(YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);     /* for wilds */
    init_pair(DEF, COLOR_WHITE, COLOR_BLACK);       /* for text displays */
    init_pair(UNO, COLOR_RED, COLOR_BLACK);     /* for UNO tag */

    /* MAYBE SOME KIND OF SPLASH SCREEN? */

    /* Quick or full game? */
    mvprintw(0,0, "<Q>uick or <F>ull game?");
    clrtoeol();
    curs_set(1);
    while((gtype=toupper(getch())) != 'Q' && gtype != 'F')
        ;

    /* get number of players */
    echo();
    do {
        mvprintw(0,0, "How many players (2-10)? ");
        clrtoeol();
        scanw("%d", &plrs);
    } while(plrs < 2 || plrs > 10);
    noecho();

    /* Player array declarations */
    struct Handcard *pl[plrs];      /* heads of players' hands (linked lists) */
    char *plname[plrs];             /* players' names */
    int plscore[plrs];              /* Players' scores */
    int uno[plrs];                  /* tracks who has "UNO"'d */


    /* get players' names, set scores to 0 */
    for(i=0; i<plrs; i++) {
        plscore[i] = 0;
        plname[i] = (char *) calloc(13, sizeof(char));
        cp = plname[i];
        mvprintw(i, 0, "Player %d, enter your name (12 chars max): ", i+1);
        getyx(stdscr, cursy, cursx);
        while((key=getch()) != '\n') {
            if(strlen(plname[i]) < 12 && (isalpha(key) || isdigit(key)
            || ispunct(key) || key == ' ')) {
                *cp++ = key;
                mvaddch(cursy, cursx++, key);
            }
            if((key == KEY_BACKSPACE || key == KEY_DC || key == 127)
            && strlen(plname[i])) {
                *--cp = '\0';
                mvaddch(cursy, --cursx, ' ');
            }
        }
        key = *cp = '\0';
        attron(A_BOLD);
        mvprintw(i,0, "%s", plname[i]); clrtoeol();
        attroff(A_BOLD);
        refresh();
    }
    mvprintw(i+1,0, "Let's go!");
    getch();
    curs_set(0);

    round = 1;
    won = -1;

    /* GAMEPLAY LOOP BEGINS HERE
     * This loop will run only once in a quick game.  In a full game,
     * it loops until any player reaches 500 points */
    while(won < 0) {

        curs_set(0);

        /* Allocate memory for players' hands, set uno's to 0 */
        for(i=0; i<plrs; i++) {
            pl[i] = NULL;
            uno[i] = 0;
        }

        /* initialize pile to sorted deck */
        pile = (struct Handcard *) malloc(sizeof(struct Handcard));
        p = pile;
        for(i=0; i<DECKSIZE; i++) {
            p->card = &deck[i];
            if(i < DECKSIZE-1)
                p->link = (struct Handcard *) malloc(sizeof(struct Handcard));
            else
                p->link = NULL;
            p = p->link;
        }

        shuffle(pile);  /* Shuffle the deck */

        /* deal the initial hand to each player */
        for(j=0; j < INITHAND; j++)
            for(i=0; i<plrs; i++) {
                if(pl[i] == NULL) {
                    pl[i] = (struct Handcard *) malloc(sizeof(struct Handcard));
                    pl[i]->card = pile->card;
                    pl[i]->link = NULL;
                } else {
                    for(plcard = pl[i]; plcard->link != NULL; plcard = plcard->link)
                        
                    plcard->link = (struct Handcard *) malloc(sizeof(struct Handcard));
                    plcard = plcard->link;
                    plcard->card = pile->card;
                    plcard->link = NULL;
                }
                p = pile;           /* Remove the card from the head of the */
                pile = pile->link;  /* pile                                 */
                free(p);            /*                                      */
            }

        /* Turn over top card */
        do {
            if(discard != NULL)
                if(discard->card->col == WHITE) {
                    p = (struct Handcard *) malloc(sizeof(struct Handcard));
                    p->card = discard->card;
                    p->link = pile;
                    pile = p;
                    free(discard);
                    discard = NULL;
                    shuffle(pile);
                }
            discard = (struct Handcard *) malloc(sizeof(struct Handcard));
            discard->card = pile->card;
            discard->link = NULL;
            p = pile;
            pile = pile->link;
            free(p);
        } while(discard->card->col == WHITE);

        i = -1; /* dealer */
        dir = 1;
        drawx = 0;
        lastcol = color = discard->card->col;

        /* If top card is Draw Two or Reverse, respectively */
        if(discard->card->num == DRAWTWO && !drawx)
            drawx = 2;
        else if(discard->card->num == REVERSE)
            dir = -dir;

        if(gtype == 'F') {
            clear();
            mvprintw(maxy/2, maxx/2, "ROUND %d", round);
            getch();
        }

        /* PLAYER TURN STARTS HERE */
        do {
            turn = 1; /* needed for stupid input loop below */

            clear();
            curs_set(0);

            /* Advance to next player, includes handling SKIPs */
            i += dir + dir*(discard->card->num == SKIP && key != 'D')
              + dir*(discard->card->num == REVERSE && plrs == 2 && key != 'D');
            if(i >= plrs)   /* ensure i is always a valid index */
                i -= plrs;
            else if(i < 0)
                i += plrs;

            mvprintw(maxy/2, maxx/2-strlen(plname[i]), 
                         "%s, get ready!", plname[i]);
            getch();
            move(maxy/2,0); clrtoeol();

            cursy = CARDTOP;        /* where card selection arrow begins */
            cursx = CARDLEFT+2;     /* each turn                         */

            plcard = pl[i]; /* Set player card pointer to head of hand */

            /* Show player's name in ll corner */
            attron(COLOR_PAIR(WHITE) | A_BOLD);
            move(maxy-1, 0);
            if(gtype == 'F')
                printw("Round %d - ", round);
            printw("%s - <D>raw, <P>lay, <U>no, <Q>uit. "
                   "Select with up/down arrows", plname[i]);
            clrtoeol();

            /* Show discard pile */
            attron(COLOR_PAIR(DEF));
            attroff(A_BOLD);
            mvprintw(maxy/2, maxx*3/8-10, "DISCARD ");
            attron(COLOR_PAIR(color) | A_BOLD);
            if(discard->card->num >= ZERO && discard->card->num <= NINE)
                attroff(A_BOLD);
            addch(discard->card->num);

            /* Display all player stats */
            attron(COLOR_PAIR(DEF) | A_BOLD);
            mvprintw(6,maxx/2, "PLAYER\tCARDS\tSCORE");
            for(j=0; j<plrs; j++) {
                attron(COLOR_PAIR(DEF));
                if(i == j) {
                    attron(A_BOLD);
                    if(plrs > 2) { /* indicate direction of gameplay */
                        if(dir > 0)
                            mvaddch(j+8,maxx/2 - 2, 'v');
                        else if(dir < 0)
                            mvaddch(j+8,maxx/2 - 2, '^');
                    }
                } else
                    attroff(A_BOLD);
                mvprintw(j+8,maxx/2, "%-12s %00d\t%000d",
                         plname[j], length(pl[j]), plscore[j]);
                if(uno[j]) {
                    attron(COLOR_PAIR(UNO) | A_BOLD);
                    printw("\tU");       /* mark if player has UNO'd */
                } else
                    printw("\t ");
            }

            /* Player forced to draw cards */
            if(drawx) {
                showhand(pl[i]);

                /* For challenging +4 wilds */
                if(discard->card->num == PLUSWILD) {
                    curs_set(1);
                    mvprintw(0,0, "%s has played a Wild Draw Four! "
                                  "Do you challenge? (Y/N)", plname[prev]);
                    clrtoeol();
                    while((key=tolower(getch())) != 'y' && key != 'n')
                        ;

                    /* Player has challenged */
                    if(key == 'y') {
                        for(p = pl[prev]; p != NULL; p = p->link)
                            if(p->card->num == discard->link->card->num
                            || p->card->col == discard->link->card->col
                            || (discard->link->card->col == WHITE 
                                && p->card->col == lastcol)
                            || p->card->num == WILD) {
                                mvprintw(0,0,
                                        "%s played a Wild Draw Four illegally! "
                                         "They will be dealt four cards.",
                                         plname[prev]);
                                clrtoeol();
                                getch();
                                for(j=0; j<4; j++) {
                                    drawx = 0;
                                    if(!draw(&pl[prev], &pile, &discard)) {
                                        mvprintw(0,0, "Oops! Not enough cards"
                                                      " in the deck.");
                                        clrtoeol();
                                        curs_set(1);
                                        getch();
                                        curs_set(0);
                                        break;
                                    }
                                }
                                mvprintw(prev+8,maxx/2, "%-12s %00d\t%000d",
                                         plname[prev], length(pl[prev]),
                                         plscore[prev]);
                                uno[prev] = 0;
                                printw("\t ");
                                break;
                            }
                        if(p == NULL) {
                            mvprintw(0,0, "Your challenge failed!");
                            clrtoeol();
                            getch();
                            drawx = 6;
                        }
                    }
                }

                if(drawx) {
                    curs_set(1);
                    mvprintw(0,0, "You must draw %d cards.", drawx);
                    clrtoeol();
                    getch();
                    curs_set(0);
                    do {
                        if(pile == NULL && length(discard) == 1) {
                            mvprintw(0,0, "It's your lucky day...not enough "
                                          "cards in the pile!");
                            curs_set(1);
                            clrtoeol();
                            getch();
                            curs_set(0);
                            drawx = 0;
                            break;
                        }
                        draw(&pl[i], &pile, &discard);
                    } while(--drawx);

                    uno[i] = 0;
                    attron(A_BOLD);
                    mvprintw(i+8,maxx/2, "%-12s %00d\t%000d",
                             plname[i], length(pl[i]), plscore[i]);
                    printw("\t ");
                    attroff(A_BOLD);

                    /* "Draw X" cards skip unless in noskip mode */
                    if(!noskip && (discard->card->num == DRAWTWO
                                || discard->card->num == PLUSWILD)) {
                        showhand(pl[i]);
                        curs_set(1);
                        mvprintw(0,0, "Proceed to next player's turn.");
                        clrtoeol();
                        getch();
                        curs_set(0);
                        continue;
                    }
                }
            }

            /* input loop. It exits once I set "turn" to 0, which
             * only happens on a break in the middle of the switch
             * block below. Otherwise I use continue to skip the
             * "turn = 0" line. A little hacky, but then it used
             * to be a goto label here... */
            do {
                if(length(pl[i]) > 1)
                    uno[i] = 0;

                curs_set(0);
                move(0,0); clrtoeol();
                showhand(pl[i]);
                cardname = getcardname(cardname, plcard->card);

                /* Show discard pile again, if we just shuffled */
                mvprintw(maxy/2, maxx*3/8-10, "DISCARD ");
                attron(COLOR_PAIR(color) | A_BOLD);
                if(discard->card->num >= ZERO && discard->card->num <= NINE)
                    attroff(A_BOLD);
                addch(discard->card->num);

                /* Draw cursor next to selected card */
                attron(COLOR_PAIR(DEF) | A_BOLD);
                mvaddch(cursy,cursx, '<');
                attroff(A_BOLD);

                /* Show name of selected card */
                mvprintw(0,0, "Selected: %s", cardname.col);
                if(plcard->card->num >= ZERO && plcard->card->num <= NINE)
                    addch(cardname.num.c);
                else
                    printw("%s", cardname.num.s);
                clrtoeol();

            /* Wait for player command */
                switch(key=toupper(getch()))
                {
                case KEY_UP:
                    if(plcard != pl[i]) {
                        p = pl[i];
                        mvaddch(cursy,cursx, ' ');
                        cursy -= 2;
                        if(cursy < CARDTOP) {
                            cursy = CARDTOP + CARDLINES*2;
                            cursx -= 4;
                        }
                        while(p->link != plcard)
                            p = p->link;
                        plcard = p;
                    } else {
                        mvaddch(cursy,cursx, ' ');
                        for(p = pl[i]; p->link != NULL; p = p->link) {
                            cursy += 2;
                            if(cursy > CARDTOP + CARDLINES*2) {
                                cursy = CARDTOP;
                                cursx += 4;
                            }
                        }
                        plcard = p;
                    }
                    continue;
                case KEY_DOWN:
                    if(plcard->link != NULL) {
                        plcard = plcard->link;
                        mvaddch(cursy,cursx, ' ');
                        cursy += 2;
                        if(cursy > CARDTOP + CARDLINES*2) {
                            cursy = CARDTOP;
                            cursx += 4;
                        }
                    } else {
                        plcard = pl[i];
                        mvaddch(cursy,cursx, ' ');
                        cursy = CARDTOP;
                        cursx = CARDLEFT+2;
                    }
                    continue;
                case 'D':                           /* Draw a card */
                    for(p = pl[i]; p != NULL; p = p->link)
                        if(p->card->num == discard->card->num 
                        || p->card->col == color
                        || p->card->col == WHITE) {
                            mvprintw(0,0, "You have playable cards! "
                                          "Draw anyway?");
                            clrtoeol();
                            curs_set(1);
                            if(tolower(getch()) != 'y')
                                break;      /* exit loop, card won't draw */
                            else {
                                p = NULL;   /* exit loop anyway, card will draw */
                                break;
                            }
                        }

                    /* call draw, and update # of player's cards on display */
                    if(p == NULL) {
                        if(!draw(&pl[i], &pile, &discard)) {
                            mvprintw(0,0, "No more cards to draw. Proceed "
                                          "to next player.");
                            clrtoeol();
                            curs_set(1);
                            getch();
                            curs_set(0);
                            break;
                        };
                        uno[i] = 0;
                        attron(A_BOLD);
                        mvprintw(i+8,maxx/2, "%-12s %00d\t%000d", 
                                 plname[i], length(pl[i]), plscore[i]);
                        printw("\t ");
                        attroff(A_BOLD);

                        /* if multdraw, then player can keep drawing indefinitely.
                         * if not, player only gets one chance to draw per turn */
                        if(multdraw)
                            continue;
                        else {
                            showhand(pl[i]);
                            for(p = pl[i]; p->link != NULL; p = p->link)
                                ;
                            if(p->card->num == discard->card->num
                            || p->card->col == color
                            || p->card->col == WHITE) {
                                mvprintw(0,0, "You drew a playable card! Play it? "
                                  "(Y/N%s", (length(pl[i]) == 2) ? "/U)" : ")");
                                clrtoeol();
                                curs_set(1);
                                while((key=tolower(getch())) != 'y' && key != 'n'
                                      && !(key == 'u' && length(pl[i]) == 2))
                                    ;
                                if(key == 'n') {
                                    break;      /* to next player */
                                } else {
                                    if(key == 'u')
                                        uno[i] = 1;
                                    plcard = p; /* ...and then go to case 'P' */
                                }
                            } else {
                                mvprintw(0,0, "Not a playable card. "
                                              "Proceed to next player.");
                                curs_set(1);
                                clrtoeol();
                                getch();
                                break;
                            }
                        }
                    } else
                        continue;
                case 'P': case 'U':                 /* Play a card */
                    if(key == 'U') {
                        for(j=0; j<plrs; j++)
                            if(length(pl[j]) == 1 && !uno[j] && j != i) {
                                attron(COLOR_PAIR(DEF));
                                attroff(A_BOLD);
                                curs_set(1);
                                mvprintw(0,0, "%s didn't say UNO!"
                                       " They must draw two cards.", plname[j]);
                                clrtoeol();
                                getch();
                                curs_set(0);
                                drawx = 2;
                                do
                                    if(!draw(&pl[j], &pile, &discard)) {
                                        mvprintw(0,0, "Oops! Not enough cards"
                                                      " in the deck.");
                                        clrtoeol();
                                        curs_set(1);
                                        getch();
                                        curs_set(0);
                                        drawx = 0;
                                        break;
                                    }
                                while(--drawx);
                                mvprintw(j+8,maxx/2, "%-12s %00d\t%000d",
                                        plname[j], length(pl[j]), plscore[j]);
                                break;
                            }
                        if(j == plrs && length(pl[i]) > 2) {
                            curs_set(1);
                            mvprintw(0,0, "Nobody forgot to say UNO!");
                            clrtoeol();
                            getch();
                        }
                        curs_set(0);
                        if(length(pl[i]) > 2)
                            continue;
                        else if(length(pl[i]) == 1) {
                            uno[i] = 1;
                            mvprintw(i+8,maxx/2, "%-12s %00d\t%000d",
                                plname[i], length(pl[i]), plscore[i]);
                            attron(COLOR_PAIR(UNO) | A_BOLD);
                            printw("\tU");
                            attron(COLOR_PAIR(DEF));
                            attroff(A_BOLD);
                            continue;
                        }
                    }

                /* Player chose a playable card */
                    if(plcard->card->num == discard->card->num 
                    || plcard->card->col == color
                    || plcard->card->col == WHITE) {
                        /* place on discard pile */
                        p = (struct Handcard *) malloc(sizeof(struct Handcard));
                        p->card = plcard->card;
                        p->link = discard;
                        discard = p;

                        /* ...and remove from player's hand */
                        if(plcard->link != NULL) {
                            plcard->card = plcard->link->card;
                            p = plcard->link->link;
                            free(plcard->link);
                            plcard->link = p;
                        } else {    /* card at end of hand */
                            if(pl[i] == plcard) {   /* Last card! */
                                free(pl[i]);
                                pl[i] = NULL;
                                break;
                            }
                            for(p=pl[i]; p->link != plcard; p = p->link)
                                ;
                            p->link = NULL;
                            free(plcard);
                        }

                        /* "UNO!" */
                        if(length(pl[i]) == 1 && key == 'U') {
                            uno[i] = 1;
                            beep();
                        }

                    /* Choose color if wild; easy to set if not */
                        if(discard->card->col != WHITE)
                            color = discard->card->col;
                        else {
                            lastcol = color;
                            color = 0;
                            curs_set(1);
                            while(color == 0) {
                                mvprintw(0,0, "Call a color (<R>ed, <B>lue, "
                                              "<G>reen, <Y>ellow): ");
                                clrtoeol();
                                switch(key = tolower(getch())) {
                                case 'r':
                                    color = RED;
                                    break;
                                case 'b':
                                    color = BLUE;
                                    break;
                                case 'g':
                                    color = GREEN;
                                    break;
                                case 'y':
                                    color = YELLOW;
                                    break;
                                default:
                                    mvprintw(0,0, "Not a valid color!");
                                    clrtoeol();
                                    getch();
                                    break;
                                }
                            }
                        }
                    /* More card actions (skip behavior handled 
                     * at start of turn) */
                        switch(discard->card->num) {
                        case REVERSE:
                            dir = -dir;
                            break;
                        case DRAWTWO:
                            drawx = 2;
                            break;
                        case PLUSWILD:
                            drawx = 4;
                            prev = i;
                            break;
                        }

                    } else {
                        mvprintw(0,0, "You can't play that card!");
                        curs_set(1);
                        clrtoeol();
                        getch();
                        curs_set(0);
                        continue;
                    }
                    break;
                case 'Q':                               /* Quit game */
                    mvprintw(0,0, "Really quit?");
                    curs_set(1);
                    clrtoeol();
                    if((key=tolower(getch())) == 'y') {
                        endwin();
                        clean(discard);
                        clean(pile);
                        for(i=0; i<plrs; i++) {
                            clean(pl[i]);
                            free(plname[i]);
                        }
                        printf("Buhbye now :3\n");
                        return 0;
                    } else
                        continue;
                default:
                    continue;
                }
                turn = 0; /* only reached on break, not continue */

            } while(turn);

        } while(length(pl[i]) > 0);     /* round ends when a 
                                         * player has no cards */

        /* Won round (or game, in quick mode) */
        clear();

        /* tallies score (for full game) */
        if(gtype == 'F') {
            attron(COLOR_PAIR(UNO) | A_BOLD);
            mvprintw(maxy/2, maxx/2, "%s won round %d", plname[i], round);
            getch();
            for(j=0; j<plrs; j++)
                if(i != j)
                    for(p = pl[j]; p != NULL; p = p->link)
                        if(p->card->num >= ZERO && p->card->num <= NINE)
                            plscore[i] += p->card->num - ZERO;
                        else if(p->card-> num == WILD 
                                || p->card->num == PLUSWILD)
                            plscore[i] += 50;
                        else
                            plscore[i] += 20;
            if(plscore[i] >= winscore)
                won = i;
        } else
            won = i;

        /* display end-of-round scores in full game */
        if(gtype == 'F') {
            clear();
            attron(COLOR_PAIR(DEF) | A_BOLD);
            mvprintw(0,0, "ROUND %d", round);
            mvprintw(1,0, "PLAYER\t\tSCORE");
            attroff(A_BOLD);
            for(j=0; j<plrs; j++)
                mvprintw(j+3,0, "%-12s\t%000d", plname[j], plscore[j]);
            attron(A_BOLD);
            printw("\n\nWin at %d", winscore);
            getch();
        }

        /* free memory allocated for this round */
        clean(pile);
        clean(discard);
        for(j=0; j<plrs; j++)
            clean(pl[j]);

        round++;
    }

    /* WINNING SCREEN */

    clear();
    attron(COLOR_PAIR(YELLOW) | A_BOLD);
    mvprintw(maxy/2, maxx/2-strlen(plname[won]), "%s is the winner!", plname[won]);
    getch();

    endwin();

    printf("Congrats, %s! gg and see y'all next time...\n\n", plname[won]);

    for(i=0; i<plrs; i++)
        free(plname[i]);

    return 0;

}

/****           -FUNCTIONS-         ****/

/* clean *
 * Frees up a linked list from tail to head end, then sets head to NULL */
void clean(struct Handcard *head)
{
    if(head == NULL) /* don't try cleaning a null list */
        return;

    struct Handcard *p;
    while(head->link != NULL) {
        for(p=head; p->link->link != NULL; p = p->link)
            ;
        free(p->link);
        p->link = NULL;
    }
    free(head);
    head = NULL;
}

/* shuffle *
 * Shuffle whatever pile has been passed to it, starting at head */
void shuffle(struct Handcard *head)
{
    int swap, i;
    struct Handcard *p, *swapcard;
    Card const *temp;

    if(head == NULL)
        return;

    srand(time(NULL));  /* Seed the random number generator each
                         * time we shuffle */

    for(p = head; p != NULL; p = p->link) {
        swap = (rand() % (length(head)));   /* index of swap card */
        temp = p->card;
        for(i=0, swapcard=head; i<swap; i++)    /* walk to swapcard */
            swapcard = swapcard->link;
        p->card = swapcard->card;               /* exchange     */
        swapcard->card = temp;                  /* addresses ;3 */
    }
}

/* showhand *
 * Display hand one card at a time, starting from head */
void showhand(struct Handcard *head)
{
    int y, x;

    if(head == NULL)
        return;

    y = CARDTOP;
    x = CARDLEFT;
    struct Handcard *p;
    for(p = head; p != NULL; p = p->link) {
        attron(COLOR_PAIR(p->card->col) | A_BOLD);
        if(p->card->num >= ZERO && p->card->num <= NINE)
            attroff(A_BOLD);
        mvaddch(y, x, p->card->num);
        y += 2;
        if(y > CARDTOP + CARDLINES*2) {
            x += 4;
            y = CARDTOP;
        }
    }
    attron(COLOR_PAIR(DEF)); attroff(A_BOLD);
}

/* length *
 * Get the length of the hand passed to it */
int length(struct Handcard *head)
{
    struct Handcard *current;
    int count = 0;

    if(head == NULL)
        return 0;
    for(current = head; current != NULL; current = current->link)
        count++;

    return count;
}

/* getcardname *
 * Gets the name of the card as strings */
struct Cardname getcardname(struct Cardname cardname, Card const *card)
{
    /* set color string */
    switch(card->col) {
    case RED:
        cardname.col = "Red ";
        break;
    case BLUE:
        cardname.col = "Blue ";
        break;
    case GREEN:
        cardname.col = "Green ";
        break;
    case YELLOW:
        cardname.col = "Yellow ";
        break;
    default:                /* for wilds */
        cardname.col = "";
        break;
    }

    /* ...and now name string */
    switch(card->num) {
    case REVERSE:
        cardname.num.s = "Reverse";
        break;
    case SKIP:
        cardname.num.s = "Skip";
        break;
    case DRAWTWO:
        cardname.num.s = "Draw Two";
        break;
    case WILD:
        cardname.num.s = "Wild Card";
        break;
    case PLUSWILD:
        cardname.num.s = "Draw Four Wild";
        break;
    default:
        cardname.num.c = card->num;
        break;
    }

    return cardname;
}

/* draw *
 * A more complicated function. Draws a card from the draw pile into
 * the player's hand, and also handles reshuffling deck if draw pile is empty.
 * Returns 1 if a card is successfully drawn, 0 if not */
int draw(struct Handcard **pl, struct Handcard **pile, struct Handcard **discard)
{
    struct Handcard *p, *d, *temp;

    /* No more cards in pile, need to shuffle */
    if(*pile == NULL) {

        /* If discard is also just one card, leave function without drawing
         * or shuffling */
        if((*discard)->link == NULL)
            return 0;

        curs_set(1);
        attron(COLOR_PAIR(DEF)); attroff(A_BOLD);
        mvprintw(0,0, "No more cards to draw. "
                      "Shuffling the discard pile first...");
        clrtoeol();
        getch();
        curs_set(0);

        /* Reassign discard pile (minus top card)
         * to draw pile before shuffling */
        *pile = (struct Handcard *) malloc(sizeof(struct Handcard));
        for(d = (*discard)->link, p = *pile; d != NULL; p = p->link) {
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
    for(p = *pl; p->link != NULL; p = p->link)
        ;
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
