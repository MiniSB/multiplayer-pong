#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <stdbool.h>
#include <windows.h>

//Socket Libraries
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define WIDTH 40
#define HEIGHT 40
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))

//Port and address for connections
int PORT = 8080;
char ADDRESS[15];

//
WSADATA wsa;
SOCKET s , new_socket;
struct sockaddr_in server , client;
char server_reply[2000];
int recv_size;

//Game board dimensions
char GC[HEIGHT][WIDTH];

//in game bool
bool ingame = false;
bool host = false;
bool loading = false;

//OPPONENT STRUCT
int opponent_x;
int user_x;

struct _ball{
	int x;int y;int dx;int dy;int ox; int oy;
};

struct _ball ball;
int sscore;
int cscore;
/*__________________________HELPER FUNCTIONS__________________________*/
//Move Cursor
void cursormove(int x, int y)
{
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
void clrscr() { system("@cls||clear"); }

//Minimum of 2 values
int minimum(int i, int j)
{
	if (i < j){return i;}else{return j;	}
}

//Range of 2 values
int range(int i, int j)
{
	if (i < j)
	{
		return abs(j - i);
	}
	else
	{
		return abs(i - j);
	}
}

//Length of pointer
int pointerlen(char c[])
{
	int i = 1;
	while (c[i] != '\0')
	{
		i++;
	};
	return i;
}

//return position for text to be centred
int centertext(char c[]){
	return (WIDTH - pointerlen(c)) / 2;
}

int ctoi(char c){
	int i = c -0; if(c == '0'){return 0;}return i;
}
/*_________________________GRAPHICS LIBRARY____________________________*/

void clearcanvas()
{
	memset(GC, ' ', sizeof(GC[0][0]) * WIDTH * HEIGHT);
}

void drawscreen()
{
	//Moves windows handle to console beginning
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = {0, 0};
	SetConsoleCursorPosition(hConsole, pos);
	for (int i = 0; i < HEIGHT; i++)
	{
		putchar('\r');
		for (int j = 0; j < WIDTH; j++)
		{
			putchar(GC[i][j]);
		}
		putchar('\n');
	}
}

void drawline(int xi, int yi, int xf, int yf, char c)
{
	//Check if line is constant or sloped
	if (xi == xf || yi == yf)
	{
		if (xi == xf)
		{
			//Draw horizontal line
			for (int i = 0; i < range(yi, yf) || range(yi, yf) == i; i++)
			{
				GC[minimum(yi, yf) + i][xi] = c;
			}
		}
		else
		{
			for (int i = 0; i < range(xi, xf) || range(xi, xf) == i; i++)
			{
				GC[yi][minimum(xi, xf) + i] = c;
			}
		}
	}
	else
	{
		float grad = (yf - yi) / (xf - xi);
		float con = yi - (grad * xi);

		if (range(xi, xf) > range(yi, yf))
		{
			for (int i = minimum(xi, xf); i < (minimum(xi, xf) + range(xi, xf)); i++)
			{
				GC[(int)round((grad * i) + con)][i] = c;
			}
		}
		else
		{
			for (int i = minimum(yi, yf); i < (minimum(yi, yf) + range(yi, yf)); i++)
			{
				GC[i][(int)round((i - con) / grad)] = c;
			}
		}
	}
}

void drawchar(int x, int y, char c)
{
	if (x >= 0 && x <= HEIGHT && y >= 0 && y <= HEIGHT)
	{
		GC[y][x] = c;
	}
}

//Initialises game state
void writescreen(int posx, int posy, char c[])
{
	int len = pointerlen(c);
	// printf("%d", len);
	if (posy <= HEIGHT && posy > 0)
	{
		int len = len;
		if ((len + posx) > WIDTH)
		{
			len = WIDTH - posx;
		}
		for (int i = 0; i < len && c[i] != '\0'; i++)
		{
			GC[posy][posx + i] = c[i];
		}
	}
}

void drawborder()
{
	drawline(0, 0, 0, HEIGHT - 1, '|');
	drawline(WIDTH - 1, 0, WIDTH - 1, HEIGHT - 1, '|');
	drawline(0, 0, WIDTH - 1, 0, '-');
	drawline(0, HEIGHT - 1, WIDTH - 1, HEIGHT - 1, '-');
}

void screenmessage(char c[]){
	clearcanvas();
	drawborder();
	writescreen(centertext(c), 20, c);
	drawscreen();
}
/*____________________________________________________________________*/

//Draws the screen
void GraphicsLoop(){
	cursor(FALSE);
	while(ingame){
		clearcanvas();
		drawborder();

		if(host){
			drawchar(ball.x, ball.y, 'O');
			drawchar(WIDTH-4, 0, sscore+'0');		
			drawchar(WIDTH-3, 0, ':');
			drawchar(WIDTH-2, 0, cscore+'0');		
		}else{
			drawchar(ball.x, HEIGHT-ball.y, 'O');
			drawchar(WIDTH-4, 0, cscore+'0');		
			drawchar(WIDTH-3, 0, ':');
			drawchar(WIDTH-2, 0, sscore+'0');
		}
	
		//User
		drawchar(user_x-1, HEIGHT-2, '-');
		drawchar(user_x, HEIGHT-2, '=');
		drawchar(user_x+1, HEIGHT-2, '-');

		//Opponent
		drawchar(opponent_x-1, 1, '-');
		drawchar(opponent_x, 1, '=');
		drawchar(opponent_x+1, 1, '-');
		drawscreen();
		Sleep(50);
	}
}

//Game Loop logic (server only) ball and its collisions
DWORD WINAPI GameLoop(void* data){
	int lx = 2; int ly =2;
	while(ingame){
		//Move ball
		if(ball.ox >= lx){
			if(ball.x + ball.dx < WIDTH-2 && ball.x + ball.dx > 1){
				ball.x += ball.dx;
			}
			else{ball.dx = ball.dx*-1;}
		}

		if(ball.oy >= ly){
			if(ball.y + ball.dy == 1 && (ball.x >= opponent_x-1 && ball.x <= opponent_x+1)){
				ball.dy = ball.dy *-1;
				if(ball.x == opponent_x-1 || ball.x == user_x-1){ball.dx = -1;}
				else if(ball.x == opponent_x+1 || ball.x == user_x+1){ball.dx = 1;}
				else if(ball.x == opponent_x || ball.x == user_x){ball.dx = 0;}

			}else if(ball.y + ball.dy == HEIGHT-2 && (ball.x >= user_x-1 && ball.x <= user_x+1)){
				ball.dy = ball.dy *-1;
				if(ball.x == opponent_x-1 || ball.x == user_x-1){ball.dx = -1;}
				else if(ball.x == opponent_x+1 || ball.x == user_x+1){ball.dx = 1;}
				else if(ball.x == opponent_x || ball.x == user_x){ball.dx = 0;}
			}else{
				ball.y += ball.dy;
			}

			//Points
			if(ball.y + ball.dy >= HEIGHT || ball.y + ball.dy <=0){
				if(ball.y > 20){
					cscore++;
				}else{
					sscore++;
				}
				ball.y = HEIGHT/2;
				ball.x = WIDTH/2;

				//check to see if game is over
				if(cscore >= 7 || sscore >= 7){
					//Game over
					Sleep(500);
					ingame = false;
					if(sscore > cscore){
						writescreen(centertext("WINNER"), HEIGHT/2, "WINNER");
					}else{
						writescreen(centertext("LOSER"), HEIGHT/2, "LOSER");
					}
					drawscreen();
					char ip = getch();
					clrscr();
				}
			}
			// else{ball.dy = ball.dy *-1;}
		}
		ball.ox++; ball.oy++;
		if(ball.ox > lx){ball.ox=0;}
		if(ball.oy > ly){ball.oy=0;}
		//Check collisions

		Sleep(50);
	}
}

//Checks if move is possible and updates
void valid_move(int d){
	if(d == -1){
		if((user_x - 1) >= 2){user_x--;}
	}else{
		if((user_x + 1) < WIDTH-2){user_x++;}
	}
}

//Controller Thread
DWORD WINAPI Controller(void* data){
	while(ingame){
		char ip = getch();
		if(ip == 'a' || ip =='A'){
			//Move Left Call function to validate input
			valid_move(-1);
		}else if(ip == 'd' || ip =='d'){
			//Move Right
			valid_move(1);
		}
		Sleep(30);
	};
}

//Client Sender
DWORD WINAPI Sender_Client(void* data){
	while(ingame){
		//Send position every 1/10th of second
		char packet[10];
		sprintf(packet, "%d", user_x);
		if(send(s , packet , 2 , 0) < 0){};
		Sleep(50);
	};
}

//Client Listener
DWORD WINAPI Listener_Client(void* data){
	while(ingame){
		if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR)
		{
			// puts("recv failed");
		}
		server_reply[recv_size] = '\0';
		opponent_x = ctoi(server_reply[0])*10 + ctoi(server_reply[1]);
		ball.x = ctoi(server_reply[2])*10 + ctoi(server_reply[3]);
		ball.y = ctoi(server_reply[4])*10 + ctoi(server_reply[5]);
		sscore = ctoi(server_reply[6]);
		cscore = ctoi(server_reply[7]);
		// printf("%d  %d\n", ball.y, ctoi(server_reply[4])*10);
		if(cscore >= 7 || sscore >= 7){
			//Game over
			Sleep(500);
			ingame = false;
			if(sscore > cscore){
				writescreen(centertext("LOSER"), HEIGHT/2, "LOSER");
			}else{
				writescreen(centertext("WINNER"), HEIGHT/2, "WINNER");
			}
			drawscreen();
			char ip = getch();
			clrscr();
		}
	};
}

