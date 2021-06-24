CFLAGS = -std=c++17
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan

vk-test: vk-test.cpp
	clang++-11 $(CFLAGS) -o vk-test vk-test.cpp $(LDFLAGS)

.PHONY: test clean

test: vk-test
	./vk-test

clean:
	rm -rf vk-test
