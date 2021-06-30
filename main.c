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

//OPPONENT STRUCT
int opponent_x;
int user_x;

struct _ball{
	int x;int y;int dx;int dy;
};

struct _ball ball;
int score;
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
int centertext(char c[])
{
	return (WIDTH - pointerlen(c)) / 2;
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
/*____________________________________________________________________*/

//Draws the screen
void GraphicsLoop(){
	cursor(FALSE);
	while(ingame){
		//Once every 10 seconds
		clearcanvas();
		drawborder();
		//User
		drawchar(user_x, HEIGHT-2, '=');
		//Opponent
		drawchar(opponent_x, 1, '=');
		drawchar(ball.x, ball.y, 'O');
		drawscreen();
		Sleep(100);
	}
}

//Game Loop logic (server only)
DWORD WINAPI GameLoop(void* data){

}

//Checks if move is possible and updates
void valid_move(int d){
	if(d == -1){
		if((user_x - 1) >= 1){user_x--;}
	}else{
		if((user_x + 1) < WIDTH){user_x++;}
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
		Sleep(25);
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
	};
}

//Client Sender
DWORD WINAPI Sender_Server(void* data){
	while(ingame){
		packet[7];
	};
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
	ball.y = WIDTH/2;
	ball.dx = 0;
	ball.dy =1;
	score=0;
}

//Create server
int gamehost(){
	//Initialise winsock
	int c;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		//Failed
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}

	//Create a socket
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	//Socket should be created by now
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

	//Bind the socket
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		//bind has failed, exit program
		printf("Bind failed with error code : %d" , WSAGetLastError());
		// exit(EXIT_FAILURE);
	}
	
	//Bind is done
	//Listen to incoming connections, int denotes time before timeout
	puts("Listening");
	listen(s , 1);
	c = sizeof(struct sockaddr_in);
	
	while( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		puts("Connection accepted");
		
		//Reply to the client
		//Game loop stuff goes in here
		// while(true){
		// 	char message[] = "Hello Client , I have received your connection. But I have to go now, bye\n";
		// 	send(new_socket , message , strlen(message) , 0);
		// 	Sleep(1000);
			
		// // }

		//Game loop stuff here
		ingame = true;
		gameinit();
		//init threads
		HANDLE thread_listener = CreateThread(NULL, 0, Listener_Server, NULL, 0, NULL);
		HANDLE thread_sender = CreateThread(NULL, 0, Sender_Server, NULL, 0, NULL);
		HANDLE thread_controller = CreateThread(NULL, 0, Controller, NULL, 0, NULL);
		//Game loop

		GraphicsLoop();
	}
	
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		// Socket was invalid, just exit
		return 1;
	}

	closesocket(s);
	WSACleanup();
	
	return 0;
}

//Join server
int gamejoin(){
	/*
		Makes a socket connection
	*/
	printf("\n");
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("\nFailed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
	printf("Initialised.\n");
	
	//Create a socket
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	printf("Socket created.\n");
	
	
	server.sin_addr.s_addr = inet_addr(ADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );

	//Connect to remote server
	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	
	puts("Connected");
	
	ingame = true;
	gameinit();
	//init all threads
	HANDLE thread_listener = CreateThread(NULL, 0, Listener_Client, NULL, 0, NULL);
	HANDLE thread_sender = CreateThread(NULL, 0, Sender_Client, NULL, 0, NULL);
	HANDLE thread_controller = CreateThread(NULL, 0, Controller, NULL, 0, NULL);

	GraphicsLoop();

	//call game loop

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