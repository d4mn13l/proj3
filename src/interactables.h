#ifndef INTERACTABLES_H
#define INTERACTABLES_H

#include <3ds.h>

// for special interactables 


typedef void(*interactable_f)(u8 *data);

// expects x and y pos of console cell in data[1] and data[2]
void main_console(u8 *data);


static const interactable_f CUSTOM_INTERACTABLES[] = {
	main_console,
};



#endif
