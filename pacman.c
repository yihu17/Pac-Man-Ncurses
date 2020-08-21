#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

//variables
//pacman coordinates 
int pxloc = 13, pyloc = 17;

//Win/lose conditions and other variables
int score = 0, lives = 3, pelletCount = 0, powerup = -1;

//ghost variables
int randomMove, level1 = 1, redo = 1, timer = 0, seconds = 0;

//ghost positions
int ghost[4][2] = {
		{7, 9},
		{13, 13},
		{7, 17},
		{13, 17},
};

//the variable that was replaced by a ghost
char ghostLastStep[4] = {' ', ' ', ' ', ' '};

int deadTime[4] = { 0, 0, 0, 0};

//score window
WINDOW *scoreWin;


//map
char map[23][27] = {
	{'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'},
	{'#','.','.','.','.','.','.','.','.','.','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','#'},
	{'#','O','#','#','#','#','.','#','#','#','#','#','.','#','.','#','#','#','#','#','.','#','#','#','#','O','#'},
	{'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#'},
	{'#','.','#','#','#','#','.','#','#','.','#','#','#','#','#','#','#','.','#','#','.','#','#','#','#','.','#'},
	{'#','.','.','.','.','.','.','#','#','.','.','.','.','#','.','.','.','.','#','#','.','.','.','.','.','.','#'},
	{'#','#','#','#','#','#','.','#','#','#','#','#',' ','#',' ','#','#','#','#','#','.','#','#','#','#','#','#'},
	{' ',' ',' ',' ',' ','#','.','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','.','#',' ',' ',' ',' ',' '},
	{' ',' ',' ',' ',' ','#','.','#','#',' ','#','#','#','-','#','#','#',' ','#','#','.','#',' ',' ',' ',' ',' '},
	{'#','#','#','#','#','#','.','#','#',' ','#',' ',' ',' ',' ',' ','#',' ','#','#','.','#','#','#','#','#','#'},
	{' ',' ',' ',' ',' ',' ','.',' ',' ',' ','#',' ',' ',' ',' ',' ','#',' ',' ',' ','.',' ',' ',' ',' ',' ',' '},
	{'#','#','#','#','#','#','.','#','#',' ','#',' ',' ',' ',' ',' ','#',' ','#','#','.','#','#','#','#','#','#'},
	{' ',' ',' ',' ',' ','#','.','#','#',' ','#','#','#','#','#','#','#',' ','#','#','.','#',' ',' ',' ',' ',' '},
	{' ',' ',' ',' ',' ','#','.','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','.','#',' ',' ',' ',' ',' '},
	{'#','#','#','#','#','#','.','#','#',' ','#','#','#','#','#','#','#',' ','#','#','.','#','#','#','#','#','#'},
	{'#','.','.','.','.','.','.','.','.','.','.','.','.','#','.','.','.','.','.','.','.','.','.','.','.','.','#'},
	{'#','.','#','#','#','#','.','#','#','#','#','#','.','#','.','#','#','#','#','#','.','#','#','#','#','.','#'},
	{'#','O','.','.','#','#','.','.','.','.','.','.','.','P','.','.','.','.','.','.','.','#','#','.','.','O','#'},
	{'#','#','#','.','#','#','.','#','#','.','#','#','#','#','#','#','#','.','#','#','.','#','#','.','#','#','#'},	
	{'#','.','.','.','.','.','.','#','#','.','.','.','.','#','.','.','.','.','#','#','.','.','.','.','.','.','#'},
	{'#','.','#','#','#','#','#','#','#','#','#','#','.','#','.','#','#','#','#','#','#','#','#','#','#','.','#'},
	{'#','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','#'},
	{'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'},			
};

//check if the move is valid for pacman
int validMove(int y, int x) {
	if(map[y][x] == '#' || map[y][x] == '-') {	//check if the position is a wall or gate
		return -1;			
	} else {
		return 1;			
	}
}

