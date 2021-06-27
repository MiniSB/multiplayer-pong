#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <stdbool.h>
#include <windows.h>

#define WIDTH 40
#define HEIGHT 40
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

int PORT= 8080;
char addr[15];
char GC[HEIGHT][WIDTH];

/*__________________________HELPER FUNCTIONS__________________________*/
//Move Cursor
void cursormove(int x, int y){
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {x, y};
    SetConsoleCursorPosition(hConsole, pos);
}

//Change Cursor Visibility
void cursor(bool visibility)
{
	CONSOLE_CURSOR_INFO cursor;    
	cursor.bVisible = visibility;    
	cursor.dwSize = sizeof(cursor);    
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);    
	SetConsoleCursorInfo(handle, &cursor);
}

//clear console screen
void clrscr(){ system("@cls||clear"); }

//Minimum of 2 values
int minimum(int i, int j){
	if(i < j){return i;}else{return j;}
}

//Range of 2 values
int range(int i, int j){
	if(i < j){return abs(j-i);}else{return abs(i-j);}
}

//Length of pointer
int pointerlen(char c[]){
	int i=1; while(c[i]!='\0'){i++;};return i;
}

//return position for text to be centred
int centertext(char c[]){
	return (WIDTH-pointerlen(c))/2;
}
/*_________________________GRAPHICS LIBRARY____________________________*/

void clearcanvas(){
	// for(int i=0; i<HEIGHT;i++){
	// 	for(int j=0;j<WIDTH;j++){
	// 		GC[i][j] = ' ';
	// 	}
	// }
	memset(GC, ' ', sizeof(GC[0][0]) * WIDTH * HEIGHT);
}

void drawscreen(){
	//Moves windows handle to console beginning
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
	for(int i=0; i<HEIGHT;i++){
		putchar('\r');
		for(int j=0;j<WIDTH;j++){
			putchar(GC[i][j]);
		}
		putchar('\n');
	}
}

void drawline(int xi, int yi, int xf, int yf, char c){
	//Check if line is constant or sloped
	if(xi == xf || yi == yf){
		if(xi == xf){
			//Draw horizontal line
			for (int i = 0; i < range(yi,yf) || range(yi,yf) == i; i++){ GC[minimum(yi,yf)+i][xi] = c;}	
		}else{
			for (int i = 0; i < range(xi,xf) || range(xi,xf) == i; i++){ GC[yi][minimum(xi,xf)+i] = c;}
		}
	}else{
		float grad = (yf-yi)/(xf-xi);
		float con = yi-(grad*xi);

		if(range(xi,xf) > range(yi,yf)){
			for(int i=minimum(xi,xf); i < (minimum(xi,xf)+range(xi,xf));i++){ GC[(int)round((grad*i)+con)][i] = c;}
		}else{
			for(int i=minimum(yi,yf); i < (minimum(yi,yf)+range(yi,yf));i++){ GC[i][(int)round((i-con)/grad)] = c;}
		}
	}
}

void drawchar(int x, int y, char c){
	if(x>=0 && x<=HEIGHT && y>=0 && y<=HEIGHT){
		GC[y][x] = c;
	}
}

//Initialises game state
void writescreen(int posx, int posy, char c[]){
	int len = pointerlen(c);
	// printf("%d", len);
	if(posy <= HEIGHT && posy > 0){
		int len = len;
		if((len + posx) > WIDTH){
			len = WIDTH - posx;
		}
		for(int i=0; i< len && c[i]!= '\0';i++){
			GC[posy][posx+i] = c[i];
		}
	}
}

void drawborder(){
	drawline(0, 0, 0, HEIGHT-1, '|');
	drawline(WIDTH-1, 0, WIDTH-1, HEIGHT-1, '|');
	drawline(0, 0, WIDTH-1, 0, '-');
	drawline(0, HEIGHT-1, WIDTH-1, HEIGHT-1, '-');
}
/*____________________________________________________________________*/
//change port
void portchange(){
	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Port"),10, "Changing Port");
	writescreen(centertext("New Port: "),20, "New Port: ");

	drawscreen();
	char prt[4];

	cursormove(centertext("New Port: ")+pointerlen("New Port: ")+1, 20);
	cursor(TRUE);
	scanf("%s", prt);
	sscanf(prt, "%d", &PORT);

	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Port"),10, "Changing Port");
	writescreen(centertext("Port Changed"), 20, "Port Changed");
	writescreen(centertext("Press any key to continue"), 35, "Press any key to continue");
	cursor(FALSE);
	drawscreen();
	getch();	
}

void addresschange(){
	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Address"),10, "Changing Address");
	writescreen(centertext("New Address: "),20, "New Address: ");

	drawscreen();
	cursormove(centertext("New Address: ")+pointerlen("New Address: ")+1, 20);
	cursor(TRUE);
	scanf("%s", addr);

	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Address"),10, "Changing Address");
	writescreen(centertext("Address Changed"), 20, "Address Changed");
	writescreen(centertext("Press any key to continue"), 35, "Press any key to continue");
	cursor(FALSE);
	drawscreen();
	getch();
}

