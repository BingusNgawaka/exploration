# Vim like CLI text editor
I've wanted to make some kind of CLI tool for a while but couldn't think of or find any ideas that piqued my interest so I just did a vim like text editor.

## Features
- Mode based text editing
- Simple text input while in "input" mode
- Simple commands to alter lines and change modes:
    - hjkl: movement while in "normal" mode
    - 'i': enter insert mode
    - 'a': move cursor forward and enter insert mode
    - 'A': move cursor to end of line and enter insert mode
    - 'o': insert new line below and enter insert mode
    - 'O': insert new line above and enter insert mode
    - ':': enter command mode
- Command line to enter commands for reading and writing to file
- Relative line numbers and infinitely scrolling screen (vert and horiz)
- Super cool epic color pallete

## Todo
- add more keybinds like 'b', 'n' etc.
- copy paste buffer
- visual select mode
- make safer to use, ie verify input when in insert mode and fix rendering of tabs and other chars
- change color palletes
- undo history

--

# Dependencies
[just](https://github.com/casey/just) by casey. Install on debian:
```
apt install just
```

[ncurses](https://invisible-island.net/ncurses/man/ncurses.3x.html). Install on debian:
```
apt install libncurses-dev
```

# Usage
just commands:
```
just build
just run "filepath"
just build-and-run "filepath"
```
binary is compiled to ./build dir