//check if the move is valid for ghost
int validMoveG(int y, int x) {
	if(map[y][x] == '#' || map[y][x] == '-' || map[y][x] == 'G') {	//check if the position is a wall or another ghost
		return -1;			
	} else {
		return 1;			
	}
}

//picking up pellets and adding to score
void pickUpPellets(int y, int x) {
	if (map[y][x] == '.' || map[y][x] == 'O') {				//check if the next position is a pellet
		pelletCount++;										//add pellet count
		
		score++;											//add score by 1
		mvwprintw(scoreWin, 1, 1, "Score: %d", score);		//print score
		wrefresh(scoreWin);
		
		if (map[y][x] == 'O') {								//if the pellet is a power pellet
			powerup = 1;									//activate power pellet
		} 	
	}  
}

//identify if pacman hits a ghost
void hitGhost() {
	map[pyloc][pxloc] = ' ';							//make pacman location to be blank
	mvaddch(pyloc, pxloc, map[pyloc][pxloc]);			//print that out
		
	pxloc = 13; 										//reseting pacman coordinates to the start
	pyloc = 17;
	
	map[pyloc][pxloc] = 'P';							//set the start array content to be pacman
	
	attron(COLOR_PAIR(3));
	mvaddch(pyloc, pxloc, 'P');							//print out pacman	
	attroff(COLOR_PAIR(3));    
	
	move(pyloc, pxloc);									//set cursor to be at the start
	
	lives--;											//minus 1 to life
	mvwprintw(scoreWin, 2, 1, "Lives: %d", lives);		//update live display
	wrefresh(scoreWin);
}

//pacman eating the ghost when it has power up
void eatGhost(int y, int x) {
	score += 200;										//increase the score by 200
	mvwprintw(scoreWin, 1, 1, "Score: %d", score);		//print score
	wrefresh(scoreWin);
	
	int ghostNum;											//varaible to identify which ghost got eaten
	
	for (int i = 0; i < 4; i++) {						//loop through the array of ghosts to find which ghost got eaten
		for (int j = 0; j < 1; j++) {
			if (ghost[i][j] == y && ghost[i][j+1] == x) {	//by look at which ghost is at pacman's coordinates
				ghostNum = i;									//store the identity
			}
		}
	}
	
	int ghosty = ghost[ghostNum][0];					//identify y and x of the ghost being eaten
	int ghostx = ghost[ghostNum][1];
	
	map[ghosty][ghostx] = ' ';						//set that coordinate to be blank
	mvaddch(ghosty, ghostx, map[ghosty][ghostx]);	//print out that blank space
	
	switch (ghostNum) {									//set the ghost coordinates to its starting coordinate
		case 0:										//depending on which ghost it is
			ghost[0][0] = 7;
			ghost[0][1] = 9;
			break;
		
		case 1:
			ghost[1][0] = 13;
			ghost[1][1] = 9;
			break;
			
		case 2:
			ghost[2][0] = 7;
			ghost[2][1] = 17;
			break;
			
		case 3:
			ghost[3][0] = 13;
			ghost[3][1] = 17;
			break;
	}
	
	ghosty = ghost[ghostNum][0];						//set temporary y and x of the starting coordinate
	ghostx = ghost[ghostNum][1];						//for the ghost that got eaten
	
	map[ghosty][ghostx] = 'G';						//set the coordinate to be the ghost
	
	ghostLastStep[ghostNum] = ' ';						//reseting the ghost last step
	
	attron(COLOR_PAIR(4));
	mvaddch(ghosty, ghostx, map[ghosty][ghostx]);	//print out the ghost on the starting coordinate
	attroff(COLOR_PAIR(4));
	
	deadTime[ghostNum] = 1;								//start dead timer
}

