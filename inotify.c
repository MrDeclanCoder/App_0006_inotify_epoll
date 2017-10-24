﻿#include <sys/inotify.h>
#include <string.h>
#include <errno.h>


/*
*参考: frameworks\native\services\inputflinger\EventHub.cpp
*/


int read_process_inotify_fd(int fd)
{
	int res;	
	char devname[PATH_MAX];    
	char *filename;	  
	char event_buf[512];	  
	int event_size;	 
	int event_pos = 0;    
	struct inotify_event *event;


	/* read */
	res = read(fd, event_buf,sizeof(event_buf));
	if(res< (int)sizeof(*event))
	{
		if(errno == EINTR)
		{
			return 0;
		}
		else 
		{
			printf("could not get event, %s\n",strerror(errno));	
			return -1;
		}
		 
	}

	/*
	*process
	*读到的
	*
	*/

	
	
	
}

int main(int argc, char* argv)
{
	int mInotifyFd;
	int result;

	if(argc != 2)
	{
		printf("Usage : %s <dir>\n",argv[0]);
		return -1;
	}

	
	/* inotify create */
	mInotifyFd = inotify_init();

	
	/* add watch */
	result= inotify_add_watch(mInotifyFd,argv[1],IN_DELETE | IN_CREATE);
	

	/* read */
	while(1)
	{
		read_process_inotify_fd(mInotifyFd);
	}

	return 0;
}