//Client Sender
DWORD WINAPI Sender_Server(void* data){
	while(ingame){
		char packet[8];

		//users x [0,1]
		if(user_x >9){
			packet[0]= user_x % 10;
			packet[1]= user_x - ((user_x % 10)*10);
		}else{
			packet[0]='0'; char c=user_x; packet[1]=c;
		}

		//ball x [2,3]
		if(ball.x >9){
			packet[2]= ball.x % 10;
			packet[3]= ball.x - ((ball.x % 10)*10);
		}else{
			packet[2]='0'; char c=ball.x; packet[3]=c;
		}

		//ball y [4,5]
		if(ball.y >9){
			packet[4]= ball.y % 10;
			packet[5]= ball.y - ((ball.y % 10)*10);
		}else{
			packet[4]='0'; char c=ball.y; packet[5]=c;
		}

		//score [6,7] server/client
		packet[6] = sscore;
		packet[7] = cscore;

		if(send(new_socket , packet , 8 , 0) < 0){};
		Sleep(50);
	};
}

DWORD WINAPI gameloading(void* data){
	int tick = 0;
	cursor(FALSE);
	int ctr= centertext("Loading");
	
	while(loading){
		clearcanvas();
		drawborder();
		char message[20] = "Loading";
		if(tick == 0){
			char c[] = ".";
			strcat(message, c);
		}else if(tick == 1){
			char c[] = "..";
			strcat(message, c);
		}else{
			char c[] = "...";
			strcat(message, c);
		}
		writescreen(ctr, 20, message);
		tick++;
		if(tick >= 3){tick = 0;}

		drawscreen();
		Sleep(500);
	}
}
//Server Listener
DWORD WINAPI Listener_Server(void* data){
	while(ingame){
		if((recv_size = recv(new_socket , server_reply , 2000 , 0)) == SOCKET_ERROR)
		{
			// server_reply[recv_size] = '\0';
			// puts(server_reply);
		}
		server_reply[recv_size] = '\0';
		opponent_x = atoi(server_reply);
		
	};
}

