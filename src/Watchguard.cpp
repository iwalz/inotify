#include "Watchguard.hpp"
#include <iostream>
#include <ctype.h>
#include <string>
#include <algorithm>
#include <queue>
#include <sys/inotify.h>
#include <signal.h>
#include "boost/filesystem.hpp"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUFF_SIZE 		( 1024 * ( EVENT_SIZE + 16 ) )

bool Watchguard::keeprunning = true;

Watchguard::Watchguard() {
    this->fd = inotify_init();
    this->dirList = new std::map<int, std::string>();
    this->eventQueue = new std::queue<inotify_event*>();

    if(this->fd < 0) {
            std::cerr << "inotify init" << std::endl;
    }
}

void Watchguard::run(std::string argv) {
	namespace bfs = boost::filesystem;

	this->addDirectory(argv);

	for(bfs::recursive_directory_iterator end, dir(argv);
		dir != end; ++dir )
	{

		bfs::file_status stat = (*dir).status();
		if(bfs::is_directory(stat))
		{
			this->addDirectory((*dir).path().string());
		}
    }

	//while(Watchguard::keeprunning) {
    	//this->handleEvent(argv);
	//}

	this->process_events();

   	this->cleanup();

    close( this->fd );

}

int Watchguard::process_events() {
	//while loop here
	while(this->keeprunning) {
		int r;
		r = this->read_events();
		if(r < 0) {
			break;
		} else {
			this->handle_event(this->eventQueue->front());
		}
	}
	return 0;
}
int Watchguard::check_events() {
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(this->fd, &rfds);
	return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}
int Watchguard::read_events() {
	int count = 0;
	char buffer[16384];
	size_t buffer_i, r;
	struct inotify_event *pevent;
	size_t event_size;

	r = read(this->fd, buffer, 16384);
	if(r <= 0) {
		return r;
	}

	buffer_i = 0;
	while(buffer_i < r) {
		pevent = (struct inotify_event *) &buffer[buffer_i];
		event_size =  offsetof (struct inotify_event, name) + pevent->len;

		if(pevent->wd) {
			this->eventQueue->push(pevent);
		}

		buffer_i += event_size;
		count++;
	}
	//Read stuff and put in queue
	return count;
}
void Watchguard::handle_event(inotify_event *event) {
	//Eventmanager stuff here
	std::string action = "";
    std::string target = event->name;

    int check = target.c_str()[0];

    if(!isalnum(check)) {
    	return;
    }

    std::map<int, std::string>::iterator it = this->dirList->find(event->wd);

    action.append(it->second);
    int size = action.size();
    if(action.substr(size-1,1) != "/")
    {
    	action.append("/");
    }

    if (event->len)
       action.append(event->name);
    //else
       //action.append(target);

    if (event->mask & IN_ACCESS)
       action.append(" was read");
    if (event->mask & IN_ATTRIB)
       action.append(" Metadata changed");
    if (event->mask & IN_CLOSE_WRITE)
       action.append(" opened for writing was closed");
    if (event->mask & IN_CLOSE_NOWRITE)
       action.append(" not opened for writing was closed");
    if (event->mask & IN_CREATE)
       action.append(" created in watched directory");
    if (event->mask & IN_DELETE)
       action.append(" deleted from watched directory");
    if (event->mask & IN_DELETE_SELF)
       action.append("Watched file/directory was itself deleted");
    if (event->mask & IN_MODIFY)
       action.append(" was modified");
    if (event->mask & IN_MOVE_SELF)
       action.append("Watched file/directory was itself moved");
    if (event->mask & IN_MOVED_FROM)
       action.append(" moved out of watched directory");
    if (event->mask & IN_MOVED_TO)
       action.append(" moved into watched directory");
    if (event->mask & IN_OPEN)
       action.append(" was opened");

    std::cout << action << std::endl;

}

bool Watchguard::addDirectory(const std::string & directory) {
		//std::cout << directory << std::endl;

		int wd = inotify_add_watch( this->fd, directory.c_str(), IN_ALL_EVENTS );
		if(wd) {
			//std::cout << this->wd << ": " << directory << std::endl;
			this->dirList->insert(std::pair<int, std::string>(wd, directory));
			//this->dirList.insert(std::pair<int, std::string>(wdfoo, directory));
		}


	return true;
}

bool Watchguard::cleanup() {
	for(
			std::map<int, std::string>::const_iterator iter = this->dirList->begin();
	    		iter != this->dirList->end();
	    		++iter
	    	)
	    {
	    	inotify_rm_watch( this->fd, iter->first);
	    }
	delete this->dirList;

	return true;
}

void Watchguard::handleEvent(const std::string & target) {
	ssize_t len, i = 0;

	   std::string action = "";
	   char buff[BUFF_SIZE] = {0};


	   len = read (this->fd, buff, BUFF_SIZE);

	   while (i < len) {
	      struct inotify_event *pevent = (struct inotify_event *)&buff[i];

	      std::string action = "";

	      if (pevent->len)
	         action.append(pevent->name);
	      else
	         action.append(target);

	      if (pevent->mask & IN_ACCESS)
	         action.append(" was read");
	      if (pevent->mask & IN_ATTRIB)
	         action.append(" Metadata changed");
	      if (pevent->mask & IN_CLOSE_WRITE)
	         action.append(" opened for writing was closed");
	      if (pevent->mask & IN_CLOSE_NOWRITE)
	         action.append(" not opened for writing was closed");
	      if (pevent->mask & IN_CREATE)
	         action.append(" created in watched directory");
	      if (pevent->mask & IN_DELETE)
	         action.append(" deleted from watched directory");
	      if (pevent->mask & IN_DELETE_SELF)
	         action.append("Watched file/directory was itself deleted");
	      if (pevent->mask & IN_MODIFY)
	         action.append(" was modified");
	      if (pevent->mask & IN_MOVE_SELF)
	         action.append("Watched file/directory was itself moved");
	      if (pevent->mask & IN_MOVED_FROM)
	         action.append(" moved out of watched directory");
	      if (pevent->mask & IN_MOVED_TO)
	         action.append(" moved into watched directory");
	      if (pevent->mask & IN_OPEN)
	         action.append(" was opened");

	      std::cout << target << " - " << action << std::endl;

	      i += sizeof(struct inotify_event) + pevent->len;

	   }

}

