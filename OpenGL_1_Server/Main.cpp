#define _CRT_SECURE_NO_WARNINGS

#define BLACK		  0
#define BLUE		  1
#define GREEN		  2
#define CYAN		  3
#define RED			  4
#define MAGENTA		  5
#define BROWN		  6
#define LIGHT_GRAY	  7
#define DARK_GRAY	  8
#define LIGHT_BLUE	  9
#define LIGHT_GREEN	  10
#define LIGHT_CYAN	  11
#define LIGHT_RED	  12
#define LIGHT_MAGENTA 13
#define YELLOW		  14
#define WHITE		  15

#include <SFML/Network.hpp>
#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>

#define PLAYERS 8

struct Player {
	char name[1024];
	unsigned short port;
	sf::IpAddress ip;
	float x, y, z;
};

int main(){
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	//for(int colour = 0x00; colour <= 0x10; colour ++)
	//	SetConsoleTextAttribute(h, colour), printf("Color: %i\n", colour);

	const unsigned short port = 50001;

	printf("Setting up server on port %i...\n\n", port);
	printf("||| Server Properties\n");
	printf(" |  Maximum Players: %i\n", PLAYERS);
	printf("\n");
	// printf("\n");

	sf::UdpSocket socket;
	socket.bind(port);

	printf("Setup done. Receiving...\n");

	char buffer[1024], sendBuffer[1024];
	std::size_t received;

	// create an empty struct of players
	Player players[PLAYERS];
	for(int i = 0; i < PLAYERS; i++){
		sprintf(players[i].name, "");
		players[i].port = 0;
		players[i].ip   = sf::IpAddress::None;
		players[i].x    = 0.f;
		players[i].y    = 0.f;
		players[i].z    = 0.f;
	}

	while(true){
		sf::IpAddress client;
		unsigned short clientPort;
		socket.receive(buffer, sizeof(buffer), received, client, clientPort);

		// remove the indicator of the buffer to get the actual information
		char buff[1024];
		for(int i = 0; i < sizeof(buffer); i++)
			buff[i] = buffer[i + 1];

		// handle messages
		switch(buffer[0]){
			// when a player connects
			case '+':{
				// insert the player into the players struct
				int i = 0;
				for(i = 0; i < PLAYERS; i++){
					// find the first empty player slot
					if(players[i].ip == sf::IpAddress::None){
						sprintf(players[i].name, buff);
						players[i].port = clientPort;
						players[i].ip   = client;

						// display client's information
						SetConsoleTextAttribute(h, LIGHT_GREEN),
						printf("%s (%s on port %i) has connected (with ID %i).\n",
							   players[i].name,
							   players[i].ip.toString().c_str(),
							   players[i].port,
							   i
						);

						// let the player know they successfully connected and tell them their ID
						sprintf(sendBuffer, "y%i", i);
						socket.send(sendBuffer, sizeof(sendBuffer), client, clientPort);

						break;
						break; // break out, don't want to fill every player slot with one player's information
					}
				}

				for(int j = 0; j < PLAYERS; j++){
					if(j != i){
						if(players[j].ip != sf::IpAddress::None){
							// inform the other players connected that a new player has connected
							sprintf(sendBuffer, "a%i,%s", i, players[i].name);
							socket.send(sendBuffer, sizeof(sendBuffer), players[j].ip, players[j].port);

							// inform the player of other connected players
							sprintf(sendBuffer, "A%i,%s", j, players[j].name);
							socket.send(sendBuffer, sizeof(sendBuffer), client, clientPort);
						}
					}
				}
			}break;
			// when a player disconnects
			case '-':{
				SetConsoleTextAttribute(h, LIGHT_RED),
				printf("%s has disconnected.\n", players[atoi(buff)].name);
				
				// inform other players a user disconnected
				for(int i = 0; i < PLAYERS; i++){
					if(i != atoi(buff)){
						sprintf(sendBuffer, "r%i", i);
						socket.send(sendBuffer, sizeof(sendBuffer), client, clientPort);
					}
				}

				// clear the player's information
				sprintf(players[atoi(buff)].name, "");
				players[atoi(buff)].port = 0;
				players[atoi(buff)].ip   = sf::IpAddress::None;
			}break;
			// when a player is moving
			case 'm':{
				int id; float x, y, z;
				sscanf(buff, "%i,%f,%f,%f", &id, &x, &y, &z);
				players[id].x = x; players[id].y = y; players[id].z = z;

				printf("move\n");

				sprintf(sendBuffer, "m%i,%f,%f,%f", id, x, y, z);

				for(int j = 0; j < PLAYERS; j++)
					if(j != id)
						if(players[j].ip != sf::IpAddress::None)
							socket.send(sendBuffer, sizeof(sendBuffer), players[j].ip, players[j].port);
			}break;

			// when the packet received from the player is unknown
			default:{
				printf("UNKNOWN PACKET RECEIVED - SIGNAL %c, INFO:\n-> %s\n", buffer[0], buff);
			}break;
		}

		SetConsoleTextAttribute(h, WHITE);
	}

	system("PAUSE");
	return EXIT_SUCCESS;
}