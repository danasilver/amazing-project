## Amazing Project Design
Updated 24 May 2015

## Data Structures

### Illegal moves

An *n* by *m* 2D array containing identifying illegal moves by wall location,
where *n* is the height of the maze and *m* is the width.

Each cell of the array will contain `char walls[4]`, a `char *` containing
some subset of `NSEW` indicating the walls relative to that coordinate.

Adding a wall will add the corresponding walls for adjacent cells.

### Past moves

An array of size *n*, where *n* is the number of avatars. Each array cell
contains a `struct location` with members `int x` and `int y`. Avatar `i`'s
last location is at index 'i'.

This is useful to detect if a turn succeeded and update the past move.

If the turn did not succeed, we can use this information to update the illegal
moves array based on where it was before and which direction it tried to move.

