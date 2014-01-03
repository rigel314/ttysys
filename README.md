ttysys v0.5
======

##About:
ttysys is a live ncurses cpu usage graph.
The graph's units are in percentage.
It updates once every second.
###Screenshot:
<a href="https://raw.github.com/rigel314/ttysys/c86392632269f3c593652013e9bc481b29049d34/images/ttysys.png">![ttysys example](https://raw.github.com/rigel314/ttysys/c86392632269f3c593652013e9bc481b29049d34/images/ttysysSmall.png)</a>

##Build:
	> make all
A simple `make all` should suffice.  If you get errors using my makefile, see the Questions/Bugs section below.

##Installation:
	> make install
Running `make install` will copy the binary to `/usr/local/bin`.  This should be in your path.
###Uninstallation:
	> make uninstall
Running `make uninstall` will `rm /usr/local/bin/ttysys`.

##Usage:
	> ttysys
* `?` - Displays a help window.
* `h` - Split current window horizontally.
* `v` - Split current window vertically.
* `u` - Un-split current window.
* Tab - Move to next window in order of creation.
* Arrow Keys - Move to next window on screen in direction pressed.
* Numbers `0` - `9` - Select a data source for a window.
	* `0` will set the data source to an overview, and `1` - `9` set it to a specific core.
* `g` - Toggle grid for selected window.
* `e` - Toggle value display in current window's title.
* `t` - Toggle display of current window's title bar.
* `l` - Toggle display of current window's label sidebar.
* `q` - Quit this program.

###How it works:
It works by reading the first few lines in `/proc/stat` that begin with cpu.<br />
`man 5 proc` explained the meaning of contents of `/proc/stat`.<br />
These lines tell you how much time each CPU spent in different states.  The sum of each line is the total time spent for each CPU.  I read this file twice with a second in between.  Then, I subtract the two totals to have the total CPU time spent during my `sleep()`.  Now, I add the user and system numbers together and divide by my difference.  Finally, it's just a matter of displaying it nicely.

##Contributions:
I didn't start out doing it, but I will be sticking to the branching model described [here](http://nvie.com/posts/a-successful-git-branching-model/).  Pull requests should also stick to this branching model.  Thanks.

#Questions/Bugs?
[Report A Bug](https://github.com/rigel314/ttysys/issues)<br />
OR<br />
Contact me at <pi.rubiks@gmail.com>