//Game init vars
void gameinit(){
	user_x = WIDTH/2;
	opponent_x = WIDTH/2;
	ball.x = WIDTH/2;
	ball.y = HEIGHT/2;
	ball.dx = 1;
	ball.dy =1;
	ball.ox = 0;
	ball.oy = 0;
	sscore=0;
	cscore=0;
}

//Create server
int gamehost(){
	//Initialise winsock
	int c;
	host = true;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		screenmessage("An Error has Occured");
		Sleep(1000);
		clrscr();
		return 1;
	}

	//Create a socket
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET){}//failed to create socket

	//Socket should be created by now
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

	//Bind the socket
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		screenmessage("An Error has Occured");
		Sleep(1000);
		clrscr();
	}
	
	//Bind is done
	//Listen to incoming connections, int denotes time before timeout
	loading = true;
	HANDLE thread_gl = CreateThread(NULL, 0, gameloading, NULL, 0, NULL);
	listen(s , 1);
	c = sizeof(struct sockaddr_in);
	
	if( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		loading = false;
		Sleep(500);
		screenmessage("User Joined Game");
		Sleep(1000);

		//Game loop stuff here
		ingame = true;
		gameinit();
		//init threads
		HANDLE thread_listener = CreateThread(NULL, 0, Listener_Server, NULL, 0, NULL);
		HANDLE thread_sender = CreateThread(NULL, 0, Sender_Server, NULL, 0, NULL);
		HANDLE thread_controller = CreateThread(NULL, 0, Controller, NULL, 0, NULL);
		HANDLE thread_gameloop = CreateThread(NULL, 0, GameLoop, NULL, 0, NULL);
		//Game loop

		GraphicsLoop();
	}
	
	loading = false;
	if (new_socket == INVALID_SOCKET)
	{
		screenmessage("An Error has Occured");
		Sleep(1000);
		clrscr();
		return 1;
	}

	host = false;
	closesocket(s);
	WSACleanup();
	
	return 0;
}

