#include <stdio.h>
#include<winsock2.h>
#include <stdlib.h>
#include <string.h>
#include<io.h>
#include <stdlib.h>

void clrscr(){ system("@cls||clear"); }

int minimum(int i, int j){
	if(i < j){return i;}else{return j;}
}

int range(int i, int j){
	if(i < j){return abs(j-i);}else{return abs(i-j);}
}


#define WIDTH 40
#define HEIGHT 40

char GC[HEIGHT][WIDTH];

void clearcanvas(){
	for(int i=0; i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++){
			GC[i][j] = ' ';
		}
	}
}

void drawscreen(){
	clrscr();
	for(int i=0; i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++){
			printf("%c", GC[i][j]);
		}
		printf("\n");
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

	}
}

void drawchar(int x, int y, char c){
	if(x>=0 && x<=HEIGHT && y>=0 && y<=HEIGHT){
		GC[y][x] = c;
	}
}

//Initialises game state
void writescreen(int posx, int posy, char c[], int len){
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


void init(){
	//Set game board to empty
	for(int i=0; i<WIDTH;i++){
		for(int j=0;j<HEIGHT;j++){
			GC[i][j] = ' ';
		}
	}

	//Set initial Game Screen

	drawline(0, 0, 0, HEIGHT-1, '|');
	drawline(WIDTH-1, 0, WIDTH-1, HEIGHT-1, '|');
	drawline(0, 0, WIDTH-1, 0, '-');
	drawline(0, HEIGHT-1, WIDTH-1, HEIGHT-1, '-');
	drawline(20, 10, 20, 10, 'a');
	writescreen(1, 10, "string" , 5);
	drawchar(20, 20, '7');
	drawscreen();
}

int main(int argc , char *argv[]){
	//run initialise program defaults/configs
	clrscr();
	init();

}