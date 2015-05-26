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

## Data Flow

#### AMStartup

0. C program called `AMStartup` is run. Sends `AM_INIT` message to the server
specifying `nAvatars` and `Difficulty`.

0. Wait for `AM_INIT_OK` message.

0. Allocate data structures on the heap.

0. Use `pthread` to start `n` threads of the `amazing_client` startup function.

#### Amazing Client

0. Send `AM_AVATAR_READY` via `MazePort`.

0. While message from server is not `AM_MAZE_SOLVED` or `AM_TOO_MANY_MOVES` or
`AM_SERVER_TIMEOUT` (or some other bad message):

 - Wait for `AM_AVATAR_TURN` from server.

 - Check past move and write message about move.

 - `draw(illegalMoves, lastMoves, newMove, justMovedAvatar)`

 - Update `lastMoves`

 - Avatar with correct AvatarID from `AM_AVATAR_TURN` makes move (algo) and
 sends `AM_AVATAR_MOVE`.

 - One of the clients writes out that the maze is solved. (lock?)

#### Graphics

 - Take current state of illegal moves (walls), last move, and the move that
 was just made, and draw/animate it.

#### Algorithm

 - Arguments: `illegalMoves`, `lastMoves`

 - Returns next move
