// 28 may 2016

#include "glew/glew.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../ui.h"

#define GLCall(x) GLClearError(); x; GLLogCall(#x, __FILE__, __LINE__);

static void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

static void GLLogCall(const char *function, const char *file, int line) {
	GLenum error;
	while((error = glGetError())) {
		printf("OpenGL Error %s (%s:%d) - 0x%x\n", function, file, line, error);
	}
}

struct Vertex {
	GLfloat x, y, z;
	GLubyte r, g, b, a;
};

typedef struct Vertex Vertex;

static const Vertex VERTICES[] = {
	{ -1.0f, -1.0f, 0.0f, 0, 255, 0, 255 },
	{ 1.0f, -1.0f, 0.0f, 0, 0, 255, 255 },
	{ 0.0f,  1.0f, 0.0f, 255, 0, 255, 255 }
};

static const char *VERTEX_SHADER =
	"#version 330 core\n"
	"layout(location=0) in vec3 aPosition;\n"
	"layout(location=1) in vec4 aColor;\n"
	"uniform mat4 aProjection;\n"
	"uniform mat4 aModelView;\n"
	"out vec4 vColor;\n"
	"void main() {\n"
	"	vColor = aColor;\n"
	"   gl_Position = aProjection * aModelView * vec4(aPosition, 1.0);\n"
	"}\n";

static const char *FRAGMENT_SHADER =
	"#version 330 core\n"
	"in vec4 vColor;\n"
	"out vec4 fColor;\n"
	"void main() {\n"
	"   fColor = vColor;\n"
	"}\n";

struct Matrix4 {
	GLfloat m11, m12, m13, m14;
	GLfloat m21, m22, m23, m24;
	GLfloat m31, m32, m33, m34;
	GLfloat m41, m42, m43, m44;
};

typedef struct Matrix4 Matrix4;

struct ExampleOpenGLState {
	GLuint VBO;
	GLuint VAO;
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Program;
	GLuint ProjectionUniform;
	GLuint ModelViewUniform;
	GLuint PositionAttrib;
	GLuint ColorAttrib;
};

typedef struct ExampleOpenGLState ExampleOpenGLState;

static Matrix4 perspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar)
{
	GLfloat f = 1.0f / tanf(fovy / 2.0f);
	Matrix4 result = {
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, (zfar + znear) / (znear - zfar), -1.0f,
		0.0f, 0.0f, (2.0 * zfar * znear) / (znear - zfar), 0.0f
	};
	return result;
}

