#include <cstdio>
#include <clocale>
#include <cstring>
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#pragma comment(lib, "glew32.lib")

const GLint WIDTH = 720, HEIGHT = 480;

GLuint EBO;
GLuint VAO, VBO, shader;

static const char* vShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos; // 위치 변수는 attribute position 0 을 가짐 
layout (location = 1) in vec3 aColor; // 컬러 변수는 attribute position 1 을 가짐 

out vec3 ourColor; // 컬러를 fragment shader 로 출력

void main()
{
	// 입력값을 처리하고 그래픽 작업을 함
	gl_Position = vec4(aPos, 1.0); //vec4 의 생성자에 vec3 를 직접적으로 줌

	// 처리된 것을 출력 변수로 출력
	ourColor = aColor // 출력 변수에 짙은 빨간색을 설정
}
)";


static const char* fShader = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;

void main()
{
FragColor = vec4(outColor, 1.0);
}
)";



static const char* verShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos; // 위치 변수는 attribute position 0 을 가짐 

out vec4 vertexColor; // fragment shader 를 위한 컬러 출력을 지정 

void main()
{
	// 입력값을 처리하고 그래픽 작업을 함
	gl_Position = vec4(aPos, 1.0); //vec4 의 생성자에 vec3 를 직접적으로 줌

	// 처리된 것을 출력 변수로 출력
	vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // 출력 변수에 짙은 빨간색을 설정
}
)";

static const char* frgShader = R"(
#version 330 core
out vec4 FragColor; 

//in vec4 vertexColor; // Vertex Shader 로 부터 받은 입력 변수 (똑같은 이름, 똑같은 타입)
uniform vec4 outColor; // OpenGL 코드에서 이 변수를 설정할 것임
void main()
{
	FragColor = outColor;
}
)";


void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	// 쉐이더 생성
	GLuint theShader = glCreateShader(shaderType);

	// 쉐이더 코드를 저장할 배열 생성
	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	// 쉐이더 코드 길이를 저장할 배열 생성
	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	// 쉐이더에 우리가 작성한 쉐이더 코드를 저장한다.
	glShaderSource(theShader, 1, theCode, codeLength);
	// 쉐이더 컴파일
	glCompileShader(theShader);

	// 에러 검출을 위한 변수 선언
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	// 쉐이더 컴파일 정상완료 여부 저장
	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		// 쉐이더 오류 로그를 저장하고 출력합니다.
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error Compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	// 쉐이더 프로그램에 쉐이더를 등록합니다.
	glAttachShader(theProgram, theShader);
}

void CompileShader()
{
	shader = glCreateProgram();

	if (shader == NULL)
	{
		printf("Error Creating Shader Program!\n");
		return;
	}

	AddShader(shader, vShader, GL_VERTEX_SHADER);
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	// 쉐이더 프로그램 연결
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error Linking Program: '%s'\n", eLog);
		return;
	}

	// 쉐이더 프로그램 검증
	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error Validating Program: '%s'\n", eLog);
		return;
	}
}

void CreateTriangle()
{
	GLfloat vertices[] = {
	 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // 우측 상단
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f // 우측 하단
	-0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 1.0f // 좌측 하단
	//-0.5f,  0.5f, 0.0f   // 좌측 상단
	};
	/*
	unsigned int indices[] = {  // 0부터 시작한다는 것을 명심하세요!
		0, 1, 3,   // 첫 번째 삼각형
		1, 2, 3    // 두 번째 삼각형
	};*/


	// OpenGL 정점 배열 생성기를 사용해서 VAO를 생성
	glGenVertexArrays(1, &VAO);
	// 우리가 생성한 VAO를 현재 수정 가능하도록 연결한다.
	glBindVertexArray(VAO);

	// OpenGL 정점 배열 생성기를 사용해서 VBO를 생성
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// 우리가 생성한 VBO를 현재 수정 가능하도록 연결한다.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// 우리가 만든 삼각형 정점 좌표를 VBO에 저장한다.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// VAO에 이 VAO를 어떻게 해석해야 할 지 알려줍니다.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// VAO 사용 허용
	glEnableVertexAttribArray(0);
	// VBO 수정 종료 및 연결 초기화
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 수정이 완료 되면 연결을 끊기 위해 초기값으로 연결한다.
	glBindVertexArray(0);
}

int main() {
	// 로케일 국가 한국 지정
	_wsetlocale(LC_ALL, L"Korean");
	// GLFW 초기화
	if (glfwInit() == GLFW_FALSE)
	{
		wprintf(L"GLFW 초기화 실패\n");
		glfwTerminate();
		return (1);
	}

	// OpenGL 버전 지정
	// OpenGL MAJOR.MINOR 방식으로 표현
	// 이번엔 3.3을 사용한다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


	// OpenGL 코어 프로필 설정
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// OpenGL 상위호환 활성화
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// GLFW 윈도우 생성
	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL TRIANGLE", NULL, NULL);
	if (mainWindow == NULL)
	{
		wprintf(L"GLFW 윈도우 생성이 실패했습니다.\n");
		glfwTerminate();
		return (1);
	}


	// 버퍼 가로, 버퍼 세로 선언
	int bufferWidth, bufferHeight;
	// mainWindow로부터 버퍼 가로 크기와 버퍼 세로 크기를 받아온다.
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	glfwMakeContextCurrent(mainWindow);

	// GLEW의 모든 기능 활성화
		glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		wprintf(L"GLEW 초기화가 실패했습니다.\n");
		// mainWindow 삭제
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return (1);
	}

	//---------------------------------------------
	CreateTriangle();
	CompileShader();

	// OpenGL Viewport 생성
	glViewport(0, 0, bufferWidth, bufferHeight);



	// GLFW가 종료되지 않는 한 계속 도는 순환문
	while (glfwWindowShouldClose(mainWindow) == GLFW_FALSE)
	{
		// GLFW 이벤트 입력
		glfwPollEvents();

		// 연두색 화면 그리기
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// OpenGL 배경색상 초기화
		glClear(GL_COLOR_BUFFER_BIT);

		
		// Shader 적용
		glUseProgram(shader);

		//uniform 사용
		//float timeValue = glfwGetTime();
		//float greenValue = (sin(timeValue) / 2.0f + 0.5f);
		//int vertexColorLocation = glGetUniformLocation(shader, "outColor");
		//glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

		// VBO에 있는 데이터 바인딩
		glBindVertexArray(VAO);
		// 데이터를 바탕으로 그리기
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// 데이터 바인딩 해제
		glBindVertexArray(0);

		// Shader 해제
		glUseProgram(0);

		// GLFW 더블 버퍼링
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}
	return 0;
}


