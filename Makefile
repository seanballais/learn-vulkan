CFLAGS = -std=c++17 -g
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan
SOURCES = main.cpp app.cpp utils/io.cpp utils/vk.cpp

vk-app:
	mkdir -p bin/
	./compile_shaders.sh
	clang++-11 $(CFLAGS) -o bin/vk-app $(SOURCES) $(LDFLAGS)

.PHONY: test clean

run:
	./bin/vk-app

clean:
	rm -rf ./bin/vk-app
