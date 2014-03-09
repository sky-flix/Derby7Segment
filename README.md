Derby7Segment
=============

This arduino code is used in conjection with a Netduino powered Pinewood Derby Timer. In order to control a series of 8 Sparkfun OpenSegment displays, a wireless serial link is established between the timer and an arduino. The arduino then interprets serial commands from the Netduino in order to display finisher order, times and other miscellaneous text based status updates (at least, as well as 7-segment LEDs can). The arduino communicates with the 8 OpenSegments over I2C (Two-Wire), using the wire.h library.

http://www.youtube.com/watch?v=XtziGewhQ8U

https://thelevers-public.sharepoint.com/Pages/IMG_5491.png

https://thelevers-public.sharepoint.com/Pages/IMG_5498.png

https://thelevers-public.sharepoint.com/Pages/IMG_5499.png
