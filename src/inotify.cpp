#include <iostream>
#include <sstream>
#include <signal.h>
#include "EventManager.hpp"
#include "Watchguard.hpp"

void sigkillhandler(int signum) {
	Watchguard::keeprunning = false;

	std::cout << "Killed" << std::endl;
}

int main(int argc, char **argv)
{
	signal(SIGINT, sigkillhandler);

	Watchguard guard;
	/*std::stringstream ss;
	std::string argument;

	ss << argv;
	ss >> argument;
*/
	guard.run(std::string(argv[1]));

	return 0;
}


