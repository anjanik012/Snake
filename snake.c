//This code is released under
//     ###################################################
//     #   DO WHAT THE F*CK YOU WANT TO PUBLIC LICENSE   #
//     #                                                 #
//     # ORIGINAL AUTHOR:- ANJANI KUMAR 	         #
//     # EMAIL:-	   anjanik012@gmail.com          #
//     # GITHUB:-          https://github.com/anjanik012 #
//     # GITLAB:-	   https://gitlab.com/anjanik012 #	
//     ###################################################

#include<ncurses.h>       
#include<pthread.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>

//To store the co-ordinates of snake elements.
struct Snake{
	struct Snake *head;
	unsigned int pos_y;
	unsigned int pos_x;
};
//To store the co-ordinates of food.
struct Food{
	unsigned int pos_y;
	unsigned int pos_x;
};

//A Window in which the snake runs.
WINDOW *playing_area = NULL;

//Staring positions of the playing area.
unsigned int starty;
unsigned int startx;

//Relative dimensions of the playing area.
#define WLINES 20
#define WCOLS 70

void create_playing_area(void);

//Global declarations.
struct Snake *snake = NULL;
struct Food food; 

//Flag to indicate requirement of food.
bool food_required = false;

//Score of the player.
unsigned int score = 0;

//Direction of snake.(Like a Mutex)
struct Dir_handler{
	unsigned int current_direction;
	bool changable;
};
struct Dir_handler Direction;

//Functions. Names are self explanatory.
void make_the_snake(void);
void add_food(void);
bool is_the_food_eaten(void);
void update_the_snake(unsigned int,unsigned int);
void grow_the_snake(unsigned int,unsigned int);
void move_the_snake(void);
bool is_snake_stuck(void);
bool is_boundary_reached(void);
bool is_snake_dead(void);
void update_score(void);
void put_on_screen(void);

//To create a separate thread.

pthread_t direction_thread;

//This function runs on a separate thread from main().
//Which listens for direction changes in snake motion.
void *director(){
	unsigned int d_input;
	while(true){
		d_input = getch();
		if(d_input == 'q')
			break;

		if(Direction.changable == true){			
			if(Direction.current_direction == KEY_LEFT || Direction.current_direction == KEY_RIGHT){
				if(d_input == KEY_UP || d_input == KEY_DOWN)
					Direction.current_direction = d_input;
			}
			else{
				if(d_input == KEY_LEFT || d_input == KEY_RIGHT)
					Direction.current_direction = d_input;
			}
		}		
	}
	endwin();
	exit(1);
}

int main()
{
	initscr();       	//Start of curses mode.	
	
	//Checking minimal terminal size.
	if(LINES <= WLINES || COLS<= WCOLS){
		endwin();
		printf("Terminal size must be atleast %d X %d \n" , WLINES , WCOLS);
		return 0;
	}

	cbreak();	 	//Disable line buffering.
	noecho();	  	//Disable echoing of characters.
	keypad(stdscr , TRUE);  //Enable arrow keys.
	curs_set(0);     	//Hides the cursor.
	starty = (LINES -WLINES)/2;
	startx = (COLS - WCOLS)/2;

	srand(time(0));		//Seed the random number generator.
	
	create_playing_area();
	make_the_snake();
	add_food();
	put_on_screen();	
	Direction.current_direction = KEY_LEFT;	//Snake moves left in the beginning.
	Direction.changable = true;

	//Creating a separate thread to listen for direction changes.
	pthread_create(&direction_thread , NULL , director , NULL);

	//Snake starts moving.
	while(true){
		move_the_snake();
		if(food_required){
			add_food();
			food_required = false;
			update_score();
		}
		put_on_screen();
	}
	return 0;
}

void create_playing_area(){
	playing_area = newwin(WLINES, WCOLS ,starty , startx);
	box(playing_area , 0 , 0);
	wrefresh(playing_area);
}

#define INIT_SIZE 3

void make_the_snake(){
	snake = malloc(sizeof(struct Snake));
	snake->pos_y = WLINES/2;
	snake->pos_x = WCOLS/2;
	struct Snake *temp = snake;
	for(int i = 1 ; i< INIT_SIZE ; ++i){
		temp->head = malloc(sizeof(struct Snake));
		temp = temp->head;
		temp->pos_y = WLINES/2;
		temp->pos_x = (WCOLS/2 + i);
	}
	temp->head = NULL;
}

void add_food(){
	struct Snake *temp = NULL;
	do{
		temp = snake;
	     	food.pos_y = 1 + (rand() % (WLINES-2));
             	food.pos_x = 1 + (rand() % (WCOLS-2));
		while(temp != NULL){
			if(food.pos_y == temp->pos_y && food.pos_x == temp->pos_x)
				break;
			temp = temp->head;	
		}	
	}while(temp != NULL);
}