//pacman move to the new location
void doMove(int y, int x) {
	if (map[y][x] == 'G') {			//check if the new array is a ghost
		if (powerup == -1) {
			hitGhost();				//hitting ghost function	
		} else {
			eatGhost(y, x);
		}
	} else {
		pickUpPellets(y, x);		//else it picks up the pellets
		
		//move pacman
		map[pyloc][pxloc] = ' ';							//set position on the array to be blank
		mvaddch(pyloc, pxloc, map[pyloc][pxloc]);			//print out the content of the array on pacman's location
	
		pxloc = x;											//set the coordinates to be pacman's coordinates
		pyloc = y;
	
		map[pyloc][pxloc] = 'P';							//set the next array content to be pacman
		
		if (powerup == 1) {									//if power up is active
			curs_set(1);									//set cursor visible to inicate the powerup
		} else {
			curs_set(0);
		}
		
		attron(COLOR_PAIR(3));
		mvaddch(pyloc, pxloc, map[pyloc][pxloc]);			//print out pacman on new position with color
		attron(COLOR_PAIR(3));
	}
}

//teleporting pacman from one side of the map to the other
void warp(int direction) {
	
	map[pyloc][pxloc] = ' ';							//make pacman location to be blank
	mvaddch(pyloc, pxloc, map[pyloc][pxloc]);			//print that out
	
	if (direction == 1) {
		pxloc = 26; 									//setting the coordinate
		pyloc = 10;
	} else {	
		pxloc = 0; 											
		pyloc = 10;							
	}
	
	doMove(pyloc, pxloc);								//move pacman
}

//moving pacman depending on the input direction
void pacMove(int key, int y, int x) {
	switch (key) {
		case KEY_UP:
			y--;												//adjust coordinates depending on direction
			if (validMove(y, x) == 1) {							//check if direction is valid
				doMove(y, x);									//do the move if its valid
			} else {
				y++;											//else revert the coordinates back
			}
			break;
		
		case KEY_DOWN:
			y++;
			if (validMove(y, x) == 1) {
				doMove(y, x);	
			} else {
				y--;
			}
			break;
			
		case KEY_LEFT:
			x--;
			if (validMove(y, x) == 1) {
				doMove(y, x);	
			} else if (x == -1 && y == 10) {					//if those are the next coordinates then warp
						warp(1);
				} else {
					x++;
				}
			break;
			
		case KEY_RIGHT:
			x++;
			if (validMove(y, x) == 1) {
				doMove(y, x);	
			} else if (x == 27 && y == 10) {
						warp(0);
				} else {
					x--;
				}
			break;
			
		default:
			break;
	}
}

//ghost dying if it runs into pacman
void die(int ghostNum) {
	score += 200;										//increase the score by 200
	mvwprintw(scoreWin, 1, 1, "Score: %d", score);		//print score
	wrefresh(scoreWin);
	
	switch (ghostNum) {			//set the respective ghost back to its starting positions
		case 0:
			ghost[0][0] = 7;
			ghost[0][1] = 9;
			break;
		
		case 1:
			ghost[1][0] = 13;
			ghost[1][1] = 9;
			break;
			
		case 2:
			ghost[2][0] = 7;
			ghost[2][1] = 17;
			break;
			
		case 3:
			ghost[3][0] = 13;
			ghost[3][1] = 17;
			break;
	}
	
	ghostLastStep[ghostNum] = ' ';		//reseting the ghost last step
	
	int ghosty = ghost[ghostNum][0];	//set temporary y and x of the starting coordinate
	int ghostx = ghost[ghostNum][1];
	
	map[ghosty][ghostx] = 'G';			//set that coordinate to be the ghost
	
	attron(COLOR_PAIR(4));
	mvaddch(ghosty, ghostx, map[ghosty][ghostx]);	//print out the ghost
	attroff(COLOR_PAIR(4));
}

//calculating which way to move in the x axis
int moveInX (int px, int gx) {
	if (px > gx) {			//try to get the ghost's x value closer to pacman's x vaue
		gx++;
		redo = 1;			//set the redo value
	} else {
		gx--;
		redo = 2;
	}
	return gx;
}

