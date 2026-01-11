CXX = g++
CC  = gcc
CXXFLAGS = -g -std=c++17 -Wall -Iinclude -Ithirdparty/slogger
LDFLAGS = -lcurl -lxml2

SRC_CPP = main.cpp src/HttpsClient.cpp src/HtmlParser.cpp
SRC_C   = thirdparty/slogger/slogger.c thirdparty/slogger/zip.c


OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C   = $(SRC_C:.c=.o)
OBJ     = $(OBJ_CPP) $(OBJ_C)

TARGET = bin/WebCrawler

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) -g -Wall -Iinclude -Ithirdparty/slogger -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)