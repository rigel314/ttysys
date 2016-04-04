ttysys v1.0
======

##About:
ttysys is an ncurses system monitor geared towards live graphs.
Supports plugins.
Comes with plugins for a cpu percentage graph, and RAM/SWAP graph
###Screenshot:
<a href="https://raw.github.com/rigel314/ttysys/26abbe0964db8f627d064974d4958542c10edd65/images/ttysys.png">![ttysys example](https://raw.github.com/rigel314/ttysys/26abbe0964db8f627d064974d4958542c10edd65/images/ttysysSmall.png)</a>

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
	> ttysys [<sequence>]
_sequence_ can consist of any number of the following characters:

* `h` - Split current window horizontally.
* `v` - Split current window vertically.
* `c` - Close current window.
* `~` - Enter command mode.
* Escape/Ctrl-C - Leave command mode.
* Tab - Move to next window in order of creation.
* Arrow Keys/u/d/r/l - Move to next window on screen in direction pressed.
* `g` - Toggle grid for selected window.
* `e` - Toggle value display in current window's title.
* `t` - Toggle display of current window's title bar.
* `o` - Toggle display of current window's ordinate label sidebar.
* `q` - Quit this program.

These same buttons will control the program while it is running.

##Command Mode:
Commands are in the form: <period> <name>(<arg1>,<arg2>,...)

_period_: Time between refreshes in deciseconds

_name_: Loads _name_.so from LD_LIBRARY_PATH

_args_: Plugin-specific arguments

Example: `10 cpu(1)`

##How it works:
###CPU
It works by reading the first few lines in `/proc/stat` that begin with cpu.<br />
`man 5 proc` explains the meaning of the contents of `/proc/stat`.<br />
These lines tell you how much time each CPU spent in different states.  The sum of each line is the total time spent for each CPU.  I read this file twice with a second in between.  Then, I subtract the two totals to have the total CPU time spent during my `sleep()`.  Now, I add the user and system numbers together and divide by my difference.  Finally, it's just a matter of displaying it nicely.

###Memory
To check the RAM and Swap, I read the appropriate lines in `/proc/meminfo`.  MemTotal, MemFree, Buffers, Cached, SwapTotal, and SwapFree are all the lines that ttysys reads.  MemFree + Buffers + Cached is the amount of free RAM usually reported by other tools, so I conformed.

##Contributions:
I will be sticking to the branching model described [here](http://nvie.com/posts/a-successful-git-branching-model/).  Pull requests should also stick to this branching model.  Thanks.

#Questions/Bugs?
[Report A Bug](https://github.com/rigel314/ttysys/issues)<br />
OR<br />
Contact me at <pi.rubiks@gmail.com>
