## Recap

In the previous HOWTO_PART2 I described what you need and how to get your first interaction with a uhf-rfid tag going. Hopefully you could observere a tag response with your SDR hardware.
I did not go into detail about what the arduino sketch is actually doing and was is happening when the reader talks to the tag. In the following I will try to shed some light on this.

## Things behind the curtain - what is happening ?

In this section I will briefly go thru the main code of the arduino sketch, what it does and what you see in the signals you observed.
