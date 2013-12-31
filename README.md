ttyload
======

##About:
ttyload is a live ncurses cpu usage graph.
The graph's units are in percentage.
It updates once every second.
###Screenshot:
<a href="https://raw.github.com/rigel314/ttyload/master/images/ttyloadCrop.png">![ttyload example](https://raw.github.com/rigel314/ttyload/master/images/ttyloadCropSmall.png)</a>

##Build:
	> make all
A simple `make all` should suffice.  If you get errors using my makefile, see the Questions/Bugs section below.

##Installation:
	> make install
Running `make install` will copy the binary to `/usr/local/bin`.  This should be in your path.
###Uninstallation:
	> make uninstall
Running `make uninstall` will `rm /usr/local/bin/ttyload`.

##Usage:
	> ttyload
'q' will quit.

##Contributions:
I didn't start out doing it, but I will be sticking to the branching model described [here](http://nvie.com/posts/a-successful-git-branching-model/).  Pull requests should also stick to this branching model.  Thanks.

#Questions/Bugs?
[Report A Bug](https://github.com/rigel314/ttyload/issues)<br />
OR<br />
Contact me at <pi.rubiks@gmail.com>
