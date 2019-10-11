﻿#include "StableFluidWidget.h"
struct VertexData
{
	QVector3D position;
	QVector2D texCoord;
};
VertexData vertices[] = {
	{QVector3D(-1.0f, -1.0f,  0.0f), QVector2D(0.0f, 0.0f)},
	{QVector3D(1.0f, -1.0f,  0.0f), QVector2D(1.f, 0.0f)},
	{QVector3D(-1.0f,  1.0f,  0.0f), QVector2D(0.f, 1.f)},
	{QVector3D(1.0f,  1.0f,  0.0f), QVector2D(1.f, 1.f)},

};
//GLushort indices[] = { 0, 1, 2, 1, 3, 2 };
GLushort indices[] = { 0, 1, 2, 3, 3 };

StableFluidWidget::StableFluidWidget(QWidget* parent) :
	m_indexBuf(QOpenGLBuffer::IndexBuffer),
	QOpenGLWidget(parent),
	m_texture(0)
{
	makeCurrent();
}

StableFluidWidget::~StableFluidWidget()
{
	// Make sure the context is current when deleting the texture
	// and the buffers.
	makeCurrent();
	m_arrayBuf.destroy();
	m_indexBuf.destroy();

	delete m_texture;
	doneCurrent();
}

void StableFluidWidget::timerEvent(QTimerEvent* t)
{
	qint64 currentTime = m_calcTimer.elapsed();
	m_deltaTime = (currentTime - m_lastTime) / 1000.0;
	update();
	m_lastTime = currentTime;
}

void StableFluidWidget::initGeometry()
{

	m_arrayBuf.create();
	m_indexBuf.create();
	m_arrayBuf.bind();
	m_arrayBuf.allocate(vertices, 4 * sizeof(VertexData));

	// Transfer index data to VBO 1
	m_indexBuf.bind();
	m_indexBuf.allocate(indices, 6 * sizeof(GLushort));
}

void StableFluidWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glClearColor(0, 0, 0, 1);

	initShaders();
	initTextures();
	initGeometry();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	m_timer.start(12, this);
	m_calcTimer.start();
	qint64 m_lastTime = m_calcTimer.elapsed();
}

void StableFluidWidget::initShaders()
{
	// Compile vertex shader
	if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "vshader.glsl"))
		close();

	// Compile fragment shader
	if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "fshader.glsl"))
		close();

	// Link shader pipeline
	if (!m_program.link())
		close();

	// Bind shader pipeline for use
	if (!m_program.bind())
		close();
}

void StableFluidWidget::initTextures()
{
	// Load cube.png image
	m_texture = new QOpenGLTexture(QImage("cube.png").mirrored());
	/*logo = QImage(":/res/designer.png");
	logo = logo.convertToFormat(QImage::Format_ARGB32);*/

	// Set nearest filtering mode for texture minification
	m_texture->setMinificationFilter(QOpenGLTexture::Nearest);

	// Set bilinear filtering mode for texture magnification
	m_texture->setMagnificationFilter(QOpenGLTexture::Linear);

	// Wrap texture coordinates by repeating
	// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
	m_texture->setWrapMode(QOpenGLTexture::Repeat);
	//m_renderTexture = new QGLFramebufferObject(m_texture->width(), m_texture->height()); 
	m_renderTexture = new QGLFramebufferObject(this->width(), this->height());
	m_renderTexture1 = new QGLFramebufferObject(this->width(), this->height());
	m_initial = false;
}

void StableFluidWidget::resizeGL(int w, int h)
{}

void StableFluidWidget::paintGL()
{
	// Clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw cube geometry using indices from VBO 1
	if (!m_initial)
	{
		m_initial = true;


		m_arrayBuf.bind();
		m_indexBuf.bind();

		// Offset for position
		quintptr offset = 0;

		// Tell OpenGL programmable pipeline how to locate vertex position data
		int vertexLocation = m_program.attributeLocation("i_position");
		m_program.enableAttributeArray(vertexLocation);
		m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

		// Offset for texture coordinate
		offset += sizeof(QVector3D);

		// Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
		int texcoordLocation = m_program.attributeLocation("i_texcoord");
		m_program.enableAttributeArray(texcoordLocation);
		m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

		glActiveTexture(GL_TEXTURE0);

		m_texture->bind();
		m_program.setUniformValue("texture", 0);
		m_renderTexture->bind();
		glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);
		

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);
		//m_texture->release();
		QRect rect(0, 0, this->width(), this->height());
		QGLFramebufferObject::blitFramebuffer(m_renderTexture1, rect, m_renderTexture, rect);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, m_renderTexture1->texture());
		m_renderTexture1->bind();
		glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);
		//glBindTexture(GL_TEXTURE_2D, m_renderTexture->texture());
		m_renderTexture1->bindDefault();
		glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);
		//if (hasOpenGLFramebufferBlit())
		QRect rect(0, 0, this->width(), this->height()); 
		
		/*QGLFramebufferObject::blitFramebuffer(QOpenGLContext::currentContext()->defaultFramebufferObject(), rect, m_renderTexture1, rect);*/
		//QOpenGLContext::currentContext()->swapBuffers();
	}

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
}