//calculating which way to move in the y axis
int moveInY (int py, int gy) {
	if (py > gy) {			//try to get the ghost's x value closer to pacman's x vaue
		gy++;
		redo = 1;			//set the redo value
	} else {
		gy--;
		redo = 2;
	}
	return gy;
}

void ghostMove(int ghostNum) {
	int y = ghost[ghostNum][0];			//setting x and y to be the ghosts x and y
	int x = ghost[ghostNum][1];

	int tempty = y;						//temporary x and y to be tested
	int temptx = x;
	
	map[y][x] = ghostLastStep[ghostNum];	//make the coordinate of the map to be the the variable that the ghost overlapped
	
	if (map[y][x] == '.' || map[y][x] == 'O') {
		attron(COLOR_PAIR(2));			
		mvaddch(y, x, map[y][x]);			//print out the orignal variable back onto the map with colors
		attroff(COLOR_PAIR(2));	
	} else {
		mvaddch(y, x, map[y][x]);			//else print out the blank space
	}

	if (level1 == 1) {					//if level1 is active, ghosts will move in a random direction
		do {							//move in a random direction as long as its valid
			tempty = y;					//resetting tempt x and y to be the real x and y
			temptx = x;
			
			randomMove = rand() %4;		//generate a random number 1-4
			
			switch (randomMove) {		//depending on the number it will move in a random direction
				case 0:
					tempty++;
					break;
				
				case 1:
					tempty--;
					break;
					
				case 2:
					temptx++;
					if (temptx == 27 && tempty == 10) {	//warp
						temptx = 0;
						tempty = 10;
					}
					break;
					
				case 3:
					temptx--;
					if (temptx == -1 && tempty == 10) {	//warp
						temptx = 26;
						tempty = 10;
					}
					break;
			}
		} while (validMoveG(tempty, temptx) != 1);	//unless the move is valid, it will keep trying to move in a random direction
	
	/*
	 * AI is not complete, have not implement going around a wall if its between ghost and pacman
	 * but can do basic chase
	 */
	} else {										//or ghost will move towards pacman 
		int diffy = abs(pyloc - tempty);			//calculating the absolute difference in x and y coordinate of pacman and the ghost
		int diffx  = abs(pxloc - temptx);
		
		if (diffx > diffy) {						//it will move axis that has a bigger difference
			//move in x direction
			temptx = moveInX(pxloc, temptx);		//calculate which direction of the x axis should the ghost move to
			
			if (validMoveG(tempty, temptx) == -1) {	//if that direction is blocked
				if (redo == 1) {					//redo the move
					temptx--;
				} else {
					temptx++;
				}
				
				tempty = moveInY(pyloc, tempty);	//calculate and move in y axis instead
					
				if (validMoveG(tempty, temptx) == -1) {	//if that move is block 
					if (redo == 1) {					//redo the move
						tempty--;
					} else {
						tempty++;
					}
				}
			}
				
		} else {
			//move in y direction
			tempty = moveInY(pyloc, tempty);		//same algorithm as moving in the x axis as about but instead in the y axis
			
			if (validMoveG(tempty, temptx) == -1) {	
				if (redo == 1) {
					tempty--;
				} else {
					tempty++;
				}
				
				
				temptx = moveInX(pxloc, temptx);
				
				if (validMoveG(tempty, temptx) == -1) {
					if (redo == 1) {
						temptx--;
					} else {
						temptx++;
					}
				} 
			}
		}
	}
	
	y = tempty;										//set real x and y to be the tempt x and y
	x = temptx;
	
	if (map[y][x] == 'P' && powerup == -1) {		//check if that coordinate is pacman and powerup is not active
		hitGhost();									// do hitGhost function
	} 
	
	if (map[y][x] == 'P' && powerup == 1) {			//check if that coordinate is pacman and power is active 
		die(ghostNum);								//do die function
		deadTime[ghostNum] = 1;						//start dead timer
	} else {
		ghostLastStep[ghostNum] = map[y][x];		//store the variable that the ghost is overlapping to the array
		
		map[y][x] = 'G';						//set the new x and y map variable to be the ghost
		
		ghost[ghostNum][0] = y;					//set the coordinates of the ghost to be the new x and y
		ghost[ghostNum][1] = x;
		
		attron(COLOR_PAIR(4));
		mvaddch(y, x, map[y][x]);				//print out the ghost on the new location with color
		attroff(COLOR_PAIR(4));
	}
}


