#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

#define defaultCol 20
#define defaultRow 18

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
   
    game_state_t* rv_game = malloc(sizeof(game_state_t));
    if (rv_game == NULL){
        return 0;
    }

    rv_game->board = malloc(sizeof(char*)*18);
        if (rv_game->board == NULL){
        return 0;
    }


    for (int row = 0; row < defaultRow; row++) {

        rv_game->board[row] = malloc(sizeof(char)*20 + 1);
        if (rv_game->board[row] == NULL) {
            return 0;
        }

        for (int col = 0; col <= defaultCol; col++) {

            if (col == defaultCol) {
                rv_game->board[row][col] = '\0';
            }
            else if (row == 0 || row == defaultRow - 1) {
                rv_game->board[row][col] = '#';
            }

            else if (col == 0 || col == defaultCol - 1) {
                rv_game->board[row][col] = '#';
            }
            else {
                rv_game->board[row][col] = ' ';
            }  
        }
    }

    rv_game->num_rows = defaultRow;
    rv_game->num_snakes = 1;

    rv_game->snakes = malloc(sizeof(snake_t));
    if (rv_game->snakes == NULL){
        return 0;
    }


    snake_t defSnake;
    
    defSnake.tail_row = 2;
    defSnake.tail_col = 2;
    defSnake.head_row = 2;
    defSnake.head_col = 4;
    defSnake.live = true;
    rv_game->snakes[0] = defSnake;

    rv_game->board[2][2] = 'd';
    rv_game->board[2][3] = '>';
    rv_game->board[2][4] = 'D';
    rv_game->board[2][9] = '*';

    
    return rv_game;
}

/* Task 2 */
void free_state(game_state_t* state) {
  for (int row = 0; row < state->num_rows; ++row) {
      free(state->board[row]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  for (int row = 0; row < state->num_rows; ++row) {
      fprintf(fp, "%s\n", state->board[row]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
     return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
    if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
        return true;
    }

    return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
    if (is_head(c) || is_tail(c) || c == '>' || c == 'v' || c == '<' || c == '^') {
     return true;
  }
    return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {

    switch (c) {
        case '>':
            return 'd';
            break;
        case 'v':
            return 's';
            break;
        case '<':
            return 'a';
            break;
        case '^':
            return 'w';
            break;
        default: 
            return 'e';
    }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {

    switch (c) {
        case 'D':
            return '>';
            break;
        case 'S':
            return 'v';
            break;
        case 'A':
            return '<';
            break;
        case 'W':
            return '^';
            break;
        default: 
            return 'E';
    }
   }

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') {
      return cur_row + 1;
  }
  else if (c == '^' || c == 'w' || c == 'W') {
      return cur_row - 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') {
      return cur_col + 1;
  }
  else if (c == '<' || c == 'a' || c == 'A') {
      return cur_col - 1;
  }

    return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  snake_t snek = state->snakes[snum];
  unsigned int headR = snek.head_row;
  unsigned int headC = snek.head_col;

  char currChar = get_board_at(state, headR, headC);

  headR = get_next_row(headR, currChar);
  headC = get_next_col(headC, currChar);


  return get_board_at(state, headR, headC);
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  snake_t* snek = &(state->snakes[snum]);
  unsigned int headR = snek->head_row;
  unsigned int headC = snek->head_col;

  char currChar = get_board_at(state, headR, headC);

  set_board_at(state, headR, headC, head_to_body(currChar));

  headR = get_next_row(headR, currChar);
  headC = get_next_col(headC, currChar);

  set_board_at(state, headR, headC, currChar);

  snek->head_row = headR;
  snek->head_col = headC;

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  snake_t* snek = &(state->snakes[snum]);
  unsigned int tailR = snek->tail_row;
  unsigned int tailC = snek->tail_col;
  char currChar = get_board_at(state, tailR, tailC);

  set_board_at(state, tailR, tailC, ' ');

  tailR = get_next_row(tailR, currChar);
  tailC = get_next_col(tailC, currChar);

  char newTail = get_board_at(state, tailR, tailC);
  
  set_board_at(state, tailR, tailC, body_to_tail(newTail));

  snek->tail_row = tailR;
  snek->tail_col = tailC;

  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int i = 0; i < state->num_snakes; ++i) {
      snake_t* snek = &(state->snakes[i]);
      char nextSquare = next_square(state, i);
      if (is_snake(nextSquare) || nextSquare == '#') {
          set_board_at(state, snek->head_row, snek->head_col, 'x');
          snek->live = false;
      }
      else if (nextSquare == '*') {
          update_head(state, i);
          add_food(state);
      }
      else {
          update_head(state, i);
          update_tail(state, i);
      }
  }

  return;
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {

    int i = getc(fp);

    if (i == EOF) {
        return NULL;
    }

    game_state_t* rv_game = malloc(sizeof(game_state_t));

    unsigned int row_size = 1;
    unsigned int col_size = 1;

    rv_game->board = malloc(sizeof(char*));
    rv_game->board[col_size - 1] = malloc(sizeof(char));

    while (i) {
        char tile = (char) i;
        int next = getc(fp);

        if (i == '\n' || i == '\0') {
            rv_game->board[row_size - 1] = (char*) realloc((rv_game->board[row_size - 1]), 
(sizeof(char) * col_size));

            rv_game->board[row_size - 1][col_size - 1] = '\0';
            if (next != EOF) {
                col_size = 1;
                row_size += 1;
                rv_game->board = (char**) realloc((rv_game->board), (sizeof(char*) * row_size));
                rv_game->board[row_size - 1] = malloc(sizeof(char));
                i = next;
                continue;
            } else {
                break;
            }
        }

        rv_game->board[row_size - 1] = (char*) realloc((rv_game->board[row_size - 1]), (sizeof(char) * col_size));

        rv_game->board[row_size - 1][col_size - 1] = tile;

        col_size += 1;

        i = next;
    }

    rv_game->num_rows = row_size;
    rv_game->num_snakes = 0;
    rv_game->snakes = NULL;

  return rv_game;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  snake_t* snek = &(state->snakes[snum]);
    unsigned int currRow = snek->tail_row;
    unsigned int currCol = snek->tail_col;
    char currBody = get_board_at(state, currRow, currCol);
    
    while(!is_head(currBody)) {
        currRow = get_next_row(currRow, currBody);
        currCol = get_next_col(currCol, currBody);
        currBody = get_board_at(state, currRow, currCol);
    }
    snek->head_row = currRow;
    snek->head_col = currCol;
    
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  state->snakes = malloc(sizeof(snake_t));

  unsigned int currRow = 0;
  unsigned int num_snakes = 0;

  for (currRow = 0; currRow < state->num_rows; ++currRow) {
      unsigned int currCol = 0;
      char currTile = get_board_at(state, currRow, currCol);
      while (is_snake(currTile) || currTile == '#' || currTile == ' ' || currTile == '*') {
          if (is_tail(currTile)) {
              num_snakes += 1;
              snake_t newSnake;
              newSnake.live = true;
              newSnake.tail_row = currRow;
              newSnake.tail_col = currCol;
              state->snakes = (snake_t*) realloc(state->snakes, sizeof(snake_t)* num_snakes);
              state->snakes[num_snakes - 1] = newSnake;
              find_head(state, num_snakes - 1);
              }
          currCol += 1;
          currTile = get_board_at(state, currRow, currCol);
      }
  }
  state->num_snakes = num_snakes;

            
  return state;
}
