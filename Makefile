
all: run

water:
	g++ src/A6-Water.cpp -o build/a6 -g -lglfw -lGLEW -lOpenGL

run: water
	./build/a6

clean:
	rm -f a.out