bool is_the_food_eaten(){
	bool ans = false;
	if(snake->pos_y == food.pos_y && snake->pos_x == food.pos_x)
		ans = true;
	return ans;
}

void update_the_snake(unsigned int y , unsigned int x){
	struct Snake *temp = malloc(sizeof(struct Snake));
	temp->head = snake;
	temp->pos_y = y;
	temp->pos_x = x;
	snake = temp;

	//Delete an element from the tail.
	while(temp->head->head != NULL)
		temp = temp->head;
	struct Snake *temp2 = temp->head;
	temp->head = NULL;
	free(temp2);
}

void grow_the_snake(unsigned int y, unsigned int x){
	struct Snake *temp = malloc(sizeof(struct Snake));
	temp->head = snake;
	temp->pos_y = y;
	temp->pos_x = x;
	snake = temp;
}

void move_the_snake(){
	Direction.changable = false;
	unsigned int y = snake->pos_y;
	unsigned int x = snake->pos_x;
	if(is_the_food_eaten()){
		food_required = true;
		switch(Direction.current_direction){
			case KEY_UP:
				grow_the_snake(--y , x);
				break;
			case KEY_DOWN:
				grow_the_snake(++y , x);
				break;
			case KEY_LEFT:
				grow_the_snake(y , --x);
				break;
			case KEY_RIGHT:
				grow_the_snake(y , ++x);
				break;
		}	
	}
	else{
		switch(Direction.current_direction){
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
	Direction.changable = true;
}

bool is_snake_stuck(){
	bool ans = false;
	struct Snake *temp = snake->head->head->head;
	while(temp!=NULL){
		if(temp->pos_y == snake->pos_y && temp->pos_x == snake->pos_x){
			ans = true;
			break;
		}
		temp = temp->head;	
	}
	return ans;
}

bool is_boundary_reached(){
	bool ans = false;
	if(snake->pos_y == 0|| snake->pos_x == 0 || snake->pos_y ==  WLINES - 1 || snake->pos_x == WCOLS - 1)
		ans = true;
	return ans;
}

bool is_snake_dead(){
	if(is_snake_stuck() || is_boundary_reached())
		return true;	
	else
		return false;
}

void update_score(){
	score++;
}

void put_on_screen(){
	//Check if snake is dead.
	if(is_snake_dead()){
		pthread_cancel(direction_thread);
		attron(A_BLINK);
		mvwprintw(stdscr , starty + WLINES + 1  ,startx , "%s" , "YOUR SNAKE IS DEAD!");
		getch();
		attroff(A_BLINK);
		endwin();
		printf("You Died!!!\n");
		if(is_snake_stuck())
			printf("Don't touch yourself!!!\n");
		else
			printf("Mind the boundaries!!!\n");
		printf("You ate $%d\n", score);
		exit(1) ;
	}

	usleep(80000);	//This call is not in the right place.
	
	//Clearing previous frame.
	wclear(stdscr);
	wclear(playing_area);

	//Drawing the boundary of the playing area.
	box(playing_area , 0 , 0);
	
	//put the snake on screen.
	struct Snake *temp = snake;
	while(temp != NULL){
		if(temp == snake)
			mvwaddch(playing_area , temp->pos_y , temp->pos_x , '@');
		else
			mvwaddch(playing_area , temp->pos_y , temp->pos_x , 'o');	
		temp = temp->head;
	}
	
	//Put the food on screen.
	if(!is_the_food_eaten())	
		mvwaddch(playing_area , food.pos_y , food.pos_x , '$');

	//Quit message.
	mvwprintw(stdscr , starty + WLINES + 1 , startx , "%s" , "Press q to quit");	

	//Current Direction
	switch(Direction.current_direction){
		case KEY_UP:
			 mvwaddch(stdscr , starty + WLINES + 1 , startx + WCOLS-14 , 'U');
			 break;
		case KEY_DOWN:
			 mvwaddch(stdscr , starty + WLINES + 1 , startx + WCOLS-14 , 'D');
			 break;
		case KEY_LEFT:	 
			 mvwaddch(stdscr , starty + WLINES + 1 , startx + WCOLS-14 , 'L');
			 break;
		case KEY_RIGHT:
			 mvwaddch(stdscr , starty + WLINES + 1 , startx + WCOLS-14 , 'R');
			 break;
	}

	//Put Score on screen.
	mvwprintw(stdscr, starty + WLINES + 1 , startx + WCOLS - 10 , "Score:%d", score); 
	
	//Finally display the completed frame.
	wrefresh(stdscr);
	wrefresh(playing_area);
}
