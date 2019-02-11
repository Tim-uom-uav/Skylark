#ifndef MISSION_CONTROL_STD_HEADERS_H_
#define MISSION_CONTROL_STD_HEADERS_H_
#define GLM_ENABLE_EXPERIMENTAL //ONLY FOR STRING_CAST
#define _SCL_SECURE_NO_WARNINGS //Because MVC is dumb
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <SOIL.h>
#include <Windows.h>
#include <ctime>
#include "Util.h"
#include "Class_Glider.h"
#include "Class_ADSBaircraft.h"

// settings
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
extern unsigned int DISPLAY_WIDTH;
extern unsigned int DISPLAY_HEIGHT;

#endif //MISSION_CONTROL_STD_HEADERS_H_
