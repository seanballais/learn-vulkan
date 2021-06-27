CFLAGS = -std=c++17 -g
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan

hello-triangle: hello_triangle.cpp
	clang++-11 $(CFLAGS) -o bin/hello_triangle hello_triangle.cpp $(LDFLAGS)

.PHONY: test clean

run: vk-test
	./bin/hello_triangle

clean:
	rm -rf ./bin/hello_triangle
