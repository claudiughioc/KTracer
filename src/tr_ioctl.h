#ifndef __TR_IOCTL_H__
#define __TR_IOCTL_H__		1

#define IOCTL_BUFF	12

#define TRACER_ADD_PROCESS	_IOC(_IOC_NONE, 'k', 1, IOCTL_BUFF)
#define TRACER_REMOVE_PROCESS	_IOC(_IOC_NONE, 'k', 2, IOCTL_BUFF)

#endif
