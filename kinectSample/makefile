all: test

CFLAGS=-fPIC -g -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
INCLUDE = -I/usr/include/libfreenect -I/usr/include/libusb-1.0
FREE_LIBS = -L/usr/lib -lfreenect -lfreenect_cv -lfreenect_sync

test:  test.cpp
	$(CXX) $(INCLUDE) $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o test

