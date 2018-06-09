# SLog By Scanner

# a simple muti-plat, muti-thread C++ style logframe 

## needs C++11 or higher , in unix based system, use -lpthread

## use it by

first: include this file in your project

do logs first by:

~~~
#include "log.h"
~~~

you can do logs like:

~~~
Log::begin() << "something you want write" << 33 << Log::end();
~~~

or use macro
~~~
LogFunc("something you want write" << 33)
~~~

you can get your log file in  local working path

## this main.cpp is a simple example for this log

for more information, Read these codes