// NB: (x, y, z) must be normalized.
static Matrix4 rotate(GLfloat theta, GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat c = cosf(theta), s = sinf(theta);
	GLfloat ci = 1.0f - c;
	Matrix4 result = {
		x*x*ci + c, y*x*ci + z*s, x*z*ci - y*s, 0.0,
		x*y*ci - z*s, y*y*ci + c, y*z*ci + x*s, 0.0,
		x*z*ci + y*s, y*z*ci - x*s, z*z*ci + c, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	return result;
}

static Matrix4 scale(GLfloat x, GLfloat y, GLfloat z)
{
	Matrix4 result = {
		  x, 0.0, 0.0, 0.0,
		0.0,   y, 0.0, 0.0,
		0.0, 0.0,   z, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	return result;
}

static Matrix4 multiply(Matrix4 a, Matrix4 b) {
	Matrix4 result = {
		a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31 + a.m14 * b.m41,
		a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32 + a.m14 * b.m42,
		a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33 + a.m14 * b.m43,
		a.m11 * b.m14 + a.m12 * b.m24 + a.m13 * b.m34 + a.m14 * b.m44,

		a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31 + a.m24 * b.m41,
		a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32 + a.m24 * b.m42,
		a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33 + a.m24 * b.m43,
		a.m21 * b.m14 + a.m22 * b.m24 + a.m23 * b.m34 + a.m24 * b.m44,

		a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31 + a.m34 * b.m41,
		a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32 + a.m34 * b.m42,
		a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33 + a.m34 * b.m43,
		a.m31 * b.m14 + a.m32 * b.m24 + a.m33 * b.m34 + a.m34 * b.m44,

		a.m41 * b.m11 + a.m42 * b.m21 + a.m43 * b.m31 + a.m44 * b.m41,
		a.m41 * b.m12 + a.m42 * b.m22 + a.m43 * b.m32 + a.m44 * b.m42,
		a.m41 * b.m13 + a.m42 * b.m23 + a.m43 * b.m33 + a.m44 * b.m43,
		a.m41 * b.m14 + a.m42 * b.m24 + a.m43 * b.m34 + a.m44 * b.m44
	};
	return result;
}

static ExampleOpenGLState openGLState = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static float rotationAngleA = 0.0f;
static float rotationAngleB = 0.0f;
static int curScale = 10;
static int mouseInWindow = 0;

static void onMouseEvent(uiOpenGLAreaHandler *h, uiOpenGLArea *a, uiAreaMouseEvent *e)
{
	double width = e->AreaWidth, height = e->AreaHeight;

	rotationAngleA = (uiPi * 2.0f) * (e->X / width);
	rotationAngleB = (uiPi * 2.0f) * (0.5 + e->Y / height);
	// rotationAngleX += 2.0f / 180.0f * uiPi;
	uiOpenGLAreaQueueRedrawAll(a);
}

static void onMouseCrossed(uiOpenGLAreaHandler *h, uiOpenGLArea *a, int left)
{
	mouseInWindow = !left;
}

static void onDragBroken(uiOpenGLAreaHandler *h, uiOpenGLArea *a)
{
}

static int onKeyEvent(uiOpenGLAreaHandler *h, uiOpenGLArea *a, uiAreaKeyEvent *e)
{
	char key = e->Key;
	if(48 <= key && key <= 57){
		if(key == 48)
			curScale = 10;
		else
			curScale = key - 48;

		uiOpenGLAreaQueueRedrawAll(a);
		return 1;
	}
	return 0;
}

static void compileShader(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE){
		glCompileShader(shader);
		char log[1024];
		GLsizei length = 0;
		glGetShaderInfoLog(shader, 1024, &length, log);
		if(length > 0)
			printf("%s\n", log);
	}
}

static void linkProgram(GLuint program)
{
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE){
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		if(maxLength > 0){
			char log[maxLength];
			glGetProgramInfoLog(program, maxLength, &maxLength, log);
			printf("%s\n", log);
			glDeleteProgram(program);
			exit(EXIT_FAILURE);
		}
	}
}

static void onInitGL(uiOpenGLAreaHandler *h, uiOpenGLArea *a)
{
	printf("Init\n");

	GLCall(glGenVertexArrays(1, &openGLState.VAO));
	GLCall(glBindVertexArray(openGLState.VAO));

	GLCall(glGenBuffers(1, &openGLState.VBO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, openGLState.VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW));

	openGLState.VertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLCall(glShaderSource(openGLState.VertexShader, 1, &VERTEX_SHADER, NULL));
	compileShader(openGLState.VertexShader);

	openGLState.FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLCall(glShaderSource(openGLState.FragmentShader, 1, &FRAGMENT_SHADER, NULL));
	compileShader(openGLState.FragmentShader);

	openGLState.Program = glCreateProgram();
	GLCall(glAttachShader(openGLState.Program, openGLState.VertexShader));
	GLCall(glAttachShader(openGLState.Program, openGLState.FragmentShader));
	linkProgram(openGLState.Program);
	GLCall(glUseProgram(openGLState.Program));
	GLCall(glDetachShader(openGLState.Program, openGLState.VertexShader));
	GLCall(glDetachShader(openGLState.Program, openGLState.FragmentShader));

	GLClearError();
	openGLState.ProjectionUniform = glGetUniformLocation(openGLState.Program, "aProjection");
	GLLogCall("glGetUniformLocation(openGLState.Program, 'aProjection')", __FILE__, __LINE__);
	GLClearError();
	openGLState.ModelViewUniform = glGetUniformLocation(openGLState.Program, "aModelView");
	GLLogCall("glGetUniformLocation(openGLState.Program, 'aModelView')", __FILE__, __LINE__);

	GLClearError();
	openGLState.PositionAttrib = glGetAttribLocation(openGLState.Program, "aPosition");
	GLLogCall("glGetAttribLocation(openGLState.Program, 'aPosition');", __FILE__, __LINE__);
	GLClearError();
	openGLState.ColorAttrib = glGetAttribLocation(openGLState.Program, "aColor");
	GLLogCall("glGetAttribLocation(openGLState.Program, 'aColor');", __FILE__, __LINE__);

	GLCall(glVertexAttribPointer(openGLState.PositionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0));
	GLCall(glEnableVertexAttribArray(openGLState.PositionAttrib));

	GLCall(glVertexAttribPointer(openGLState.ColorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, r)));
	GLCall(glEnableVertexAttribArray(openGLState.ColorAttrib));
}

