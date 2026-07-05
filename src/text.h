#ifndef LORE_H
#define LORE_H


static const char text[][1024] = {
	// hand written note in work room
	"\x1b[3m(a hand-written note)\x1b[0m\n" // this is supposed to be cursive but its not supported apparently
	"\x1b[1;4mProject 3 Press Release sketch\x1b[0m\n"
	"Project 3: A research project by DaEVIL Corp. Ltd. exploring the secrets of the depths of our oceans.\nThe follow-up to Project 1 and 2 where uncrewed drones were sent down the Mariannah Trench "
	"to gather information about its biosphere and potential natural resources.\n"
	"Both drones detected strange electromagnetic signals, which eventually became so strong that connection to them was lost. The only way to continue "
	"research was to send a crewed expedition. Four brave researches \x1b[9magreed to be on the submarine because we are holding their families hostage\x1b[0m volunteered to join the expedition "
	"to contribute to \x1b[9mour annual profit growth\x1b[0m the scientific progress of humanity."
	,

	// dead crewmate in dorm
	"(a recording of the voice of one of your crewmates)\n"
	"(you hear cries and explosions in the background)\n"
	"shitshitshitshitshit\n"
	"(you hear the lamps starting to blink)\n"
	"nonono not now where is my flashlight\n"
	"(the blinking stops)\n"
	"fuck i cant see shit\n"
	"AAAAAAAAAAAHHHH\n"
	"(suddenly your crewmate cries out in pain)"
	,

	// main console when captains hand is not in inventory
	"(the main console to the board computer with access to all the submarines controls)\n"
	"(you put you hand on the scanner but it gets rejected)\n"
	"(probably the captian is the only one authorised to access it)\n"
	,

	// main console when captains hand is in ventory
	"(you put the captain's hand on the scanner)\n"
	"> Access granted\n"
	"(you click on the red \"UNLOCK ESCAPE PODS\" button, but the AI refuses)\n"
	"> While I recognise the imminent threat of painful death and fully understand how that may make you feel uncomfortable, sadly I cannot let you leave. "
	"There is still valuable corporate property down in the mines that must not be left behind.\n"
	"Since you are the only one still alive, it is your responsability to take them with you to the surface.\n"
	". . . .\n"
};


#endif
