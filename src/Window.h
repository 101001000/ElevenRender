#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


struct PixelBuffer {
	int width, height, channels;
	float* data;
};

class Window {

public:

	GLFWwindow* window;
	unsigned int VBO, VAO, EBO, ID;
	unsigned int texture;

	PixelBuffer previewBuffer;
	int width, height;

	Window(int _width, int _height);

	int init();

	void renderUpdate();

	void stop();


};