//Join server
int gamejoin(){
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		screenmessage("An Error has Occured");
		Sleep(1000);
		clrscr();
		return 1;
	}
	
	//Create a socket
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET){}//Failed to create socket

	server.sin_addr.s_addr = inet_addr(ADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );

	//Connect to remote server
	loading = true;
	HANDLE thread_gl = CreateThread(NULL, 0, gameloading, NULL, 0, NULL);
	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		loading = false;
		Sleep(500);
		clrscr();
		screenmessage("Connection Error");
		Sleep(1000);
		clrscr();
		return 1;
	}
	loading = false;
	screenmessage("Game Joined");
	Sleep(1000);
	
	ingame = true;
	gameinit();
	//init all threads
	HANDLE thread_listener = CreateThread(NULL, 0, Listener_Client, NULL, 0, NULL);
	HANDLE thread_sender = CreateThread(NULL, 0, Sender_Client, NULL, 0, NULL);
	HANDLE thread_controller = CreateThread(NULL, 0, Controller, NULL, 0, NULL);

	GraphicsLoop();

	Sleep(1000);
	closesocket(s);
	WSACleanup();
	return 0;
}

//change port
void portchange()
{
	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Port"), 10, "Changing Port");
	writescreen(centertext("New Port: "), 20, "New Port: ");

	drawscreen();
	char prt[4];

	cursormove(centertext("New Port: ") + pointerlen("New Port: ") + 1, 20);
	cursor(TRUE);
	scanf("%5s", prt);
	sscanf(prt, "%d", &PORT);

	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Port"), 10, "Changing Port");
	writescreen(centertext("Port Changed"), 20, "Port Changed");
	writescreen(centertext("Press any key to continue"), 35, "Press any key to continue");
	cursor(FALSE);
	drawscreen();
	getch();
}

void addresschange()
{
	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Address"), 10, "Changing Address");
	writescreen(centertext("New Address:") / 2, 20, "New Address:");

	drawscreen();
	cursormove(centertext("New Address: ") / 2 + pointerlen("New Address: ") + 1, 20);
	cursor(TRUE);
	char address[10];
	scanf("%14s", address);
	sscanf(address, "%14s", ADDRESS);

	clearcanvas();
	drawborder();
	writescreen(centertext("Changing Address"), 10, "Changing Address");
	writescreen(centertext("Address Changed"), 20, "Address Changed");
	writescreen(centertext("Press any key to continue"), 35, "Press any key to continue");
	cursor(FALSE);
	drawscreen();
	getch();
}

//Client
void clientmenu()
{
	//client menu loop

	int option = 0;
	bool loop = false;

	while (!loop)
	{
		clearcanvas();
		drawborder();

		// writescreen(centertext("Find Game"), 10, "Find Game");

		//Cast port to string
		char prt[4];
		sprintf(prt, "%d", PORT);

		//concate port message
		char cport[13] = "Port: ";
		strcat(cport, prt);
		writescreen(centertext(cport), 10, cport);

		// concate address message
		char caddr[26] = "Address: ";
		strcat(caddr, ADDRESS);
		writescreen(centertext(caddr), 11, caddr);

		if (option == 0)
		{
			writescreen(centertext("1. Change Port"), 20, " -> 1. Change Port");
		}
		else
		{
			writescreen(centertext("1. Change Port"), 20, "1. Change Port");
		}

		if (option == 1)
		{
			writescreen(centertext("2. Change Address"), 25, " -> 2. Change Address");
		}
		else
		{
			writescreen(centertext("2. Change Address"), 25, "2. Change Address");
		}

		if (option == 2)
		{
			writescreen(centertext("3. Join Game"), 30, " -> 3. Join Game");
		}
		else
		{
			writescreen(centertext("3. Join Game"), 30, "3. Join Game");
		}

		if (option == 3)
		{
			writescreen(centertext("4. Back"), 35, " -> 4. Back");
		}
		else
		{
			writescreen(centertext("4. Back"), 35, "4. Back");
		}
		drawscreen();

		char ip = getch();

		if (ip == 's' || ip == 'S')
		{
			option++;
		}
		else if (ip == 'w' || ip == 'W')
		{
			option--;
		}

		if (option > 3)
		{
			option = 0;
		}
		else if (option < 0)
		{
			option = 3;
		}

		if (ip == 'f' || ip == 'F')
		{
			if (option == 0)
			{
				portchange();
			}
			else if (option == 1)
			{
				addresschange();
			}
			else if (option == 2)
			{
				gamejoin();
			}
			else if (option == 3)
			{
				loop = true;
			}
		};
	}
}

