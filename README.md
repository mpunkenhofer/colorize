# Colorize
Inspired from a [post on stackexchange](https://codegolf.stackexchange.com/a/22326) I created my own c++ implementation.

# Dev progress pictures
Pictures from the development progress can be found on [imgur album](https://imgur.com/a/i4BBQm7).

# How the algorithm colors an image

We start by generating all possible RGB colors for the current resolution. For example for a resolution of 256x128 
we have use all 15bit colors (15-bit colors are the 32768 colors that can be made by mixing 32 reds, 32 greens, and 32 blues)
and randomize the order of colors. 
Next we put the first color in the center of the image (or any location we choose) and get all the neighbors of this pixel 
we just placed. Of this neighbors we choose the best location (where euclidean distance is minimal) for the next color we are 
about to place. As we iterate over the generated colors the number of available locations increases by the number of uncolored 
neighbors of the location we picked for the current color. Once there are no more colors or available locations we are done.

![image 1](https://imgur.com/FHsMIsE)

![image 2](https://imgur.com/Fimji6o)

![image 3](https://imgur.com/nqa0rPP)

![image 4](https://imgur.com/faIQAEw)

![image 5](https://imgur.com/2ytgLGf)

# Video
I made a 1920x1080 render by capturing snapshots every (Nr. of Frames / 660) percent colored for roughly ~10s 60 fps 
of 1080p footage and uploaded it to [youtube.com](http://www.youtube.com/watch?v=tHLiE3ykvAU). 

[![1920x1080 video](http://img.youtube.com/vi/tHLiE3ykvAU/0.jpg)](http://www.youtube.com/watch?v=tHLiE3ykvAU)

