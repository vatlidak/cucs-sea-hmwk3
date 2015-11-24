Simulate print spoller
==
Copyright (C) 2015 V. Atlidakis

COMS W4187 Fall 2015, Columbia University

## Project structure

* Makefile: build, (un)install, test, exec
* include/: Header file for defines
* src/addqueue.c: Implements addqueue command
* src/rmqueue.c: Implements rmqueue command
* src/showqueue.c: Implements showqueue command
* scripts/checkpatch.pl: Format checking script
* tests: contain some test media files

## Notes
* Each file added in the queue is named after a 16-bytes unique
  file identifier randomly created from /dev/urandom.

* The installation needs root priviledges to:
  - Create a user "print_spoller"
  - Create a directory "/var/print_spooler"
  - Change owner, enable setuid bit, and copy executables under /bin

## Errors
* Please supress stderr before running scripts to automatic parsing
  of output messages, since standard error is utilized for some error
  messages

## Build
* make [build]

## Installation
* make install

## Test
* make test

## Exec
* make exec_showqueue
* make make exec_addqueue ARGS="./bin/addqueue ./bin/rmqueueu ./bin/*"
* make exec_rmqueue ARGS="nop"
