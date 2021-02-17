/**
 * @author Schier Jonasz
 */


#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
typedef struct segment segment;

//declaration global variables
int height = 20;
int width = 50;
int appleX, appleY;
int direction = 1;
char nickname[30];
int score;
int numberOfPlayers = 200;

//snake segment
struct segment
{
    segment *nastepny;
    int x;
    int y;
};

//snake body and his position
typedef struct
{
    int dx;
    int dy;
    segment *glowa;
} waz;

//player struct
typedef struct player
{
    char nickname[50];
    int score;
} player;

player players[200];


/**
 * Function creating ranking by insert sort
 */
void createRanking() {
    int lenPlayers = 0;
    int k = 0;
    player temp;

    while (players[k].score != -1)
    {
        lenPlayers++;
        k++;
    }

    for (int i = 0; i < lenPlayers - 1; i++) {
        for (int j = 0; j < (lenPlayers - 1 - i); j++) {
            if (players[j].score < players[j + 1].score) {
                temp = players[j];
                players[j] = players[j + 1];
                players[j + 1] = temp;
            }
        }
    }
}


//--------------------- CREATE WINDOWS -----------------------

/**
 * Main window
 * @param win ncurses window
 * @param height of window
 * @param width of window
 * @param startY start y position of window
 * @param startX start x position of window
 */
void createWindow(WINDOW *win, int height, int width, int startY, int startX)
{
    win = newwin(height, width, startY, startX);
    box(win, 0, 0);
    refresh();
    wrefresh(win);
}

/**
 * Score window
 * @param win ncurses window
 * @param startY start y position of window
 * @param startX start x position of window
 */
void createScoreWindow(WINDOW * win, int startY, int startX) {
    win = newwin(2, 20, startY, startX);
    init_pair(4, COLOR_YELLOW, COLOR_BLUE);

    wattron(win, COLOR_PAIR(4));
    mvwaddstr(win, 1, 1, "SCORE: ");
    wclrtoeol(win);
    
    wprintw(win, "%d", score);
    wattroff(win, COLOR_PAIR(4));

    refresh();
    wrefresh(win);
}

/**
 * Score window
 * @param win ncurses window
 * @param startY start y position of window
 * @param startX start x position of window
 * @param showHighscore
 * @param players players list
 * @param nickname of player
 */
void createHighscoresWindow(WINDOW * win, int startY, int startX, bool *showHighscore, player * players, char * nickname) {
  win = newwin(height + 10, width, startY, startX);
  init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(8, COLOR_YELLOW, COLOR_RED);
  init_pair(9, COLOR_WHITE, COLOR_BLUE);

  wattron(win, COLOR_PAIR(8) | A_BOLD);
  mvwprintw(win, 1, startX / 2 + 2, "HIGHSCORES:");
  wattroff(win, COLOR_PAIR(8) | A_BOLD);

  wattron(win, A_REVERSE | A_BLINK);
  mvwprintw(win, height + 8, startX / 2 + 2, "HIT ENTER TO QUIT");
  wattroff(win, A_REVERSE | A_BLINK);


  createRanking();
  //PRINT PLAYERS
  int i = 0;
  while (players[i].score != -1) {
    if (strcmp(nickname, players[i].nickname) == 0) {
      wattron(win, COLOR_PAIR(9) | A_BOLD);
    }
    mvwprintw(win, i + 3, 2, "%d. %s:\t %d\n", i + 1, players[i].nickname, players[i].score);
    wattroff(win, COLOR_PAIR(9) | A_BOLD);
    i++;
  }


  wattron(win, COLOR_PAIR(7));
  box(win, 42, 42);
  wattroff(win, COLOR_PAIR(7));

  int choose = getch();
  if (choose == 10) {
    *showHighscore = false;
  }
  refresh();
  wrefresh(win);
}

/**
 * Nickname window
 * @param win ncurses window
 * @param startY start y position of window
 * @param startX start x position of window
 * @param nickname of player
 */