//printing out the map
void printMap() {
	for (int i = 0; i < 23; i++) {
		for (int j = 0; j < 27; j++) {
			switch (map[i][j]) {				//depending on the variable, it will print the variable with its respective colours
				case '#':
					attron(COLOR_PAIR(1));
					addch(map[i][j]);
					attroff(COLOR_PAIR(1));
					break;
				
				case '.':
					attron(COLOR_PAIR(2));
					addch(map[i][j]);
					attroff(COLOR_PAIR(2));
					break;
					
				case 'O':
					attron(COLOR_PAIR(2));
					addch(map[i][j]);
					attroff(COLOR_PAIR(2));
					break;
					
				case 'P':
					attron(COLOR_PAIR(3));
					addch(map[i][j]);
					attroff(COLOR_PAIR(3));
					break;
					
				case 'G':
					attron(COLOR_PAIR(4));
					addch(map[i][j]);
					attroff(COLOR_PAIR(4));
					break;
					
				default:
					addch(map[i][j]);
					break;
			}
		}	
		printw("\n");
	}
	refresh();	
}

int main() {
	//pacman logo
	char *logo[10] = {
		" ________________________________________________________ \n" ,
		"|  ____________________________________________________  |\n" ,
		"| |  _____        _____                             _  | |\n" ,
		"| | |     \\ /\\   /     /      |\\    /|    /\\   |\\  | | | |\n" ,
		"| | |   O  /  \\ |     /   __  | \\  / |   /  \\  | \\ | | | |\n" ,
		"| | |  ___/  O \\|    <   |__| |  \\/  |  /  O \\ |  \\| | | |\n" ,
		"| | | |  /      \\     \\       |      | /      \\|     | | |\n" ,
		"| | |_| /________\\_____\\      |______|/________\\_____| | |\n" ,
		"| |____________________________________________________| |\n",
		"|________________________________________________________|\n" ,
	};
	
	int keypressed;
	
	srand(time(0));
	
	initscr();				//start curses mode
	raw();					//disable line buffering, so user doesn't need to press enter for every input
	keypad(stdscr, TRUE); 	//enable special key presses such as arrow keys
	noecho();				//Don't echo while theres getch()
	start_color();			//enable colours
	
	init_pair(1, COLOR_BLUE, COLOR_BLACK);		//font blue with black background
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);	//then its the same for the following with their respective colours
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_CYAN, COLOR_BLACK);
	
	//printing logo
	for (int i = 0; i < 10; i++) {
		printw("%s", logo[i]);
	}
	
	curs_set(0);	//hide the cursor
	
	int pick, highlighted = 0;
	
	//printing level selection menu
	while (pick != 10) {						//while pick is not enter
		for (int i = 0; i < 2; i++) {			//score through number of levels
			if (i == highlighted) {				//if the that level is highlighted
				attron(A_REVERSE);				//reverse the backgound and foreground colours
			}
			
			mvprintw(12+i,24, "Level %d", level1+i);	//print out the level option
			attroff(A_REVERSE);					//turn attribute off
		}
		
		pick = getch();							//get user input
		
		switch(pick) {							//if its up or down, the highlighter goes accordingly
			case KEY_UP:
				highlighted--;
				if (highlighted < 0) {			//if it goes under 0 or over 1, set it back to 1 or 0 so highlight won't dissapear
					highlighted = 1;
				}
				break;
				
			case KEY_DOWN:
				highlighted++;
				if (highlighted > 1) {
					highlighted = 0;
				}
				break;
		}
	}
	
	if (highlighted == 1) {						//if highlighted number is 1 then user was highlighting level 2
		level1 = 2;								//set level to level 2
	}
	
	clear();	    //clear the screen
	
	printMap();		//print out the map	
	
	mvprintw(22, 28, "Press X to exit");
	
	int sHeight = 4, sWidth = 13, sYStart = 0, sXStart = 28;		//set values for score window
	
	scoreWin = newwin(sHeight, sWidth, sYStart, sXStart);			//initialise score window
	box(scoreWin, 0, 0);											//put a box on the window
	mvwprintw(scoreWin, 1, 1, "Score: %d", score);					//print out lives and score
	mvwprintw(scoreWin, 2, 1, "Lives: %d", lives);
	wrefresh(scoreWin);							

	move(17,13);	//move the cursor to where pacman is	
	
	//infinite loop
	while(keypressed != 'X' && lives > 0 && pelletCount < 204) {
		refresh();	
		timeout(250);							//timer in 200 miliseconds
		keypressed = getch();					//read user input
		
		if (keypressed == 'P') {
			timeout(-1);
			while (keypressed != 'P') {
				keypressed = getch();
			}
		}
		
		pacMove(keypressed, pyloc, pxloc);		//move pacman	
		
		//moving the ghost
		for (int i = 0; i < 4; i++) {
			if (deadTime[i] == 0) {				//check if ghost is dead
				ghostMove(i);					//move the ghosts
			} else {
				deadTime[i] += 250;				//else add to the dead timer
			}
			
			if (deadTime[i] > 10000) {			//if the dead timer is more than 10 seconds
				deadTime[i] = 0;				//set it to 0 so ghostcan move again
			}
		}		
		
		move(pyloc, pxloc);						//move cursor back to pacman's location		
		
		//function of the duration of the powerup
		if (powerup == 1) {						//if powerup is active
			timer = timer + 500;				//start the timer
			
			if (timer > 1000) {				//if timer is 1000, then its 1 second
				timer = 0;						//reset timer
				seconds++;						//increase seconds
			}
			
			if (seconds > 10) {					//if seconds is more than 10 
				seconds = 0;					//reset seconds
				powerup = -1;					//turn off power up
			}
		}
	}
	
	timeout(-1);		//remove timer for input
	
	delwin(scoreWin);	//close scoreWin
	
	clear();
	
	int eHeight = 6, eWidth = 35, eYStart = 0, eXStart = 0;					//set values for end game window
	
	WINDOW *endWin = newwin(eHeight, eWidth, eYStart, eXStart);				//initialise endgame window
	box(endWin, 0, 0);														//put a box on the window
	refresh();
	
	if (lives == 0) {														//prints out end game message depending on how user gets there
		mvwprintw(endWin, 1, 1, "You Lose...");
		mvwprintw(endWin, 2, 1, "You score a total of %d points!", score);
		mvwprintw(endWin, 4, 1, "Press X to exit");
	} else if (pelletCount == 204) {
		mvwprintw(endWin, 1, 1, "YOU WIN!!!");
		mvwprintw(endWin, 2, 1, "You score a total of %d points!", score);
		mvwprintw(endWin, 4, 1, "Press X to exit");
	} else { 
		mvwprintw(endWin, 1, 1, "You Forfeited...");
		mvwprintw(endWin, 2, 1, "You score a total of %d points!", score);
		mvwprintw(endWin, 4, 1, "Press X to exit");
	}	
	
	wrefresh(endWin);
	
	keypressed = getch();
	
	while (keypressed != 'X') {		//will only exit if user press X
		keypressed = getch();
	}
	
	delwin(endWin);
	
	endwin();		//end curses mode
	
	return 0;
}

