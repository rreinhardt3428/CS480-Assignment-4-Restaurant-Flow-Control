# Ethan Kent CS480
# REDID: 826843661
# Roger Reinhardt
# REDID: 826470808
# Makefile

#.RECIPEPREFIX +=

# Specify compiler
CC = g++
# Compiler flags, if you want debug info, add -g
CCFLAGS = -std=c++11 -g3 -Wall -c
CFLAGS = -g3 -c

# Object files
OBJS = producer.o queue.o consumer.o main.o log.o

# Program name
PROGRAM = dineseating

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.cpp
	$(CC) $(CCFLAGS) main.cpp

producer.o : producer.cpp producer.h
	$(CC) $(CCFLAGS) producer.cpp

queue.o : queue.cpp queue.h
	$(CC) $(CCFLAGS) queue.cpp

consumer.o : consumer.cpp consumer.h
	$(CC) $(CCFLAGS) consumer.cpp

log.o : log.cpp log.h
	$(CC) $(CCFLAGS) log.cpp

# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -rf $(OBJS) *~ $(PROGRAM)