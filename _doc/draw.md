# The Drawing Model

> Note: This model is not exclusive to libui; it is also applicable to many 2D graphics libraries, such as Direct2D, cairo, and Core Graphics.

## The Coordinate System and Points

In the traditional way we think of drawing, we think of rendering onto a plane of pixels. The pixels have a fixed size, and coordinates refer to the entire space that a pixel occupies.

For instance, in the traditional model, the coordinate system looks like

TODO image

and when we say "draw a line from (0, 0) to (5, 5) exclusive", we mean "fill the spaces that are occupied by the pixels at (0, 0), (1, 1), (2, 2), (3, 3), and (4, 4)":

TODO image

Ugh. With pixels as big as the ones TODO.

But now let's pretend we're working in a coordinate system where the point at (x, y) corresponds strictly to the top-left corner of the area that a pixel occupies.

TODO image

In this model, when we say "draw a line from (0, 0) to (5, 5)", we mean "draw a straight line filling every pixel that we cross if we traced a line from the top-left corner of what we used to call the pixel at (0, 0 to the top-left corner of what we used to call the pixel at (5, 5)":

TODO image

TODO.

There are both technical and non-technical reasons for following this model. The technical reason is that implementing certain drawing operations, such as filling shapes, is much easier if we do things this way. The [cairo FAQ](http://www.cairographics.org/FAQ/#sharp_lines) explains in more detail. The non-technical reason has to do with DPI independence.

## DPI Independence vs. DPI Awareness

An upcoming trend in computing is the high-resolution display. These displays fit more dots in the same area that older screens could. The conventional term for the number of dots that fit in a given area is the "dots per inch", or DPI, measure.

A naive approach to writing programs for these new displays is to think "well, if I just take the DPI and only use it in calculations where I need to deal with real-world measurements such as inches, rendering pure pixels as I always have, I should be fine". This kind of design is centered around *DPI awareness*. I know, I used to believe this too. But here's a little secret: this is wrong! A common myth about high-resolution monitors among non-technical people is that it makes the stuff on screen smaller. The mindset I just described causes this: TODO

Instead, what we want out of a high-resolution display is *to show a more detailed view of the same image in the same space*. [The first image on Apple''s discussion of the topic](https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Art/backing_store_2x.png) is the perfect example. On the left, you see a low-resolution monitor. Notice how big chunks of the shapes go into the boxes. When the code that maps points to pixels runs, it can't have two colors in one square, so it has to decide what color to use. TODO

TODO
- talk about how this relates to the OpenGL unit cube
- talk about the various names (point, user space coordinate, device-independent pixel)
- talk about "scaling"
