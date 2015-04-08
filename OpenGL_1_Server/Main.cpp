#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Network.hpp>
#include <iostream>
#include <cstdlib>

#define PLAYERS 8

struct Player {
	char name[1024];
	unsigned short port;
	sf::IpAddress ip;
};

int main(){
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
			// when a user connects
			case '0':{
				// insert the client into the players struct
				int i = 0;
				for(i = 0; i < PLAYERS; i++){
					// find the first empty player slot
					if(players[i].ip == sf::IpAddress::None){
						sprintf(players[i].name, buff);
						players[i].port = clientPort;
						players[i].ip   = client;

						// display client's information
						printf("+ %s (%s on port %i) has connected (with ID %i).\n",
							   players[i].name,
							   players[i].ip.toString().c_str(),
							   players[i].port,
							   i
						);

						break; // break out, don't want to fill every player slot with one player's information
					}
				}

				// let the client know they successfully connected and tell them their ID
				sprintf(sendBuffer, "0%i", i);
				socket.send(sendBuffer, sizeof(sendBuffer), client, clientPort);
			}break;
			// when a user disconnects
			case '-':{
				int id = atoi(buff);

				printf("- %s has disconnected.\n", players[id].name);

				// clear the client's information
				sprintf(players[id].name, "");
				players[id].port = 0;
				players[id].ip   = sf::IpAddress::None;
			}break;
		}
	}

	system("PAUSE");
	return EXIT_SUCCESS;
}