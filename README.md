# jpegtofb

Version 0.1b

## What is this?

`jpegtofb` is a utility that writes JPEG images direct to a
Linux framebuffer. It can write a single image, or a sequence of
images as a slideshow. It's completely self-contained -- the
relevant parts of the traditional Linux JPEG library are built
into the code. Consequently there are no dependencies apart 
from the standard C library.

This utility is intended for 'digital photo frame' applications
on embedded Linux systems. It builds cleanly on a 
Raspberry Pi, and works
with the standard 7" touchscreen. Other screens that provide
a standard framebuffer interface will probably work as well.
A plug-in HDMI display may or may 
not provide a framebuffer interface,
dependending on the version of the Pi and how it is set up.

`jpegtofb` is somewhat like `fbv`, except that `jpegtofb` has
facilities for filtering images from non-images, making the
slideshow feature easier to use. 

The utility will scale JPEGs to fit the framebuffer, so far
as possible, keeping the aspect ratio correct.
It can handle images of any reasonable
size, although really huge (> 10 megapixels or so) images
may take a few seconds to load on embedded systems.

## Building

`jpegtofb` is designed to be built using GCC. It uses some features
that are specific to GCC and, although these features could be implemented
for other compilers, this would require some extra effort.

    $ make
    $ sudo make install

## Running
    $ sudo jpegtofb /path/to/images/*.jpg 

Any number of image files can be specified. With multiple images, they
will be displayed for a selectable time.

## Command-line switches

`-d,--fbdev=device`

Specify the framebuffer device. The default is `/dev/fb0`.

`-f,--fit-width`

Scale the image so that it fits the width or the framebuffer
rather than the height, which is the default. This will nearly
always fill the screen, if the images are photos. However,
a significant amount of the top and bottom of the image might
be cut off.

`-l,--landscape`

Include only landscape-format images in slideshow mode.

`-r,--randomize`

Randomize the order of presentation of images in slideshow
mode.

`--log-level=0..5`

Set the verbosity of logging. The default is 2; levels 3
and above will probably only comprehensible alongside the 
source code.

`-s,--sleep=seconds`

Set the amount of time to wait between images in slideshow
mode.

`--syslog`

Write messages to the system log. 

`-x,--exec=cmd`

Executes the shell command after changing the image, in slideshow
mode. The purpose of this option is to allow this utility to
broadcast a signal (e.g., using 'killall') to other programs that
may need to redraw after the background has changed.

## Notes 

You can include any number of files in slideshow mode, and
it shouldn't be a problem if some of those files are not
JPEG -- 
`jpegtofb` will filter out the files it can display and 
ignore the rest. 

The user needs to have access rights to the framebuffer device. 
Conventionally this is owned by `root` and neither readable nor
writeable by anybody else. Either `jpegtofb` needs to be run
as a privileged user, or the permissions need to be relaxed
on the framebuffer.

In slideshow mode, you can send a `USR1` signal to skip the
wait, and go straight to the next picture.

`jpegtofb` will not change the aspect ratio of an image, which is
very ugly. It either fits the image to the width of the framebuffer,
or the height. There is no foolproof way to figure out in 
advance which method will be best -- it depends on the images.

## Limitations

`jpegtofb` writes direct to a Linux framebuffer. It is really
designed for embedded applications, although it does work
on desktop Linux systems. However, it almost certainly won't
work in an X session or other Linux desktop -- if the desktop
uses the framebuffer, this utility will be competing with the
desktop. If the desktop doesn't use the framebuffer, framebuffer
display will probably be suppressed.

`jpegtofb` supports only RGB JPEGs. I haven't seen a JPEG from a
digital camera or a photo editing program that isn't RGB, but 
I know they exist. Unfortunately, I can't find any to test.

Only 32-bit framebuffers are supported. Again, I know that 16-bit 
framebuffers exist, but I don't have access to one to test with.

Early Raspberry Pi systems and units of similar specification
similar might struggle to find enough memory to handle really large
images. 

The scaling process this utility uses is crude, but fast. It does
not interpolate -- it just picks a sampling of pixels from the source
image and puts them on the display. In practice, this seems to
produce reasonable results on the kinds of screens embedded devices
are likely to have. It would not be difficult to implement
proper interpolation, but it would be very slow on the kind of
hardware `jpegtofb` is designed for.

## Legal, etc 

`jpegtofb` is copyright (c)2020 Kevin Boone, and distributed under
the terms of the GNU Public Licence, v3.0. You can
use this software however you like, so long as the original authors
continue to be acknowledged, and source code remains available.

The embedded JPEG library is copyright (c)1991-1998 Thomas G Lane
and others, and distributed under a less restrictive licence than
the GPL. In particular, this licence does not require source code
to be made available, while the GPL does. 

There is no warranty of any kind.

## Bugs

Undoutedly there are some. Please report bugs through GitHub.

## Version history

1.0a Feb 2020 <br/>
First working version<br/>

1.0b Feb 2020<br/> 
Added options to control screen fitting<br/> 
Added --exec option<br/> 

1.0c Feb 2020<br/> 
Added code to allow for framebuffers with stride != 
width * bytes\_per\_pixel (thanks [rajeevvp])<br/>
Optimized arithmetic in the framebuffer byte transfer (and made
it even more unreadable ;) )<br/>


