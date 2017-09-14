#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

#include "Renderer.hpp"
#include "Engine/Core/Rgba.hpp"

void Renderer::afterFrame() {}

void Renderer::beforeFrame() {}

void Renderer::drawLine(const Vector2 & start, const Vector2 & end, const Rgba& color) const {
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_LINES);
	glVertex2f(start.x, start.y);
	glVertex2f(end.x, end.y);
	glEnd();
}

void Renderer::setOrtho2D(const Vector2 & bottomLeft, const Vector2 & topRight) {
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
	//glOrtho(0.f, SCREEN_WIDTH, 0.f, SCREEN_HEIGHT, 0.f, 1.f);
}

void Renderer::pushMatrix() {
	glPushMatrix();
}

void Renderer::popMatrix() {
	glPopMatrix();
}

void Renderer::traslate(const Vector2 & translation) { // QA: better to have 2 different fn for 2D and 3D? 
	glTranslatef(translation.x, translation.y, 0);
}

void Renderer::rotate2D(float degree) {
	glRotatef(degree, 0.f, 0.f, 1);
}

void Renderer::scale(float ratio) {
	glScalef(ratio, ratio, ratio);
}

void Renderer::swapBuffers(HDC ctx) {
	SwapBuffers(ctx);
}

void Renderer::loadIdentity() {
	glLoadIdentity();
}

void Renderer::cleanScreen(const Rgba & color) {
	float r = 0, g = 0 , b = 0, a = 1;
	//color.getAsFloats(r, g, b, a);
	glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT);// TODO: move to renderer
}
