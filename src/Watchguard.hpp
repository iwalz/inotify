/*
 * Watchgurad.hpp
 *
 *  Created on: 01.07.2012
 *      Author: ingo
 */

#ifndef WATCHGUARD
#define WATCHGUARD
#include <vector>
#include <string>
#include <queue>
#include <sys/inotify.h>

typedef void (*sighandler_t)(int);

class Watchguard {
	//sighandler_t sigkillhandler(int a);
	void handleEvent(const std::string & target);
	bool addDirectory(const std::string & directory);
	bool cleanup();
	std::vector<std::string> dirList;
	std::queue<inotify_event*> eventQueue;
	int fd;
	int wd;
	int process_events();
	int check_events();
	int read_events();
	void handle_event(inotify_event *event);
public:
	Watchguard();
	void run(std::string argv);
	static bool keeprunning;
};


#endif