void createNickWindow(WINDOW * win, int startY, int startX, char *nickname) {
    
    win = newwin(2, 50, startY, startX + width - 20);
    init_pair(6, COLOR_MAGENTA, COLOR_YELLOW);

    wattron(win, COLOR_PAIR(6));
    mvwaddstr(win, 1, 1, "NICKNAME: ");
    wclrtoeol(win);
    
    wprintw(win, "%s", nickname);
    wattroff(win, COLOR_PAIR(6));

    refresh();
    wrefresh(win);
}

/**
 * Title window
 * @param win ncurses window
 * @param startY start y position of window
 * @param startX start x position of window
 */
void createTitleWindow(WINDOW * win, int startY, int startX) {
    win = newwin(4, 20, startY, startX - 1);
    init_pair(5, COLOR_GREEN, COLOR_RED);

    wattron(win, COLOR_PAIR(5) | A_BOLD);
    mvwaddstr(win, 0, 0, "C N A K E");
    wattroff(win, COLOR_PAIR(5) | A_BOLD);

    refresh();
    wrefresh(win);
}


//--------------------- END CREATE WINDOWS -----------------------

// ------------------- SNAKE STUFF -----------------------

/**
 * Function drawing snake on screen
 * @param snake
 * @param win ncurses window
 */
void drawSnake(waz *snake, WINDOW *win)
{
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_GREEN, COLOR_GREEN);

    segment *currSegment = snake->glowa;

    attron(COLOR_PAIR(1));
    mvwaddch(win, currSegment->y, currSegment->x, ':');
    attroff(COLOR_PAIR(1));

    if (currSegment->nastepny != NULL)
    {
        currSegment = currSegment->nastepny;
        while (currSegment->nastepny != NULL)
        {
            attron(COLOR_PAIR(2));
            mvwaddch(win, currSegment->y, currSegment->x, '#');
            attroff(COLOR_PAIR(2));
            currSegment = currSegment->nastepny;
        }
    }
}

/**
 * Function moves our snake's coordinates
 * @param snake
 */
void runSnake(waz *snake)
{
    segment *currSegment = snake->glowa;
    int dy, yPrev = currSegment->y;
    int dx, xPrev = currSegment->x;
    currSegment->y = currSegment->y + snake->dy;
    currSegment->x = currSegment->x + snake->dx;

    if (currSegment->nastepny != NULL)
    {
        currSegment = currSegment->nastepny;
        while (currSegment->nastepny != NULL)
        {
            dy = currSegment->y;
            dx = currSegment->x;

            currSegment->y = yPrev;
            currSegment->x = xPrev;

            yPrev = dy;
            xPrev = dx;

            currSegment = currSegment->nastepny;
        }
    }
}

/**
 * Function adds a segment to the end of snake
 * @param snake
 */
void addTail(waz *snake)
{
    segment *currSegment = snake->glowa;
    segment *tail = malloc(sizeof(segment));
    //go to end of tail
    while (currSegment->nastepny != NULL)
    {
        currSegment = currSegment->nastepny;
    }

    tail->y = currSegment->y;
    tail->x = currSegment->x;
    tail->nastepny = NULL;

    currSegment->nastepny = tail;
}

// ------------------- END SNAKE STUFF -----------------------

//---------------- MOVE FUNCTIONS ---------------
void moveUp(waz *snake)
{
    snake->dy = -1;
    snake->dx = 0;
    direction = 0;
}

void moveDown(waz *snake)
{
    snake->dy = 1;
    snake->dx = 0;
    direction = 2;
}

void moveRight(waz *snake)
{
    snake->dy = 0;
    snake->dx = 1;
    direction = 1;
}

void moveLeft(waz *snake)
{
    snake->dy = 0;
    snake->dx = -1;
    direction = 3;
}

//---------------- END MOVE FUNCTIONS ---------------

// ----------------- FOR APPLE ----------------- 

/**
 * Function creates random coordinates of apple
 * @param yMax y maximum window
 * @param xMax x maximum window
 * @param snake
 */
