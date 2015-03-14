#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <fstream>


#include <alcommon/alproxy.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>


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
  const AL::ALValue jointName = "HeadYaw";
  std::ofstream monFlux("/home/flavien/Documents/time.txt", std::ios::app);
  char c;
  double diff;

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
    motion.stiffnessInterpolation(jointName, stiffness, time);





    if(monFlux)  //On teste si tout est OK
    {
        monFlux << "Ouverture" << std::endl;
        clock_t begin = clock();

        while (c != 's'){
            std::cout << std::endl << "Entrer une instruction :" << std::endl << "a pour avancer" << std::endl << "r pour reculer" << std::endl << "s pour quitter" << std::endl << std::endl;
            std::cin >> c;

            if(c == 'a'){

                clock_t begin = clock();
                std::cout << "Avancer" << std::endl;
		// Example showing the moveTo command
		// as length of path is less than 0.4m
		// the path will be an SE3 interpolation
		// The units for this command are meters and radians
		float x  = 0.2f;
		float y  = 0.2f;
		// pi/2 anti-clockwise (90 degrees)
		float theta = 0.0f;
		motion.moveTo(x, y, theta);
                std::cout << "Deplacement termine" << std::endl;

            }

            else if(c == 'r'){

                clock_t begin = clock();
                std::cout << "Reculer" << std::endl;
		// Example showing the moveTo command
		// as length of path is less than 0.4m
		// the path will be an SE3 interpolation
		// The units for this command are meters and radians
		float x  = -0.2f;
		float y  = -0.2f;
		// pi/2 anti-clockwise (90 degrees)
		float theta = 0.0f;
		motion.moveTo(x, y, theta);
                std::cout << "Deplacement termine" << std::endl;

            }
			else{

	    	}
            clock_t end = clock();
            diff = double(end - begin) / CLOCKS_PER_SEC;
            if(c == 'a' || c == 'r') monFlux << "temps ecoule :" << diff << std::endl;
        }
        monFlux << "Fermeture" << std::endl << std::endl;

    }
    else
    {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier." << std::endl;
    }


    /** Remove the stiffness on the head. */
    stiffness = 0.0f;
    time = 1.0f;
    motion.stiffnessInterpolation(jointName, stiffness, time);

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
