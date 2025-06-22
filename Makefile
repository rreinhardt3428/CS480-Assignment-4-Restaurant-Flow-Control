# Roger Reinhardt
# 826470808
#
#

# Makefile

# Specify compiler
CC = g++
CCFLAGS = -std=c++11 -g3 -Wall -c
CFLAGS = -g3 -c

# Object files
OBJS = main.o producer.o consumer.o shared_queue.o log.o

# Program name
PROGRAM = dineseating

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.cpp
	$(CC) $(CCFLAGS) main.cpp

producer.o : producer.cpp producer.h shared_queue.h seating.h
	$(CC) $(CCFLAGS) producer.cpp

consumer.o : consumer.cpp consumer.h shared_queue.h seating.h
	$(CC) $(CCFLAGS) consumer.cpp

log.o : log.cpp log.h
	$(CC) $(CFLAGS) log.cpp

clean :
	rm -rf $(OBJS) *~ $(PROGRAM)
