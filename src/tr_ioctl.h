#ifndef __TR_IOCTL_H__
#define __TR_IOCTL_H__		1

#define TRACER_ADD_PROCESS	_IOC(_IOC_NONE, 'k', 1, sizeof(int))
#define TRACER_REMOVE_PROCESS	_IOC(_IOC_NONE, 'k', 2, sizeof(int))

#endif
