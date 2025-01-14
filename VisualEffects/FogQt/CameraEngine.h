#pragma once
#include <QVector3D>
#include <qmatrix.h>

class CameraEngine
{
public:
	void SetCameraPosition(QVector3D i_position);
	QVector3D GetCameraPosition() { return m_position; }
	/*void SetCameraDirection(glm::vec3 i_direction);
	void SetCameraRight(glm::vec3 i_right);*/
	//void SetPlaneDistance(float i_nearPlane, float i_farPlane);
	CameraEngine() {
		m_position = QVector3D(0, 0, 0);
		m_direction = QVector3D(0, 0, 1);
		m_right = QVector3D(1, 0, 0);
		m_up = QVector3D::crossProduct(m_direction, m_right).normalized();
		m_Yaw = 0;
		m_Pitch = 0;
	}
	void AddAxisOffset(QVector3D i_dir);
	float m_Yaw;
	float m_Pitch;
	float m_PitchTmpOffset;
	float m_YawTmpOffset;
	float CurrentYaw() { return m_Yaw + m_YawTmpOffset;  }
	float CurrentPitch() { return m_Pitch + m_PitchTmpOffset; }
private:
	QVector3D m_position;
	QVector3D m_direction;
	QVector3D m_right;
	QVector3D m_up;
	
	//float m_nearPlane, m_farPlane;
};

