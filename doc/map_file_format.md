# the totally sane map file format

the map file format consists of 4 sections that are seperated by a ]:

## wall section
on the first line the dimensions of the map seperated by a space. 
then essentially a top-down view of the map. to represent a wall, either put the index of the texture or # (for index 2). for no wall put a space. 
also make sure to put spaces at the end of lines so that they all have the specified length.

## cell enter action section
each line is in the format

x y name

with types x: int, y: int, name: string

where the cell enter action (cea) with name name will be called every time the player enters cell (x, y). 
the cea and its name are taken from the cell_enter_actions array in cell_enter_actions.h

## sprite section
each line is in the format

x y tx ty

with types x, y: float, tx, ty: int

this puts a non-interactable sprite with texture (tx, ty) at (x, y). 
the rest of the line after these 4 numbers is ignored so you can put comments or whatever you want.

## interactables section
each line is in the format

type x y tx ty d0 d1 d2

with types x, y: float, type, tx, ty: int, d0, d1, d2: u8

this puts an interactable sprite with texture (tx, ty) at (x, y). 
it has interaction type type (see map.h for the different types) and [d0, d1, d2] as its data. 
different interaction types use this data for different stuff, which is also explained in map.h. 

again the rest of the line after these 4 numbers is ignored so you can put comments or whatever you want.
