#ifndef _VRDESKTOP_SPHERE
#define _VRDESKTOP_SPHERE

#include <ui/mat4.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace android;

class Sphere{
public:
	Sphere(float r);
	~Sphere();

	void create();
	void drawSelf(mat4 mvp);
	
private:
	//private method


	//private variable
	mat4 mMVPMatrix;
	GLint mProgram;
	GLuint mTexture;
	GLint mMVPMatrixHandle;
	GLint mPositionHandle;
	GLint mTextureHandle;
	
	float* mVertexData;
	float* mTexcoordData;
	unsigned short* mIndexData;
	int mIndexCount;
	float mRadius;
	
	void initTexture();
	float toRadians(float degree);
	void initVertexData(float r);
	GLuint buildShader(const char* source, GLenum type);
	void createProgram(const char* vertex, const char* fragment);
};

#endif