#include <stdio.h>
#include<winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <stdbool.h>

#define WIDTH 40
#define HEIGHT 40

char GC[HEIGHT][WIDTH];

/*__________________________HELPER FUNCTIONS__________________________*/

int minimum(int i, int j){
	if(i < j){return i;}else{return j;}
}

int range(int i, int j){
	if(i < j){return abs(j-i);}else{return abs(i-j);}
}

int pointerlen(char c[]){
	int i=1; while(c[i]!='\0'){i++;};return i;
}

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
	printf("\x1b[H");
	for(int i=0; i<HEIGHT;i++){
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

//Client 
void clientmenu(){
	//client menu loop
}


//Server
void servermenu(){
	//server menu loop
}


//Main Menu
void menu(int option){
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
	for(int i=0; i<WIDTH;i++){
		for(int j=0;j<HEIGHT;j++){
			GC[i][j] = ' ';
		}
	}
}

int main(int argc , char *argv[]){
	//run initialise program defaults/configs and set title screen
	init();

	int option = 0;

	menu(option);
	drawscreen();

	bool valid = false;
	while(!valid){
		char ip = getch();
		
		if(ip =='s' || ip =='S'){
			option++;
		}else if(ip == 'w' || ip =='W'){
			option--;
		}else{
			goto selectoropt;
		}

		if(option >2){
			option = 2;
		}else if(option < 0){
			option = 0;
		}else

		menu(option);
		drawscreen();

		selectoropt:
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
}