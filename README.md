Sectors statistics
==================

Search for the largest areas without bad sectors


I had a 250GB, 2.5", SATA hard disk laying around with a lot of bad sectors.
I wanted to use it again, because it was the only model I could connect to my
router. I scanned my diks with a program that logged every block with access
time bigger than 50ms. I regexped the log to get the most useful information
from it (format like `LBA address:access time`).


And here is my program. It reads that file, line by line, and computes ranges
without bad sectors. Then it adds it into the array with the largest areas.
Its size is specified by user (`-c` flag, default value is 10). It also sums up
all bad sectors.


Usage
-----

    ./sectors-statistics [-v] [-c count] logfile
        -v tells what am I doing (no, not author)
        -c number of ranges (10 is default)


Notes
-----

I also wanted to add average access time of area, but not listed blocks would be
traktowane as ones with 0ms access time which is not quite true.

To compile program just type `make`.

Here is a picture of my notes which helped me wrote this program:
![My notes](https://raw.github.com/hahiserw/Sectors-statistics/master/notes.jpg)