//Server
void servermenu()
{

	int option = 0;
	bool loop = false;
	while (!loop)
	{

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

		if (option == 0)
		{
			writescreen(centertext("1. Change Port"), 20, " -> 1. Change Port");
		}
		else
		{
			writescreen(centertext("1. Change Port"), 20, "1. Change Port");
		}

		if (option == 1)
		{
			writescreen(centertext("2. Create Game"), 25, " -> 2. Create Game");
		}
		else
		{
			writescreen(centertext("2. Create Game"), 25, "2. Create Game");
		}

		if (option == 2)
		{
			writescreen(centertext("3. Back"), 30, " -> 3. Back");
		}
		else
		{
			writescreen(centertext("3. Back"), 30, "3. Back");
		}
		drawscreen();

		char ip = getch();

		if (ip == 's' || ip == 'S')
		{
			option++;
		}
		else if (ip == 'w' || ip == 'W')
		{
			option--;
		}

		if (option > 2)
		{
			option = 0;
		}
		else if (option < 0)
		{
			option = 2;
		}

		if (ip == 'f' || ip == 'F')
		{
			if (option == 0)
			{
				portchange();
			}
			else if (option == 1)
			{
				//Create Game
				gamehost();
			}
			else if (option == 2)
			{
				loop = true;
			}
		};
	}
}

//Main Menu
void mainmenu(int option)
{
	//Set initial Game Screen
	clearcanvas();
	drawborder();
	writescreen(centertext("Pong, Multiplayer Game"), 10, "Pong, Multiplayer Game");

	if (option == 0)
	{
		writescreen(centertext("1. Create Game"), 20, " -> 1. Create Game");
	}
	else
	{
		writescreen(centertext("1. Create Game"), 20, "1. Create Game");
	}

	if (option == 1)
	{
		writescreen(centertext("2. Join Game"), 25, " -> 2. Join Game");
	}
	else
	{
		writescreen(centertext("2. Join Game"), 25, "2. Join Game");
	}

	if (option == 2)
	{
		writescreen(centertext("3. Exit"), 30, " -> 3. Exit");
	}
	else
	{
		writescreen(centertext("3. Exit"), 30, "3. Exit");
	}
	// writescreen((WIDTH-pointerlen("Pong, Multipleayer Game"))/2);
}

void init()
{
	//Set game board to empty
	clrscr();
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < HEIGHT; j++)
		{
			GC[i][j] = ' ';
		}
	}
}

int main(int argc, char *argv[])
{
	//Change Window Title
	SetConsoleTitle("Pong but better");
	cursor(FALSE);

	//run initialise program defaults/configs and set title screen
	init();

	int option = 0;

	bool valid = false;
	while (!valid)
	{
		mainmenu(option);
		drawscreen();
		char ip = getch();

		if (ip == 's' || ip == 'S')
		{
			option++;
		}
		else if (ip == 'w' || ip == 'W')
		{
			option--;
		}

		if (option > 2)
		{
			option = 0;
		}
		else if (option < 0)
		{
			option = 2;
		}

		if (ip == 'f' || ip == 'F')
		{
			if (option == 0)
			{
				servermenu();
			}
			else if (option == 1)
			{
				clientmenu();
			}
			else if (option == 2)
			{
				valid = true;
			}
		};
	}
	//Exited Game loop here
	cursor(TRUE);
}