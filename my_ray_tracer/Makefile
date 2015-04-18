CIBLE = my_ray_tracer
SRCS = $(wildcard *.cpp)
LIBS =  -L/usr/lib/nvidia/ -lglut -lGLU -lGL -lm 

CC = g++
CPP = g++

FLAGS = -Wall -O2 

CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS)

OBJS = $(SRCS:.cpp=.o)   

all: $(CIBLE)

$(CIBLE): $(OBJS)
	g++ $(LDFLAGS) -o $(CIBLE) $(OBJS) $(LIBS)
clean:
	rm -f  *~  $(CIBLE) $(OBJS)