//Client 
void clientmenu(){
	//client menu loop

	int option = 0;
	bool loop = false;

	while(!loop){
		clearcanvas();
		drawborder();

		// writescreen(centertext("Find Game"), 10, "Find Game");

		//Cast port to string
		char prt[4];
		sprintf(prt, "%d", PORT);

		//concate port message
		char cport[] = "Port: ";
		strcat(cport, prt);
		writescreen(centertext(cport), 10, cport);

		//concate address message
		char caddr[] = "Address: ";
		strcat(caddr, addr);
		writescreen(centertext(caddr), 11, caddr);

		if(option == 0){
			writescreen(centertext("1. Change Port"), 20, " -> 1. Change Port");
		}else{
			writescreen(centertext("1. Change Port"), 20, "1. Change Port");
		}

		if(option == 1){
			writescreen(centertext("2. Change Address"), 25, " -> 2. Change Address");
		}else{
			writescreen(centertext("2. Change Address"), 25, "2. Change Address");
		}

		if(option == 2){
			writescreen(centertext("3. Join Game"), 30, " -> 3. Join Game");
		}else{
			writescreen(centertext("3. Join Game"), 30, "3. Join Game");
		}

		if(option == 3){
			writescreen(centertext("4. Back"), 35, " -> 4. Back");
		}else{
			writescreen(centertext("4. Back"), 35, "4. Back");
		}
		drawscreen();

		char ip = getch();

		if(ip =='s' || ip =='S'){
			option++;
		}else if(ip == 'w' || ip =='W'){
			option--;
		}

		if(option >3){
			option = 0;
		}else if(option < 0){
			option = 3;
		}

		if(ip =='f' || ip =='F'){
			if(option == 0){
				portchange();
			}else if(option == 1){
				addresschange();
			}else if(option == 2){
				//Join game
			}else if(option == 3){
				loop = true;
			}
		};
	}
}


//Server
void servermenu(){
		
	int option = 0;
	bool loop = false;
	while (!loop){

		//server menu loop
		clearcanvas();
		drawborder();

		//Cast port to string
		char prt[4];
		sprintf(prt, "%d", PORT);

		//concate port message
		char cport[] = "Port: ";
		strcat(cport, prt);

		writescreen(centertext("Hosting Game"), 5, "Hosting Game");
		writescreen(centertext(cport), 10, cport);

		if(option == 0){
			writescreen(centertext("1. Change Port"), 20, " -> 1. Change Port");
		}else{
			writescreen(centertext("1. Change Port"), 20, "1. Change Port");
		}

		if(option == 1){
			writescreen(centertext("2. Create Game"), 25, " -> 2. Create Game");
		}else{
			writescreen(centertext("2. Create Game"), 25, "2. Create Game");
		}

		if(option == 2){
			writescreen(centertext("3. Back"), 30, " -> 3. Back");
		}else{
			writescreen(centertext("3. Back"), 30, "3. Back");
		}
		drawscreen();

		char ip = getch();

		if(ip =='s' || ip =='S'){
			option++;
		}else if(ip == 'w' || ip =='W'){
			option--;
		}

		if(option >2){
			option = 0;
		}else if(option < 0){
			option = 2;
		}

		if(ip =='f' || ip =='F'){
			if(option == 0){
				portchange();
			}else if(option == 1){
				//Create Game
			}else if(option == 2){
				loop = true;
			}
		};

	}
	
}


//Main Menu
void mainmenu(int option){
	//Set initial Game Screen
	clearcanvas();
	drawborder();
	writescreen(centertext("Pong, Multiplayer Game"), 10, "Pong, Multiplayer Game");

	if(option == 0){
		writescreen(centertext("1. Create Game"), 20, " -> 1. Create Game");
	}else{
		writescreen(centertext("1. Create Game"), 20, "1. Create Game");
	}

	if(option == 1){
		writescreen(centertext("2. Join Game"), 25, " -> 2. Join Game");
	}else{
		writescreen(centertext("2. Join Game"), 25, "2. Join Game");
	}

	if(option == 2){
		writescreen(centertext("3. Exit"), 30, " -> 3. Exit");
	}else{
		writescreen(centertext("3. Exit"), 30, "3. Exit");
	}
	// writescreen((WIDTH-pointerlen("Pong, Multipleayer Game"))/2);
}

void init(){
	//Set game board to empty
	clrscr();
	for(int i=0; i<WIDTH;i++){
		for(int j=0;j<HEIGHT;j++){
			GC[i][j] = ' ';
		}
	}
}

int main(int argc , char *argv[]){
	//Change Window Title
	SetConsoleTitle("Pong but better");
	cursor(FALSE);

	//run initialise program defaults/configs and set title screen
	init();

	int option = 0;

	bool valid = false;
	while(!valid){
		mainmenu(option);
		drawscreen();
		char ip = getch();
		
		if(ip =='s' || ip =='S'){
			option++;
		}else if(ip == 'w' || ip =='W'){
			option--;
		}

		if(option >2){
			option = 0;
		}else if(option < 0){
			option = 2;
		}

		if(ip =='f' || ip =='F'){
			if(option == 0){
				servermenu();
			}else if(option == 1){
				clientmenu();
			}else if(option == 2){
				valid = true;
			}
		};
	}
	//Exited Game loop here
	cursor(TRUE);
}