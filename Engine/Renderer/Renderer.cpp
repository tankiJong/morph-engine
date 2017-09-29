#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations

#include <map>
#include <string>

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"

Renderer::Renderer() {
	loadIdentity();
	m_textures = new std::map<std::string, Texture*>();
}

Renderer::~Renderer() {
	for (const auto& kv: *m_textures) {
		delete kv.second;
	}
	delete m_textures;
}

void Renderer::afterFrame() {
	HWND hWnd = GetActiveWindow();
	HDC hDC = GetDC(hWnd);
	SwapBuffers(hDC);
}

void Renderer::beforeFrame() {}

void Renderer::drawLine(const Vector2& start, const Vector2& end, 
						const Rgba& startColor, const Rgba& endColor, float lineThickness) const {
	glBegin(GL_LINES);
	glLineWidth(lineThickness);
	glColor4ub(startColor.r, startColor.g, startColor.b, startColor.a);
	glVertex2f(start.x, start.y);
	glColor4ub(endColor.r, endColor.g, endColor.b, endColor.a);
	glVertex2f(end.x, end.y);
	glEnd();
}

void Renderer::drawTexturedAABB2(const AABB2& bounds, 
								 const Texture& texture, 
								 const Vector2& texCoordsAtMins, 
								 const Vector2& texCoordsAtMaxs, 
								 const Rgba& tint) {
	glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, texture.m_textureID); 
	glColor4ub(tint.r, tint.g, tint.b, tint.a);
	glBegin(GL_QUADS);
	{
		//glTexCoord2i(0, 0); glVertex2i(100, 100);
		//glTexCoord2i(0, 1); glVertex2i(100, 500);
		//glTexCoord2i(1, 1); glVertex2i(500, 500);
		//glTexCoord2i(1, 0); glVertex2i(500, 100);
		glTexCoord2f(texCoordsAtMins.x, texCoordsAtMins.y);
		glVertex2f(bounds.mins.x, bounds.mins.y);
		glTexCoord2f(texCoordsAtMaxs.x, texCoordsAtMins.y);
		glVertex2f(bounds.maxs.x, bounds.mins.y);
		glTexCoord2f(texCoordsAtMaxs.x, texCoordsAtMaxs.y);
		glVertex2f(bounds.maxs.x, bounds.maxs.y);
		glTexCoord2f(texCoordsAtMins.x, texCoordsAtMaxs.y);
		glVertex2f(bounds.mins.x, bounds.maxs.y);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void Renderer::setOrtho2D(const Vector2& bottomLeft, const Vector2& topRight) {
  loadIdentity();
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
	//glOrtho(0.f, SCREEN_WIDTH, 0.f, SCREEN_HEIGHT, 0.f, 1.f);
}

void Renderer::pushMatrix() {
	glPushMatrix();
}

void Renderer::popMatrix() {
	glPopMatrix();
}

void Renderer::traslate2D(const Vector2& translation) { // QA: better to have 2 different fn for 2D and 3D? 
	glTranslatef(translation.x, translation.y, 0);
}

void Renderer::rotate2D(float degree) {
	glRotatef(degree, 0.f, 0.f, 1);
}

void Renderer::scale2D(float ratioX, float ratioY, float ratioZ) {
	glScalef(ratioX, ratioY, ratioZ);
}

void Renderer::swapBuffers(HDC ctx) {
	SwapBuffers(ctx);
}

void Renderer::loadIdentity() {
	glLoadIdentity();
}

void Renderer::drawAABB2(const AABB2& bounds, const Rgba& color) {
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_QUADS);
	{
		glVertex2f(bounds.mins.x, bounds.mins.y);
		glVertex2f(bounds.maxs.x, bounds.mins.y);
		glVertex2f(bounds.maxs.x, bounds.maxs.y);
		glVertex2f(bounds.mins.x, bounds.maxs.y);
	}
	glEnd();
}

void Renderer::cleanScreen(const Rgba& color) {
	float r = 0, g = 0 , b = 0, a = 1;
	color.getAsFloats(r, g, b, a);
	glClearColor(r,g,b,a);
	glClear(GL_COLOR_BUFFER_BIT);// TODO: move to renderer
}

Texture* Renderer::createOrGetTexture(const std::string& filePath) {
	auto it = m_textures->find(filePath);
	if (it == m_textures->end()) {
		Texture* texture = new Texture(filePath);
		(*m_textures)[filePath] = texture;
		return texture;
	}
	return it->second;
}
