//Program working well using the difference between real angle and gyro and to adjust the position
//Precision can be improved by modifying the threshhold after which we do a mouvement.

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include "gyro/gyro.h"


#include <alcommon/alproxy.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>

#define PI  3.141592653

struct sigaction action;
bool inter = true;


void hand(int sig){
	if (sig == SIGINT){
		printf("SIGINT\n");
		inter = false;
	}
	else{
		printf("bizarre !\n");
	}
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

  /** The name of the joint to be moved. */
  const AL::ALValue names = "HeadYaw";
  int fd;
  double angle;
  //std::ofstream monFlux("/home/flavien/Documents/presision.txt", std::ios::app); //Ouverture d'un fichier pour noter la précision
  unsigned char buf[8] = {0};
  action.sa_handler = hand;
  sigaction(SIGINT, &action, NULL);

  init_serial(&fd); //initialization of serial port

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


    /** Make sure the head is stiff to be able to move it.
    * To do so, make the stiffness go to the maximum in one second.
    */
    /** Target stiffness. */
    AL::ALValue stiffness = 1.0f;
    /** Time (in seconds) to reach the target. */
    AL::ALValue time = 1.0f;
    /** Call the stiffness interpolation method. */
    motion.stiffnessInterpolation("Head", stiffness, time);

    double newAngle=0.0,a0=0,rangle;
	while(a0 == 0){
		getData(buf,fd);
		a0 = getAngle(buf);
	}

	while(inter){
		
		getData(buf,fd);
		angle = getAngle(buf);
		if (angle != 0){
			rangle = angle -a0;
		}

		if (rangle > 180){
			rangle = rangle - 360;
		}
		else if (rangle < -180){
			rangle = rangle + 360;
		}
		rangle = (rangle*PI)/180;
		std::vector<float> sensorAngle = motion.getAngles(names, true);


	    /** Set the target angle list, in radians. */
		double changes = rangle - sensorAngle[0];
		//std::cout << "sensorAngle: " << sensorAngle[0] << std::endl;
		//std::cout << "rangle: " << rangle << std::endl;
		std::cout << "changes: " << changes << std::endl;
		/** Set the speed */
	  	float fractionMaxSpeed = 0.10f;
		/** Call the change angles method. The joint will reach the
		* desired angles with the desired speed.
		*/
		if(changes > 0.07 || changes < -0.07){
			motion.changeAngles(names, changes, fractionMaxSpeed);
		}
		else{
			//std::cout << "Hors boucle" << std::endl;
			//monFlux << "precision: " << (changes*180)/PI;
			//monFlux << std::endl;
		}

			
	}

    /** Remove the stiffness on the head. */
    stiffness = 0.0f;
    time = 1.0f;
    motion.stiffnessInterpolation("Head", stiffness, time);

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
