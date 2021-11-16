.PHONY: clean

CC = g++


all:	aofparser.cc
	$(CC) $^ -g -std=c++11 -o aofparser.app

clean:
	rm -rf ./aofparser.app
