# Multiplayer Pong written in C
## Compiling the project
To compile the project, run > gcc main.c -o pong.exe -lws2_32 
In windows command prompt or powershell. This game will only 
run and compile on windows machines.

## Playing the game
###### Creating/Joining a game
There must be a full lobby (Both host and client) before the game 
will run. The Client must enter the address and port of the host
in order to connect to the game. The Host must only supply an open
port to the program. The default port is 8080.

###### Controls (**Menu**)
Use W/S to scale Up/Down the menu. Press F to select the option. When
typing in an address and/or port, press Enter to submit.

###### Controls (**In Game**)
Use A/D to move the paddle Left/Right respectively

###### Gameplay
A point is scored when an opponent fails to bounce back the ball and it
hits the back wall. First person to 7 points wins the game.