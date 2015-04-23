CIBLE = my_ray_tracer
SRCS = $(wildcard *.cpp)

platform=$(shell uname)

ifeq ($(platform),Darwin)
LIBS =  -framework GLUT -framework OpenGL -lm
else
LIBS =  -L/usr/lib/nvidia/ -lglut -lGLU -lGL -lm
endif

CC = g++
CPP = g++

FLAGS = -Wall -O2 -std=c++11

CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS)

OBJS = $(SRCS:.cpp=.o)   

all: $(CIBLE)

$(CIBLE): $(OBJS)
	g++ $(LDFLAGS) -o $(CIBLE) $(OBJS) $(LIBS)
clean:
	rm -f  *~  $(CIBLE) $(OBJS)
