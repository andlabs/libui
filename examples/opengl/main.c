// 28 may 2016

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <math.h>
#include <stdio.h>
#include "../../ui.h"

#define SLOW_ROTATION_SPEED		2.0f
#define FAST_ROTATION_SPEED		5.0f

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
	GLchar r, g, b, a;
};

typedef struct Vertex Vertex;

static const Vertex VERTICES[] = {
	{ -1.0f, -1.0f, 0.0f, 0, 255, 0, 255 },
	{ 1.0f, -1.0f, 0.0f, 0, 0, 255, 255 },
	{ 0.0f,  1.0f, 0.0f, 255, 0, 255, 255 }
};

static const char *VERTEX_SHADER =
	"attribute vec3 aPosition;\n"
	"attribute vec4 aColor;\n"
	"uniform mat4 aProjection;\n"
	"uniform mat4 aModelView;\n"
	"varying vec4 vColor;\n"
	"void main() {\n"
	"	vColor = aColor;\n"
	"   gl_Position = aProjection * aModelView * vec4(aPosition, 1.0);\n"
	"}\n";

static const char *FRAGMENT_SHADER =
	"varying vec4 vColor;\n"
	"void main() {\n"
	"   gl_FragColor = vColor;\n"
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

static ExampleOpenGLState openGLState = { 0, 0, 0, 0, 0, 0, 0, 0 };

static float rotationAngle = 0.0f;

static void onMouseEvent(uiOpenGLAreaHandler *h, uiOpenGLArea *a, uiAreaMouseEvent *e)
{
	printf("onMouseEvent\n");
	double width;
	uiOpenGLAreaGetSize(a, &width, NULL);

	rotationAngle = (uiPi * 2.0f) * (e->X / width);
	// rotationAngle += 2.0f / 180.0f * uiPi;
	uiOpenGLAreaQueueRedrawAll(a);
}

static void onMouseCrossed(uiOpenGLAreaHandler *h, uiOpenGLArea *a, int left)
{
	printf("onMouseCrossed\n");
}

static void onDragBroken(uiOpenGLAreaHandler *h, uiOpenGLArea *a)
{
	printf("onDragBroken\n");
}

static int onKeyEvent(uiOpenGLAreaHandler *h, uiOpenGLArea *a, uiAreaKeyEvent *e)
{
	printf("onKeyEvent\n");
	return 0;
}

static void onInitGL(uiOpenGLAreaHandler *h, uiOpenGLArea *a)
{
	printf("Init\n");
	GLCall(glGenBuffers(1, &openGLState.VBO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, openGLState.VBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW));

	openGLState.VertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLCall(glShaderSource(openGLState.VertexShader, 1, &VERTEX_SHADER, NULL));
	GLCall(glCompileShader(openGLState.VertexShader));

	openGLState.FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLCall(glShaderSource(openGLState.FragmentShader, 1, &FRAGMENT_SHADER, NULL));
	GLCall(glCompileShader(openGLState.FragmentShader));

	openGLState.Program = glCreateProgram();
	GLCall(glAttachShader(openGLState.Program, openGLState.VertexShader));
	GLCall(glAttachShader(openGLState.Program, openGLState.FragmentShader));
	GLCall(glLinkProgram(openGLState.Program));

	GLCall(glEnable(GL_DEPTH_TEST));

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
}

static void onDrawGL(uiOpenGLAreaHandler *h, uiOpenGLArea *a, double width, double height)
{
	GLCall(glViewport(0, 0, width, height));

	GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GLCall(glUseProgram(openGLState.Program));

	GLCall(glEnableVertexAttribArray(openGLState.PositionAttrib));
	GLCall(glEnableVertexAttribArray(openGLState.ColorAttrib));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, openGLState.VBO));
	GLCall(glVertexAttribPointer(openGLState.PositionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0));
	GLCall(glVertexAttribPointer(openGLState.ColorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, r)));

	Matrix4 projection = perspective(45.0f / 180.0f * uiPi,
									 (float)width / (float)height,
									 0.1f,
									 100.0f);
	GLCall(glUniformMatrix4fv(openGLState.ProjectionUniform, 1, GL_FALSE, &projection.m11));
	Matrix4 modelview = rotate(rotationAngle, 0.0f, 1.0f, 0.0f);
	modelview.m43 -= 5.0f;
	GLCall(glUniformMatrix4fv(openGLState.ModelViewUniform, 1, GL_FALSE, &modelview.m11));

	GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	uiOpenGLAreaSwapBuffers(a);
	printf("drawing\n");
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
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(shouldQuit, mainwin);

	uiOpenGLAttributes *attribs = uiNewOpenGLAttributes();
	uiOpenGLAttributesSetAttribute(attribs, uiOpenGLAttributeMajorVersion, 2);
	uiOpenGLAttributesSetAttribute(attribs, uiOpenGLAttributeMinorVersion, 0);

	uiBox *b = uiNewHorizontalBox();
	uiWindowSetChild(mainwin, uiControl(b));

	uiOpenGLArea *glarea = uiNewOpenGLArea(&AREA_HANDLER, attribs);
	uiBoxAppend(b, uiControl(glarea), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();

	uiFreeOpenGLAttributes(attribs);

	uiUninit();
	return 0;
}

