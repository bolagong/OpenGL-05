//
//  main.cpp
//  LearnOpenGL
//
//  Created by changbo on 2019/10/12.
//  Copyright © 2019 CB. All rights reserved.
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLFrame.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

// 着色器管理器
GLShaderManager         shaderManager;

// 模型视图矩阵
GLMatrixStack           modelViewMatrix;

// 投影矩阵
GLMatrixStack           projectionMatrix;

// 视景体
GLFrustum               viewFrustum;

// 几何变换管线
GLGeometryTransform     transformPipeline;


// 4个批次容器类
GLBatch     floorBatch;  // 地面
GLBatch     ceilingBatch; // 天花板
GLBatch     leftWallBatch; // 左墙面
GLBatch     rightWallBatch; // 右墙面


// 初始深度值-65（往屏幕里面）
GLfloat     viewZ = -65.0f;


// 纹理标识符号
#define TEXTURE_BRICK       0  // 墙面
#define TEXTURE_FLOOR       1  // 地板
#define TEXTURE_CEILING     2  // 天花板
// 纹理个数
#define TEXTURE_COUNT       3
// 纹理标记数组
GLuint textures[TEXTURE_COUNT];


// tga文件名字数组
const char * szTextureFiles[TEXTURE_COUNT] = {"brick.tga","floor.tga","ceiling.tga"};




// 菜单栏选择
void ProcessMenu(int value) {
    // 4个纹理
    GLint iLoop;
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        // 1.绑定纹理
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        
        // 2.配置纹理相关参数
        switch (value) {
            case 0:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                break;
            case 1:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                break;
            case 2:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                break;
            case 3:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                break;
            case 4:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                break;
            case 5:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
            case 6:
                // 设置各项异性过滤
                GLfloat fLargest;
                // 获取各项异性过滤的最大数量
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
                // 设置纹理参数
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
                break;
            case 7:
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
                break;
            default:
                break;
        }
    }
    
    glutPostRedisplay();
}


//为程序作一次性的设置
void SetupRC()
{
    GLbyte *pBytes;
    GLint iWidth, iHeight, iComponets;
    GLenum eFormat;
    GLint iLoop;
    
    //设置背影颜色
    glClearColor(0.67f,1.0f,0.83f,1.0f);
    //初始化着色管理器
    shaderManager.InitializeStockShaders();
    
    // 1.加载纹理
    glGenTextures(TEXTURE_COUNT, textures);
    
    // 2.循环加载纹理文件
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        
        // 3.绑定纹理
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        
        // 4.加载TGA文件
        pBytes = gltReadTGABits(szTextureFiles[iLoop], &iWidth, &iHeight, &iComponets, &eFormat);
        
        // 5.配置纹理属性
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // 6.载入纹理
        glTexImage2D(GL_TEXTURE_2D, 0, iComponets, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        
        // 7.生成map贴图
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // 释放
        free(pBytes);
    }
    
    // 地板
    // 建立几何图形(设置 28个顶点，1组纹理)
    floorBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    // 循环创建纹理/图形坐标
    GLfloat z = 0;
    for (z = 60.0f; z >= 0.0f; z -= 10.0f) {
        
        // 纹理坐标
        floorBatch.MultiTexCoord2f(0, 0, 0);
        // 图形坐标
        floorBatch.Vertex3f(-10.0f, -10.0f, z);
        
        floorBatch.MultiTexCoord2f(0, 1, 0);
        floorBatch.Vertex3f(10.0f, -10.0f, z);
        
        floorBatch.MultiTexCoord2f(0, 0, 1);
        floorBatch.Vertex3f(-10.0f, -10.0f, z-10.0f);
        
        floorBatch.MultiTexCoord2f(0, 1, 1);
        floorBatch.Vertex3f(10.0f, -10.0f, z-10.0f);
    }
    
    floorBatch.End();
    
    
    // 天花板
    ceilingBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0.0f; z -= 10.0f) {
        
        ceilingBatch.MultiTexCoord2f(0, 0, 1);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z-10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z-10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 0, 0);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 0);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z);
    }
    ceilingBatch.End();
    
    // 左墙面
    leftWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0.0f; z -= 10.0f) {
        
        leftWallBatch.MultiTexCoord2f(0, 0, 0);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 0, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 0);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z-10.0f);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z-10.0f);
    }
    leftWallBatch.End();
    
    // 右墙面
    rightWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0.0f; z -= 10.0f) {
        
        rightWallBatch.MultiTexCoord2f(0, 0, 0);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 0, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 0);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z-10.0f);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z-10.0f);
    }
    rightWallBatch.End();
}


//窗口大小改变时接受新的宽度和高度，其中0,0代表窗口中视口的左下角坐标，w，h代表像素
void ChangeSize(int w,int h)
{
    if (h == 0) {
        h = 1;
    }
    glViewport(0,0, w, h);
    
    // 设置投影方式
    viewFrustum.SetPerspective(80.0, float(w)/float(h), 1.0f, 120.0f); // 透视投影
    // 正投影 viewFrustum.SetOrthographic(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}


void ShutdownRC(void) {
    // 删除纹理
    glDeleteTextures(TEXTURE_COUNT, textures);
}

void SpecialKeys(int key, int x, int y) {
    // 上下键来移动隧道中的位置
    // 通过修改 viewZ 深度值来进行平移
    if (key == GLUT_KEY_UP) {
        viewZ += 0.5f;
    }
    
    if (key == GLUT_KEY_DOWN) {
        viewZ -= 0.5f;
    }
    
    // 重新渲染
    glutPostRedisplay();
}

//开始渲染
void RenderScene(void)
{
    // 清除缓冲区
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    // 模型视图压栈
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translate(0.0f, 0.0f, viewZ);
    
    // 参数1: GLT_SHADER_TEXTURE_REPLACE 着色器标签
    // 参数2: 模型视图矩阵
    // 参数3: 纹理层
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 0);
    
    // 绑定纹理
    // 参数1: 纹理模式 GL_TEXTURE_1D，GL_TEXTURE_2D，GL_TEXTURE_3D
    // 参数2: 需要绑定的纹理
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_FLOOR]);
    floorBatch.Draw();
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_CEILING]);
    ceilingBatch.Draw();
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BRICK]);
    leftWallBatch.Draw();
    rightWallBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
}

int main(int argc,char* argv[])
{
    //设置当前工作目录，针对MAC OS X
    gltSetWorkingDirectory(argv[0]);
    //初始化GLUT库
    glutInit(&argc, argv);
    // 初始化双缓冲窗口 (双缓冲窗口、RGB颜色模式)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    //GLUT窗口大小，标题窗口
    glutInitWindowSize(800,500);
    glutCreateWindow("Tunnel");
    //注册回调函数
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    
    
    // 添加菜单入口，改变过滤器
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("GL_NEAREST", 0);
    glutAddMenuEntry("GL_LINEAR", 1);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", 3);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 4);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", 5);
    glutAddMenuEntry("Anisotropic Filter", 6);
    glutAddMenuEntry("Anisotropic off", 7);
    
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    
    //驱动程序的初始化中没有出现任何问题。
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        return 1;
    }
    
    // 启动循环，关闭纹理
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    
    return 0;
}

