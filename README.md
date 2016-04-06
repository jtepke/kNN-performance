#INTRODUCTION#
This is a proof-of-concept implementation which aims to provide a convenient tool to run performance tests against the implemented approaches for the k-nearest neighbor problem. Right now, the naive column scan algorithm, an on-the-fly grid approach and a MapReduce implementation are available. All approaches use a fairly similar point data model in order to remain comparable.

#BUILD#
This project can be build in release and debug mode. All debug related files can be found in the _Debug/_ folder. Release related things can be found under _Release/_. Both builds can be initiated by simply typing

    make all

on the command line in respective folder.

##REQUIREMENTS##
* Unix operating system
* gcc version supporting c++11
* make
* Pthreads

#Exmaples#
In this section the test runner will be introduced briefly by discussing some examples.

... to be continued....
