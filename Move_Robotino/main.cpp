//  Copyright (C) 2004-2008, Robotics Equipment Corporation GmbH

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

#include "rec/robotino/api2/all.h"


using namespace rec::robotino::api2;

bool _run = true;

void messageUDP(char *hote,char *service,float message,int taille){

	struct addrinfo precisions,*resultat;
	int statut;
	int s;

	/* Creation de l'adresse de socket */
	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_DGRAM;
	statut=getaddrinfo(hote,service,&precisions,&resultat);
	if(statut<0){ perror("messageUDPgenerique.getaddrinfo"); exit(EXIT_FAILURE); }

	/* Creation d'une socket */
	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("messageUDPgenerique.socket"); exit(EXIT_FAILURE); }

	/* Option sur la socket */
	int vrai=1;
	if(setsockopt(s,SOL_SOCKET,SO_BROADCAST,&vrai,sizeof(vrai))<0){
		perror("initialisationServeurUDPgenerique.setsockopt (BROADCAST)");
		exit(-1);
	}

	/* Envoi du message */
	int nboctets=sendto(s,&message,taille,0,resultat->ai_addr,resultat->ai_addrlen);
	if(nboctets<0){ perror("messageUDPgenerique.sento"); exit(EXIT_FAILURE); }

	/* Liberation de la structure d'informations */
	freeaddrinfo(resultat);

	/* Fermeture de la socket d'envoi */
	close(s);
}

void sigint_handler( int signum )
{
	_run = false;
}

class MyCom : public Com
{
public:
	MyCom()
	{
	}
	void errorEvent( const char* errorString )
	{
		std::cerr << "Error: " << errorString << std::endl;
	}
	void connectedEvent()
	{
		std::cout << "Connected." << std::endl;
	}
	void connectionClosedEvent()
	{
		std::cout << "Connection closed." << std::endl;
	}
	void logEvent( const char* message, int level )
	{
		std::cout << message << std::endl;
	}
	void pingEvent( float timeMs )
	{
		std::cout << "Ping: " << timeMs << "ms" << std::endl;
	}
};

class MyBumper : public Bumper
{
public:
	MyBumper()
		: bumped( false )
	{
	}
	void bumperEvent( bool hasContact )
	{
		bumped |= hasContact;
		std::cout << "Bumper has " << ( hasContact ? "contact" : "no contact") << std::endl;
	}
	bool bumped;
};


MyBumper bumper;
MyCom com;
OmniDrive omniDrive;
Odometry odometry;

void init( const std::string& hostname )
{
	// Connect
	std::cout << "Connecting...";
	com.setAddress( hostname.c_str() );

	com.connectToServer( true );

	if( false == com.isConnected() )
	{
		std::cout << std::endl << "Could not connect to " << com.address() << std::endl;
		rec::robotino::api2::shutdown();
		exit( 1 );
	}
	else
	{
		std::cout << "success" << std::endl;
		odometry.set(0,0,0,false);
	}
}

void move(char * hote)
{
	float mouvement[2] = {0.0f, 0.0f};
	double x=0,y=0,phi=0,sep=0;
	unsigned int *seq;
	char *service="5000";
	sep = 4000;
	int t0=com.msecsElapsed(),t=0,dir=0;
	
	while (dir != 1 && dir != 2 && dir != 3 && dir != 4 )
	{	
		std::cout << "which direction :" << std::endl;
		std::cout << "1 for front" << std::endl;
		std::cout << "2 for back" << std::endl;
		std::cout << "3 for left" << std::endl;
		std::cout << "and 4 for right" << std::endl;	
		std::cin >> dir;
	}
	
	if (dir == 1)
	{
		mouvement[0]=0.2f;
		mouvement[1]=0.0f;
	}
	else if (dir == 2)
	{
		mouvement[0]=-0.2f;
		mouvement[1]=0.0f;
	}
	else if (dir == 3)
	{
		mouvement[0]=0.0f;
		mouvement[1]=0.2f;
	}
	else if (dir == 4)
	{
		mouvement[0]=0.0f;
		mouvement[1]=-0.2f;
	}

	while( com.isConnected() && false == bumper.value() && _run && t<3000)
	{


		omniDrive.setVelocity( mouvement[0], mouvement[1], 0 );


		com.processEvents();
		
		odometry.readings (&x,&y,&phi,seq);
		messageUDP(hote,service,(float)x,sizeof(x));
		messageUDP(hote,service,(float)y,sizeof(y));
		messageUDP(hote,service,(float)phi,sizeof(phi));
		messageUDP(hote,service,(float)sep,sizeof(sep));
		odometry.set(0,0,0,false);
		t=com.msecsElapsed()-t0;
	}
}

void destroy()
{
	com.disconnectFromServer();
}

int main( int argc, char **argv )
{
	std::string hostname = "172.26.1.1";
	if( argc > 1 )
	{
		hostname = argv[1];

	}

	struct sigaction act;
	memset( &act, 0, sizeof( act ) );
	act.sa_handler = sigint_handler;
	sigaction( SIGINT, &act, NULL );
	char *hote = argv[2];



	try
	{
		init(hostname);
		while( com.isConnected() && false == bumper.value() && _run)
			move(hote);
		destroy();
	}
	catch( const rec::robotino::api2::RobotinoException& e )
	{
		std::cerr << "Com Error: " << e.what() << std::endl;
	}
	catch( const std::exception& e )
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch( ... )
	{
		std::cerr << "Unknow Error" << std::endl;
	}

	rec::robotino::api2::shutdown();

}
