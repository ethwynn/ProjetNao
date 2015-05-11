#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ctime>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#include <string.h>

#define MAX_UDP_MESSAGE 1024

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


#include <alcommon/alproxy.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>

struct sigaction action;
bool inter = true;

//Init sigint
void hand(int sig){
	if (sig == SIGINT){
		printf("SIGINT\n");
		inter = false;
	}
	else{
		printf("bizarre !\n");
	}
}

int initialisationSocketUDP(char *service){

	struct addrinfo precisions,*resultat;
	int statut;
	int s;

	/* Construction de la structure adresse */
	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_DGRAM;
	precisions.ai_flags=AI_PASSIVE;
	statut=getaddrinfo(NULL,service,&precisions,&resultat);
	if(statut<0){ perror("initialisationSocketUDP.getaddrinfo"); exit(EXIT_FAILURE); }

	/* Creation d'une socket */
	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("initialisationSocketUDP.socket"); exit(EXIT_FAILURE); }

	/* Options utiles */
	int vrai=1;
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
		perror("initialisationServeurUDPgenerique.setsockopt (REUSEADDR)");
		exit(-1);
	}

	/* Specification de l'adresse de la socket */
	statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
	if(statut<0) {perror("initialisationServeurUDP.bind"); exit(-1);}

	/* Liberation de la structure d'informations */
	freeaddrinfo(resultat);

	return s;

}

int boucleServeurUDP(int s,AL::ALMotionProxy motion)
{

	struct sockaddr_storage adresse;
	socklen_t taille=sizeof(adresse);
	int nboctets;
	float data[4];
	int i,sep;
	printf("serveur\n");
	for(i=0;i<4;i++)
	{	
		nboctets=recvfrom(s,&data[i],MAX_UDP_MESSAGE,0,(struct sockaddr *)&adresse,&taille);
		if(nboctets<0) return -1;

		printf("%f\n",data[i]);
	}
	sep=(int)data[3];
	if (sep == 4000 && (data[0] != 0 | data[1] != 0))
	{
		printf("move\n");
		motion.post.moveTo(data[0]/2,data[1]/2,data[2]);
	}
	return 0;

}

int main(int argc, char* argv[])
{
  // We will try to connect our broker to a running NAOqi
  int pport = 9559;
  std::string pip = "127.0.0.1";

  // command line parse option
  // check the number of arguments
  if (argc != 1 && argc != 3 && argc != 5)
  {
    std::cerr << "Wrong number of arguments!" << std::endl;
    std::cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << std::endl;
    exit(2);
  }

  // if there is only one argument it should be IP or PORT
  if (argc == 3)
  {
    if (std::string(argv[1]) == "--pip")
      pip = argv[2];
    else if (std::string(argv[1]) == "--pport")
      pport = atoi(argv[2]);
    else
    {
      std::cerr << "Wrong number of arguments!" << std::endl;
      std::cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << std::endl;
      exit(2);
    }
  }

  // Sepcified IP or PORT for the connection
  if (argc == 5)
  {
    if (std::string(argv[1]) == "--pport"
        && std::string(argv[3]) == "--pip")
    {
      pport = atoi(argv[2]);
      pip = argv[4];
    }
    else if (std::string(argv[3]) == "--pport"
             && std::string(argv[1]) == "--pip")
    {
      pport = atoi(argv[4]);
      pip = argv[2];
    }
    else
    {
      std::cerr << "Wrong number of arguments!" << std::endl;
      std::cerr << "Usage: mymodule [--pip robot_ip] [--pport port]" << std::endl;
      exit(2);
    }
  }

	char buffer[1024];
 	action.sa_handler = hand;
  	sigaction(SIGINT, &action, NULL);

	char *service="5000";
	int s=initialisationSocketUDP(service);

  /** Create a ALMotionProxy to call the methods to move NAO's head.
  * Arguments for the constructor are:
  * - IP adress of the robot
  * - port on which NAOqi is listening, by default 9559
  */
  AL::ALMotionProxy motion(pip, 9559);
  AL::ALRobotPostureProxy robotPosture(pip,9559);

  try {

    //Make the NAO stand
    robotPosture.goToPosture("Stand", 1.0f);
	printf("debout\n");
		while (inter)
		{
			boucleServeurUDP(s,motion);
    		}


	//Make the NAO sit
	robotPosture.goToPosture("Sit", 1.0f);
     } 
	 
	catch (const AL::ALError& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		//Make the NAO sit
		robotPosture.goToPosture("Sit", 1.0f);

		exit(1);
	}
	exit(0);
}
