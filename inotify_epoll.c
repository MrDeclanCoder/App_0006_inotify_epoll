#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define MAX_FILES 1000
#define DATA_MAX_LEN 500

static char *base_dir;
static char *epoll_files[MAX_FILES];

#if 0
typedef union epoll_data{
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
}epoll_data_t;

struct epoll_event {
	__uint32_t   events;		/* Epoll events */
	epoll_data_t data;		/* User data variable */
};
#endif

int add_to_epoll(int fd, int epollFd)
{
	int result;
	struct epoll_event eventItem;
	memset(&eventItem, sizeof(eventItem));
	eventItem.events = EPOLLIN;
	eventItem.data.fd = fd;
	result = eopll_ctl(epollFd, EPOLL_CTL_ADD, fd, &eventItem);
	return result;
}

void rm_to_epoll(int fd, int epollFd)
{
	eopll_ctl(epollFd, EPOLL_CTL_DEL, fd, &eventItem);
}

int get_epoll_fd_for_name(char *name)
{
	int i;
	char name_to_find[DATA_MAX_LEN];
	sprintf(name_to_find,"%s%s", base_dir,name);
	for(i=0;i<MAX_FILES;i++)
	{
		if(!epoll_files[i])
		{
			continue;
		}
		if(!strcmp(epoll_files[i],name_to_find))
		{
			return i;
		}
		return -1;
	}
}

/*
*参考: frameworks\native\services\inputflinger\EventHub.cpp
*/
/*Usage: inotify <dir> */
int read_process_inotify_fd(int fd, int mEpollFd)
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
			return 0; 
		else 
		{
			printf("could not get event, %s\n",strerror(errno));	
			return -1;
		}		 
	}
	/*
	*process
	*读到的数据是一个或者多个inotify_event
	*他们的长度不一样, 逐个处理
	*/
	while(res >= (int)sizeof(*event))
	{
		event = (struct inotify_event*)(event_buf + event_pos);
		if(event->len)
		{
			if(event->mask & IN_CREATE)
			{
				printf("create file: %s\n",event->name);
				char *name = malloc(512);
				sprintf(name, "%s%s", base_dir,event->name);
				int tmpFd = open(name, O_RDWR);
				printf("add to epoll: %s\n", name);
				add_to_epoll(tmpFd,mEpollFd);
				epoll_files[tmpFd] = name;
			} else {
				printf("delete file: %s\n",event->name);
				int tmpFd = get_epoll_fd_for_name(event->name);
				if(tmpFd > 0)
				{
					printf("remove from epoll: %s%s\n",base_dir,event->name);
					rm_from_epoll(tmpfd,mEpollFd);
					free(epoll_files[tmpFd]);
				}
			}
		}

		event_size = sizeof(*event)+event->len;
		res -= event_size;
		event_pos += event_size;
	}
	return 0;
}



int main(int argc, char** argv)
{
	int mInotifyFd;
	int mEpollFd;
	int i;
	char buf[DATA_MAX_LEN];
	// Maximum number of signalled FDs to handle at a time.
	static const int EPOLL_MAX_EVENTS = 16;
	// The array of pending epoll events and the index of the next event to be handled.
	struct epoll_event mPendingEventItems[EPOLL_MAX_EVENTS];
	
	if(argc < 2)
	{
		printf("Usage: %s <tmp>\n",argv[0]);
		return -1;
	}

	base_dir = argv[1];
	
	/* epoll create */
	mEpollFd = epoll_create(8);
	
	/* inotify create */
	mInotifyFd = inotify_init();

	/* add watch*/
	result = inotify_add_watch(mInotifyFd,base_dir,IN_DELETE | IN_CREATE);
	add_to_epoll(mInotifyFd,mEpollFd);
 
	/* epoll wait */
	while(1)
	{
		int pollResult = epoll_wait(mEpollFd,mPendingEventItems,EPOLL_MAX_EVENTS, -1);
		for(i=0;i<pollResult;i++)
		{
			if(mPendingEventItems[i].data.fd == mInotifyFd)
			{
				read_process_inotify_fd(mInotifyFd,mEpollFd);
			} else {
				int len = read(mPendingEventItems[i].data.fd, buf, DATA_MAX_LEN);
				buf[len] = "\0";
				printf("get data: %s\n",buf);
			} 	
		}
	}

	return 0;
}










