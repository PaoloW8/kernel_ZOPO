kernel_ZOPO
===========

ZOPO kernel source code

ZOPO has never released the kernel source of ZP980 and ZP990
I managed to compile a compatible kernel using the sources of different phones (iocean and yaiyu)

first of all thanks to dr-shadow (https://github.com/Dr-Shadow/android_kernel_mt6589) for his kernel source that I used as base

this kernel is compatible with ZOPO model: 980, 990, C3

to choose which model to compile you have to edit the file build.sh and change the variable MODEL (valid values ​​are 990a, 990b, 980a, 980b, C3)

for the 980 and 990 there are variants that differ in the sensor accelerometer, try first with the A version...if the rotation of the screen (in landscape) is upside down recompile with B version

you also need to edit the path of toolchain


for compile:
./build.sh

for clean
./clean.sh
