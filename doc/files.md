# a list of all the files

## in src/

- main

- player
  movement, items

- creature
  creature "ai" (at least the parts that are finished) and movement

- map
  all things that are on the map, so:
  - walls
  - sprites
  - interactables (but not handling interactions with them)
  - reading the map file

- game
  everything that concerns multiple parts of the game, like:
  - tick
  - handling interactions
  - game state (playing, cutscene, text, ...)

- ppm
  all image-related stuff like reading a ppm file, setting pixels etc

- shared
  global variables: delta, log file and game state struct

- text
  the long stuff that is printed when interacting with something

- util
  some util macros, mainly assert stuff

- maths
  vector stuff and some other maths helpers

- interactables
  functions for interactables that dont fit into the standart categories

- cell_enter_actions
  functions for cell_enter_actions (wow)

- keymap
  maps keys

## romfs
map files and textures

## makefile
pretty much the default libctru makefile. just run make to use it.
