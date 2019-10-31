﻿#include "StableFluidWidget.h"
#include "util.h"
#include "Compution.h"
#include <string>


struct VertexData
{
	QVector3D position;
	QVector2D texCoord;
};


StableFluidWidget::StableFluidWidget(QWidget* parent) :
	m_indexBuf(QOpenGLBuffer::IndexBuffer),
	QOpenGLWidget(parent),
	m_texture(0)
{
	m_isPressed = false;
	m_width = 150;
	m_height = 150;
	makeCurrent();
}

StableFluidWidget::~StableFluidWidget()
{
	// Make sure the context is current when deleting the texture
	// and the buffers.
	makeCurrent();
	m_arrayBuf.destroy();
	m_indexBuf.destroy();
	m_speedBuf.destroy();

	delete m_texture;
	doneCurrent();
}

void StableFluidWidget::timerEvent(QTimerEvent* t)
{
	qint64 currentTime = m_calcTimer.elapsed();
	m_time = currentTime / 1000.0;
	m_deltaTime = (currentTime - m_lastTime) / 1000.0;
	update();
	m_lastTime = currentTime;
}

void StableFluidWidget::initGeometry()
{

	m_geometry = static_cast<cGeometry>(SquraGeometry(m_width + 1, m_height + 1));
	m_arrayBuf.create();
	m_arrayBuf.bind();
	m_arrayBuf.allocate(&m_geometry.m_vertices[0], m_geometry.m_vertices.size() * sizeof(cGeometryVertex));

	// Transfer index data to VBO 1
	m_indexBuf.create();
	m_indexBuf.bind();
	m_indexBuf.allocate(&m_geometry.m_indices[0], m_geometry.m_indices.size() * sizeof(GLuint));

	m_speedBuf.create();
	m_speedBuf.bind();
	m_velocity = Fluid::Compution::vectorFiledGrid(m_geometry.m_vertices.size(), QVector2D(0, 0));
	m_interVelocity1 = Fluid::Compution::vectorFiledGrid(m_geometry.m_vertices.size(), QVector2D(0, 0));
	m_interVelocity2 = Fluid::Compution::vectorFiledGrid(m_geometry.m_vertices.size(), QVector2D(0, 0));
	m_density = Fluid::Compution::vectorFiledGrid(m_geometry.m_vertices.size(), QVector2D(1, 1));
	m_div = Fluid::Compution::scalarFieldGrid(m_geometry.m_vertices.size(), 0);
	m_P1 = Fluid::Compution::scalarFieldGrid(m_geometry.m_vertices.size(), 0);
	m_P2 = Fluid::Compution::scalarFieldGrid(m_geometry.m_vertices.size(), 0);
	m_speedBuf.allocate(&m_velocity[0], m_geometry.m_vertices.size() * sizeof(QVector2D));

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
	m_timer.start(1, this);
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
	m_texture = new QOpenGLTexture(QImage("cube1.jpg").mirrored());
	m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
	m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
	m_texture->setWrapMode(QOpenGLTexture::Repeat);
	m_renderTexture = new QGLFramebufferObject(this->width(), this->height());
	m_renderTexture1 = new QGLFramebufferObject(this->width(), this->height());
	m_initial = false;
}

void StableFluidWidget::resizeGL(int w, int h)
{}


void StableFluidWidget::mousePressEvent(QMouseEvent* e)
{
	m_mousePositionForVelocity = QVector2D(1 - QCursor::pos().y() * 1.0 / this->height(), QCursor::pos().x() * 1.0 / this->width());
	m_firstTime = true;
	m_isPressed = true;
}

void StableFluidWidget::mouseReleaseEvent(QMouseEvent* e)
{
	m_isPressed = false;

}
void test1(int i_n, int i_m);
void StableFluidWidget::paintGL()
{
	test1(m_height, m_width);
	// Clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*velocity step
	add_source(N, u, u0, dt); add_source(N, v, v0, dt);
	SWAP(u0, u); diffuse(N, 1, u, u0, visc, dt);
	SWAP(v0, v); diffuse(N, 2, v, v0, visc, dt);
	project(N, u, v, u0, v0);
	SWAP(u0, u); SWAP(v0, v);
	advect(N, 1, u, u0, u0, v0, dt); advect(N, 2, v, v0, u0, v0, dt);
	project(N, u, v, u0, v0);*/

	float m_vicosityParam = 1e-5;
	float m_forceExponentParam = 200;

	m_previousMousePositionForVelocity = m_mousePositionForVelocity;
	auto mpos = QWidget::mapFromGlobal(QCursor::pos());
	m_mousePositionForScreen = QVector2D(mpos.x() * 1.0 / this->width(), 1 - mpos.y() * 1.0 / this->height());
	m_mousePositionForVelocity = QVector2D(1 - mpos.y() * 1.0 / this->height(), mpos.x() * 1.0 / this->width());

	QVector2D m_forceVector(0, 0);
	if (m_isPressed)
	{
		if (!m_firstTime)
		{
			m_forceVector = (m_mousePositionForVelocity - m_previousMousePositionForVelocity) * 50;
		}
		m_firstTime = false;
	}

	m_program.setUniformValue("i_time", (float)m_time);
	m_program.setUniformValue("i_deltaTime", (float)m_deltaTime);
	m_program.setUniformValue("i_forceOrigin", m_mousePositionForScreen + (!m_isPressed) * QVector2D(2.0,2.0));
	m_program.setUniformValue("i_forceExponent",300.0f);

	// Draw cube geometry using indices from VBO 1
	if (!m_initial)
	{
		m_initial = true;


		m_arrayBuf.bind();
		
		
		// Offset for position
		quintptr offset = 0;

		// Tell OpenGL programmable pipeline how to locate vertex position data
		int vertexLocation = m_program.attributeLocation("i_position");
		m_program.enableAttributeArray(vertexLocation);
		m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(cGeometryVertex));

		// Offset for texture coordinate
		offset += 3 * sizeof(float);

		// Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
		int texcoordLocation = m_program.attributeLocation("i_texcoord");
		m_program.enableAttributeArray(texcoordLocation);
		m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(cGeometryVertex));
		
		

		m_indexBuf.bind();
		glActiveTexture(GL_TEXTURE0);

		m_texture->bind();
		m_program.setUniformValue("texture", 0);
		m_renderTexture->bind();
		glDrawElements(GL_TRIANGLES, m_geometry.m_indices.size(), GL_UNSIGNED_INT, 0);
		QRect rect(0, 0, this->width(), this->height());
		QGLFramebufferObject::blitFramebuffer(m_renderTexture1, rect, m_renderTexture, rect);
	}
	else
	{
	
		m_speedBuf.bind();
		m_speedBuf.write(0, &m_velocity[0], m_geometry.m_vertices.size() * sizeof(QVector2D));
		int speedLocation = m_program.attributeLocation("i_speed");
		m_program.enableAttributeArray(speedLocation);

		m_program.setAttributeBuffer(speedLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));
		glBindTexture(GL_TEXTURE_2D, m_renderTexture1->texture());
		m_renderTexture1->bind();
		glDrawElements(GL_TRIANGLES, m_geometry.m_indices.size(), GL_UNSIGNED_INT, 0);
		m_renderTexture1->bindDefault();
		glDrawElements(GL_TRIANGLES, m_geometry.m_indices.size(), GL_UNSIGNED_INT, 0);
		//QRect rect(0, 0, this->width(), this->height()); 
	}
}