void locateApple(int yMax, int xMax, waz *snake)
{
    int randX, randY;
    int spawnAgain = 1;

    while (spawnAgain)
    {
        spawnAgain = 0;
        appleX = (rand() % width) + xMax;
        appleY = (rand() % height) + yMax;

        if (appleX == xMax)
        {
            ++appleX;
        }
        if (appleY == yMax)
        {
            ++appleY;
        }
        if (appleX == (xMax + width) - 1)
        {
            appleX = appleX - 1;
        }
        if (appleY == (yMax + height))
        {
            appleY = appleY - 2;
        }
        if (appleY == (yMax + height - 1))
        {
            appleY = appleY - 1;
        }

        //check if apple spawn inside our snake
        segment *currSegment = snake->glowa;
        while (currSegment->nastepny != NULL)
        {
            if ((currSegment -> x == appleX) && (currSegment -> y == appleY))
            {
                spawnAgain = 1;
                break;
            }
            currSegment = currSegment -> nastepny;
        }
    }
}

/**
 * Function drawing an apple on the screen
 * @param win ncurses window
 * @param yMax y maximum window
 * @param xMax x maximum window
 */
void drawApple(WINDOW *win, int yMax, int xMax)
{
    init_pair(3, COLOR_RED, COLOR_BLACK);

    attron(COLOR_PAIR(3));
    mvwaddstr(win, appleY, appleX, "o");
    attroff(COLOR_PAIR(3));
}

// ------------------------END APPLE------------------------------

// ------------------- CHECKS -------------------

/**
 * Function checks if snake has hit the frames box
 * @param snake
 * @param yMax y maximum window
 * @param xMax x maximum window
 * @return true or false
 */
bool checkBoxCollision(waz * snake, int yMax, int xMax) {
    segment * head = snake -> glowa;
    if ((head -> y == (yMax)) || (head -> y == (yMax + height - 1))) {
        return true;
    }
    if ((head -> x == (xMax)) || (head -> x == (xMax + width - 1))) {
        return true;
    }
    return false;
}

/**
 * Function checks if snake has hit itself
 * @param snake
 * @return true or false
 */
bool checkSelfSnakeCollision(waz * snake) {
    segment * head = snake -> glowa;
    segment * body = snake -> glowa -> nastepny;

    while (body -> nastepny != NULL) {
        if ((head -> x == body -> x) && (head -> y == body -> y)) {
            return true;
        }
        body = body -> nastepny;
    }
    
    return false;
}

/**
 * Function checks if snake has eaten apple
 * @param snake
 * @return true or false
 */
bool checkIfEatenApple(waz * snake) {
    if ((snake -> glowa -> x == appleX) && (snake -> glowa -> y == appleY)) {
        return true;
    }
    return false;
}

// ------------------- END CHECKS -------------------

// ---------------------- FILES STUFF ----------------------

/**
 * Function split nickname and score from file
 * @param source text from file
 * @param nickname player
 * @param score
 * @param delim delimeter
 */
void mySplit(const char * source, char *nickname, char *score, char delim) {
    char c;
    while ((c = *source) != delim) {
        // printf("%c", *source);
        *nickname = c;
        nickname++;
        source++;
    }
    *nickname = '\0';
    source++;
    // printf("\n");
    while ((c = *source) != '\0') {
        // printf("%c", *source);
        *score = c;
        score++;
        source++;
    }
    *score = '\0';
    source++;
}

/**
 * Function save nickname and score to the file
 * @param fileName
 * @param nickname
 * @param score
 */
void saveScore(const char *fileName, char nickname[], int score) {
  FILE *fp = fopen(fileName, "a");
  if (!fp) {
    printw("File error...");
  }

  fprintf(fp, "%s;%d\n", nickname, score);
  fclose(fp);
}

void fillPlayers(const char *fileName) {
    FILE *fp = fopen(fileName, "r");

    if(!fp) {
        printf("Brak pliku z danymi!");
        exit(1);
    }
    int maxN = 200;
    char text[maxN];
    char nickname[50];
    char score[30];
    int i = 0;
    
    while(fgets(text, 200, fp)) {
        // printf("%s\n", text);
        mySplit(text, nickname, score, ';');
        strcpy(players[i].nickname, nickname);
        players[i].score = atoi(score);
        i++;
    }

    fclose(fp);
}

// -------------------- END FILES STUFF --------------------

// ------ START GAME FUNCTION ----------

