#include<ncurses.h>
#include<stdlib.h>

struct Snake{
	struct Snake *head;
	int pos_y , pos_x;
};

struct Snake *snake = NULL;

void make_the_snake(void);
void add_food(void);
void update_the_snake(int,int);
void move_the_snake(int);
bool is_snake_stuck(void);
bool is_boundary_reached(void);
void put_on_screen(void);

int main(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr , TRUE);
	curs_set(0);    //Hides the cursor.
	
	make_the_snake();	
	put_on_screen();
	//Moving the snake starts here.
	int ch;
	while( (ch = getch()) != 'q'){
		move_the_snake(ch);
		put_on_screen();
	}
	endwin();
	return 0;
}

#define INIT_SIZE 10

void make_the_snake(){
	snake = malloc(sizeof(struct Snake));
	snake->pos_y = LINES/2;
	snake->pos_x = COLS/2;
	struct Snake *temp = snake;
	for(int i = 1 ; i<= INIT_SIZE ; ++i){
		temp->head = malloc(sizeof(struct Snake));
		temp = temp->head;
		temp->pos_y = LINES/2;
		temp->pos_x = (COLS/2 + i);
	}
	temp->head = NULL;
}

//Incomplete
void add_food(){
	struct Snake *temp = snake;
	while(temp->head->head != NULL){
		temp = temp->head;
	}
	temp->head->head = malloc(sizeof(struct Snake));
	temp->head->pos_y = temp->pos_y;
	temp->head->pos_x = temp->pos_x;
}

void update_the_snake(int y , int x){
	struct Snake *temp = malloc(sizeof(struct Snake));
	temp->head = snake;
	temp->pos_y = y;
	temp->pos_x = x;
	snake = temp;

	while(temp->head->head != NULL)
		temp = temp->head;
	struct Snake *temp2 = temp->head;
	temp->head = NULL;
	free(temp2);
}

void move_the_snake(int direction){
	int y = snake->pos_y;
	int x = snake->pos_x;
	switch(direction){
		case KEY_UP:
			update_the_snake(--y , x);
			break;
		case KEY_DOWN:
			update_the_snake(++y , x);
			break;
		case KEY_LEFT:
			update_the_snake(y , --x);
			break;
		case KEY_RIGHT:
			update_the_snake(y , ++x);
			break;	
	}
}

bool is_snake_stuck(){
	bool ans = false;
	struct Snake *temp = snake->head->head;
	while(temp!=NULL){
		if((temp->pos_y == snake->pos_y) && (temp->pos_x == snake->pos_x)){
			ans = true;
			break;
		}
		temp = temp->head;	
	}
	return ans;
}

bool is_boundary_reached(){
	bool ans = false;
	if(snake->pos_y == 0 || snake->pos_x == 0 || snake->pos_y == LINES || snake->pos_x == COLS)
		ans = true;
	return ans;
}

void put_on_screen(){
	if(is_snake_stuck() || is_boundary_reached()){
		attron(A_BLINK);
		mvwprintw(stdscr , LINES - 1  , 1 , "%s" , "YOUR SNAKE IS DEAD!");
		getch();
		attroff(A_BLINK);
		endwin();
		exit(1) ;
	}

	wclear(stdscr);
	struct Snake *temp = snake;
	while(temp != NULL){
		mvwaddch(stdscr , temp->pos_y, temp->pos_x , '#');	
		temp = temp->head;
	}
	wrefresh(stdscr);
}
