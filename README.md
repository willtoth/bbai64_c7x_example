# (Extremely) Minimal C7x remoteproc example for Beaglebone AI-64

C7x "Hello World" on Beaglebone AI-64, showing how to initialize a minimal remoteproc resource table with a working trace log. Be warned that this is only a proof-of-concept, and includes just enough code to get *something* up and running on the C7x (meaning no initialization code for interrupts, cache, MPU, RAT, etc...)

To compile this example, you need the TI C7000 CGT installed, and set the location in the environment variable TI_CGT_C7x.

https://www.ti.com/tool/C7000-CGT

If you were compiling this directly on the Beaglebone AI-64, you can uncomment the commented lines in the Makefile to run the example. If everything worked you should see "Hello world!" printed from the remoteproc trace log.
