all:
	gcc -o test streamdeck.c test.c -lhidapi-libusb
