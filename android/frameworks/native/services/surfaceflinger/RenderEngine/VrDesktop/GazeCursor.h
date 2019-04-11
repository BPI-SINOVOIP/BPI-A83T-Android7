#ifndef _VRDESKTOP_GAZECURSOR
#define _VRDESKTOP_GAZECURSOR

#include <ui/mat4.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace android;

class GazeCursor{
public:
	GazeCursor(float r);
	~GazeCursor();

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

