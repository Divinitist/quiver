#!/bin/bash

g++ -I../src -L. -std=c++20 pipe_srv.cpp ../build/libnon_qt_lib.a -o xpipe_srv
g++ -I../src -L. -std=c++20 pipe_cli.cpp ../build/libnon_qt_lib.a -o ypipe_cli