/**
 * Function draws all basic things like windows, snake, apple
 * @param source text from file
 * @param nickname player
 * @param score
 * @param delim delimeter
 */
void playGame(WINDOW *win, WINDOW *scoreWin, WINDOW *titleWin, WINDOW * nickWin, waz *snake, int yMax, int xMax)
{
    createWindow(win, height, width, yMax, xMax);
    createScoreWindow(scoreWin, yMax - 2, xMax);
    createTitleWindow(titleWin, (yMax / 2), (xMax + (xMax / 2) + 2));
    createNickWindow(nickWin, yMax - 2, xMax, nickname);
    drawSnake(snake, win);
    drawApple(win, yMax, xMax);
    runSnake(snake);
}

int main(void)
{
    WINDOW *mainwin;
    WINDOW *scoreWin;
    WINDOW *titleWin;
    WINDOW *nickWin;
    WINDOW *highscoresWin;

    // Initializacja ncurses
    if ((mainwin = initscr()) == NULL || (scoreWin = initscr()) == NULL || (titleWin = initscr()) == NULL || (nickWin = initscr()) == NULL || (highscoresWin = initscr()) == NULL)
    {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(EXIT_FAILURE);
    }

    char *fileName = "highscores_cnake.txt";

    // -------------------- START --------------------    

    int j = 0;
    while (j < numberOfPlayers) {
      players[j].score = -1;
      j++;
    }

    bool startGame = true;
    do {

      score = 0;
      int yMax, xMax, yNick, xNick;
      getmaxyx(stdscr, yMax, xMax);
      getmaxyx(stdscr, yNick, xNick);
      yNick = yNick / 3;
      xNick = xNick / 3;
      yMax = yMax / 4;
      xMax = (xMax / 4) - 1;

      //nickname is required
      while (strlen(nickname) == 0) {
        attron(A_REVERSE);
        mvprintw(yNick, xNick, "Enter your nickname:");
        attroff(A_REVERSE);
        getstr(nickname);
      }

      
      keypad(stdscr, true); // key arrow ON
      start_color();         // ncurses color ON
      
      noecho();              // do not show entered data
      cbreak();
      curs_set(0);           //cursor OFF
      timeout(500); // wait 500ms for click

      //CREATE SNAKE HEAD
      segment *head = malloc(sizeof(segment));
      waz *snake = malloc(sizeof(waz));

      snake->dx = 1;
      snake->dy = 0;

      head->nastepny = NULL;
      head->x = xMax + 6;
      head->y = yMax + 6;
    
      snake->glowa = head;
      addTail(snake);
      locateApple(yMax, xMax, snake);

      
      bool play = false;
      do
      {
          int c = getch();
          switch (c)
          {
          case KEY_UP:
              if (direction != 2)
              {
                  moveUp(snake);
              }
              break;
          case KEY_DOWN:
              if (direction != 0)
              {
                  moveDown(snake);
              }
              break;
          case KEY_RIGHT:
              if (direction != 3)
              {
                  moveRight(snake);
              }
              break;
          case KEY_LEFT:
              if (direction != 1)
              {
                  moveLeft(snake);
              }
              break;
          case 'q':
              play = true;
              break;
          default:
              break;
          }

          clear();
          if (checkBoxCollision(snake, yMax, xMax)) {
              play = true;
          }
          if (checkIfEatenApple(snake)) {
              addTail(snake);
              score++;
              locateApple(yMax, xMax, snake);
          }
          if (checkSelfSnakeCollision(snake)) {
              play = true;
          }
          playGame(mainwin, scoreWin, titleWin, nickWin, snake, yMax, xMax);
          refresh();

      } while (!play);
      
      saveScore(fileName, nickname, score);

      bool showHigscore = true;
      clear();
      fillPlayers(fileName);
      while(showHigscore) {
        createHighscoresWindow(highscoresWin, yMax, xMax, &showHigscore, players, nickname);
      }
      
      startGame = false;
    } while (startGame);

    

    // clearing
    nocbreak();
    echo();
    refresh();
    delwin(mainwin);
    delwin(titleWin);
    delwin(scoreWin);
    delwin(highscoresWin);
    delwin(nickWin);
    endwin();
    return EXIT_SUCCESS;
}