static void onDrawGL(uiOpenGLAreaHandler *h, uiOpenGLArea *a, double width, double height)
{
	if(!mouseInWindow)
		rotationAngleA += 0.05f;

	GLCall(glViewport(0, 0, width, height));

	GLCall(glClear(GL_COLOR_BUFFER_BIT));

	GLCall(glBindVertexArray(openGLState.VAO));

	Matrix4 projection = perspective(45.0f / 180.0f * uiPi,
									 (float)width / (float)height,
									 0.1f,
									 100.0f);
	GLCall(glUniformMatrix4fv(openGLState.ProjectionUniform, 1, GL_FALSE, &projection.m11));
	Matrix4 modelview = multiply(
		rotate(rotationAngleA, 0.0f, 1.0f, 0.0f),
		multiply(
			rotate(rotationAngleB, 1.0f, 0.0f, 0.0f),
			scale(curScale/10.0f, curScale/10.0f, curScale/10.0f)
		)
	);
	modelview.m43 -= 5.0f;
	GLCall(glUniformMatrix4fv(openGLState.ModelViewUniform, 1, GL_FALSE, &modelview.m11));

	GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	uiOpenGLAreaSwapBuffers(a);
}

static uiOpenGLAreaHandler AREA_HANDLER = {
	onDrawGL,
	onMouseEvent,
	onMouseCrossed,
	onDragBroken,
	onKeyEvent,
	onInitGL
};

static int onClosing(uiWindow *w, void *data)
{
	uiControlDestroy(uiControl(w));
	uiQuit();
	return 0;
}

static int shouldQuit(void *data)
{
	uiControlDestroy((uiControl *)data);
	return 1;
}

static int render(void *d)
{
	uiOpenGLArea *area = d;
	if(!mouseInWindow)
		uiOpenGLAreaQueueRedrawAll(area);
	return 1;
}

int main(void)
{
	uiInitOptions o = { 0 };
	const char *err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	uiWindow *mainwin = uiNewWindow("libui OpenGL Example", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(shouldQuit, mainwin);

	uiOpenGLAttributes *attribs = uiNewOpenGLAttributes();
	uiOpenGLAttributesSetAttribute(attribs, uiOpenGLAttributeMajorVersion, 3);
	uiOpenGLAttributesSetAttribute(attribs, uiOpenGLAttributeMinorVersion, 0);
	uiOpenGLAttributesSetAttribute(attribs, uiOpenGLAttributeCompatProfile, 0);

	uiBox *b = uiNewVerticalBox();
	uiBoxSetPadded(b, 1);
	uiWindowSetChild(mainwin, uiControl(b));

	uiBoxAppend(b, uiControl(uiNewLabel("Press keys 0-9 to set scale")), 0);
	uiOpenGLArea *glarea = uiNewOpenGLArea(&AREA_HANDLER, attribs);
	uiBoxAppend(b, uiControl(glarea), 1);
	uiOpenGLAreaSetVSync(glarea, 1);
	uiOpenGLAreaMakeCurrent(glarea);

	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		fprintf(stderr, "GLEW init error: %s\n", glewGetErrorString(glewStatus));
		exit(EXIT_FAILURE);
	}

	uiTimer(1000/60, render, glarea);

	uiControlShow(uiControl(mainwin));
	uiMain();

	uiFreeOpenGLAttributes(attribs);

	uiUninit();
	return 0;
}
