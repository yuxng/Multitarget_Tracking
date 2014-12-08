################################################################################
# OPENCV
################################################################################
OPENCV_INCLUDES := /usr/local/include
OPENCV_LIBS := /usr/local/lib

CC = g++
LD = g++

CFLAGS = -O3 -Wall -g
LDCFLAGS = -O3 -lm -Wall

INCLUDES += -I$(OPENCV_INCLUDES)
LIBRARIES += -lopencv_highgui -lopencv_core

# Target rules
SRC_C = main.cpp common.cpp hungarian.cpp Target.cpp Tracker.cpp

OBJ_C = $(addsuffix .o, $(basename $(SRC_C)))

all: tracker

tracker: $(OBJ_C)
	$(CC) $(ALL_LDFLAGS) -o $@ $+ $(LIBRARIES) $(LDFLAGS) -L$(OPENCV_LIBS)

%.o: %.cpp
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $<

clean: $(OBJ)
	rm -f $(OBJ_C)
	rm -f tracker
