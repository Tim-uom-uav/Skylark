#ifndef MISSION_CONTROL_CLASS_CAMERA_H_
#define MISSION_CONTROL_CLASS_CAMERA_H_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	const int CAMERA_MODE_TARGET = 100;
	const int CAMERA_MODE_ANGLE = 101;
private:
	int mode = CAMERA_MODE_ANGLE;
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 target;
	// Euler Angles
	float Yaw;
	float Pitch;
	float Roll;
public:
	Camera() {
	}
	~Camera() {
	}
	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float yaw = 0, float pitch = 0)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Roll = 0;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix(){
		return glm::lookAt(Position, Position + Front, Up);
	}
	int setPosition(glm::vec3 position) {
		Position = position;
		updateCameraVectors();
		return 1;
	}
	glm::vec3 getPosition() {
		return Position;
	}
	int setMode(int newMode) {
		mode = newMode;
		return 1;
	}
	int getMode() {
		return mode;
	}
	//Set the camera angle in degrees from looking down the X-axis
	int setAngle(float pitch, float yaw, float roll) {
		Pitch = pitch;
		Yaw = yaw;
		Roll = roll;
		updateCameraVectors();
		return 1;
	}
	int lookAt(glm::vec3 target) {
		glm::vec3 directionVec = glm::normalize(target-Position);
		updateCameraVectors(directionVec);
		return 1;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors(glm::vec3 front = glm::vec3( 0,0,0 )) {
		if (mode == CAMERA_MODE_ANGLE) {
			// Calculate the new Front vector
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.z = sin(glm::radians(Pitch));
			front.y = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		}
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};


class OrthoCamera {
private:
	glm::vec3 position;
	double aspectRatio;
	double viewWidth;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
public:
	OrthoCamera() {
		position = glm::vec3(0, 0, 1);
		aspectRatio = 1;
		viewWidth = 200000; //default to 200km width
		updateMatrices();
	}
	~OrthoCamera() {
	}
	glm::vec3 setPos(glm::vec3 newPos) {
		glm::vec3 oldPos = position;
		position = newPos;
		updateMatrices();
		return oldPos;
	}
	glm::vec3 move(glm::vec3 deltaPos) {
		glm::vec3 oldPos = position;
		position += deltaPos;
		updateMatrices();
		return oldPos;
	}
	double setViewWidth(double newWidth) {
		double oldWidth = viewWidth;
		viewWidth = newWidth;
		updateMatrices();
		return oldWidth;
	}
	double zoom(double amount) {
		double oldWidth = viewWidth;
		viewWidth = viewWidth / amount;
		updateMatrices();
		return oldWidth;
	}
	double setAspectRatio(double newAspectRatio) {
		double oldAspect = aspectRatio;
		aspectRatio = newAspectRatio;
		updateMatrices();
		return oldAspect;
	}
	glm::mat4 getViewMatrix() {
		return viewMatrix;
	}
	glm::mat4 getProjectionMatrix() {
		return projectionMatrix;
	}
	glm::vec3 getPosition() {
		return position;
	}
	double getAspectRatio() {
		return aspectRatio;
	}
	double getDisplayWidth() {
		return viewWidth;
	}
private:
	void updateMatrices() {
		double viewHeight = viewWidth*aspectRatio;

		projectionMatrix = glm::ortho(float(-viewWidth/2),float(viewWidth/2), float(-viewHeight/2),float(viewHeight/2), 0.001f, 1000.0f);//glm::perspective(glm::radians(50.0f), (float)mapWindowWidth / (float)mapWindowHeight, 0.1f, 1000.0f); //
		viewMatrix = glm::lookAt(position, glm::vec3(position.x, position.y, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
};
#endif