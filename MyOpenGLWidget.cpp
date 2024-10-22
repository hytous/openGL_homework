#include "MyOpenGLWidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)  // 类的作用域解析运算符::构造函数的名称  C++ 中，构造函数的名称必须与类的名称完全相同
    : QOpenGLWidget(parent), shaderProgram(nullptr)  // 初始化 shaderProgram 指针
{   /*初始化列表
    上面“: ”后面的东西（即QOpenGLWidget(parent), shaderProgram(nullptr)）叫初始化列表，
    是在构造函数体执行之前初始化的成员变量，初始化列表中的成员变量会被直接初始化，而不是在构造函数体内赋值。

    若在构造体内初始化（例如 shaderProgram = nullptr;），这实际上是先构造 shaderProgram，然后再进行赋值。
    在构造体内初始化会导致成员变量先被构造为其默认值（例如，对于指针来说是未定义的），然后再赋值为 nullptr。

    使用初始化列表来初始化成员变量通常更高效

    如果你的类成员是常量（const）或者引用类型（&），
    你必须在初始化列表中初始化它们，因为它们不能在构造体内被赋值。
    */
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    if (shaderProgram) {
        delete shaderProgram;  // 释放着色器资源
    }
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // 红，绿，蓝，透明度

    GLfloat vertices[] = {
        // 位置          // 纹理坐标  // 法线
        0.0f,  0.5f, 0.0f,  0.5f, 1.0f,   0.0f, 0.0f, 1.0f,   // 顶点 1
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   // 顶点 2
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f    // 顶点 3
    };

    // Create VAO (Vertex Array Object)
    vao.create();
    vao.bind();  // 绑定 VAO，所有后续顶点属性配置都会记录在这个 VAO 中
    // glBindVertexArray(vao)与vao.bind()在功能上是等价的，
    //glBindVertexArray(vao)是opengl的原生库
    //vao.bind()是qt中对opengl封装过的高级库

    // Create VBO (Vertex Buffer Object)
    vbo.create();
    vbo.bind();  // 绑定 VBO
    vbo.allocate(vertices, sizeof(vertices));  // 在vbo中复制一份顶点

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);  // 顶点位置
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));  // 纹理坐标
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));  // 纹理坐标
    glEnableVertexAttribArray(2);

    // 解绑 VBO 和 VAO
    vbo.release();
    vao.release();

    // 编译和链接着色器程序
    shaderProgram = new QOpenGLShaderProgram();

    /*着色器工作流程概述：
        1.顶点着色器：处理图形的几何形状，确定每个顶点的最终位置，并将一些信息传递给片段着色器（比如颜色、纹理坐标等）。
        2.光栅化：OpenGL 将顶点信息转换为片段（像素）。在这一阶段，生成的片段会被送入片段着色器。
        3.片段着色器：每个片段（像素）都会通过片段着色器计算出最终的颜色，out vec4 fragColor; 就是用来输出这个颜色的变量。
        4.写入帧缓冲：片段着色器的输出会被写入帧缓冲区（Frame Buffer），OpenGL 决定如何将这些颜色显示到屏幕上。
     */

    // 顶点着色器（传递纹理坐标
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
    #version 460 core
    layout(location = 0) in vec3 position;  // 顶点位置
    layout(location = 1) in vec2 texCoord;  // 纹理坐标
    layout(location = 2) in vec3 normal;    // 法线

    out vec2 TexCoord;  // 向片段着色器传递纹理坐标
    out vec3 FragPos;   // 向片段着色器传递片段位置
    out vec3 Normal;    // 向片段着色器传递法线

    uniform mat4 model; // 模型矩阵
    uniform mat4 view;  // 视图矩阵
    uniform mat4 projection; // 投影矩阵

    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);  // 设置顶点位置
        TexCoord = texCoord;  // 传递纹理坐标
        FragPos = vec3(model * vec4(position, 1.0)); // 计算片段位置
        Normal = mat3(transpose(inverse(model))) * normal; // 计算法线
    }
    )");


    // 从纹理中采样颜色
    /*uniform sampler2D texture1;
     是一个常用的 GLSL（OpenGL Shading Language）声明，
     用于在着色器中定义一个二维纹理采样器。它不是固定语句，但在实际使用中非常普遍。以下是这个声明的详细说明：
    uniform：表示这个变量的值在顶点着色器和片段着色器之间是共享的，并且在每次绘制调用中不会改变。这意味着它的值在整个渲染过程中是固定的。
    sampler2D：这是 GLSL 中的一种特定类型，用于表示二维纹理采样器。它可以用来从绑定到纹理单元的纹理中读取颜色值。
    texture1：这是变量的名称，你可以根据需要自定义这个名字。在 GLSL 中，变量名可以是任意有效的标识符，但为了可读性，通常会根据用途进行命名。
     */
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
    #version 460 core
    out vec4 fragColor;  // 输出的片段颜色

    in vec2 TexCoord;   // 从顶点着色器传递的纹理坐标
    in vec3 FragPos;    // 从顶点着色器传递的片段位置
    in vec3 Normal;     // 从顶点着色器传递的法线

    uniform vec3 lightPos; // 光源位置
    uniform vec3 viewPos;  // 观察者位置
    uniform vec3 lightColor; // 光源颜色
    uniform vec3 objectColor; // 物体颜色

    void main() {
        // 环境光
        float ambientStrength = 0.1;  // 0.1
        vec3 ambient = ambientStrength * lightColor;

        // 漫反射
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // 镜面光
        float specularStrength = 0.5;  // 0.5
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        // 计算最终颜色
        vec3 result = (ambient + diffuse + specular) * objectColor;
        fragColor = vec4(result, 1.0);
    }
    )");


    shaderProgram->link();  // 链接着色器
    // shaderProgram->bind();  // 绑定着色器程序
    // 设置光源和物体颜色
    shaderProgram->bind();
    // shaderProgram->setUniformValue("lightPos", QVector3D(0.0f, 0.0f, 0.0f)); // 光源位置
    shaderProgram->setUniformValue("lightPos", QVector3D(1.0f, 1.0f, 1.0f)); // 光源位置
    shaderProgram->setUniformValue("viewPos", QVector3D(0.0f, 0.0f, 3.0f));  // 观察者位置 0，0，3
    shaderProgram->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f)); // 光源颜色
    shaderProgram->setUniformValue("objectColor", QVector3D(1.0f, 0.5f, 0.31f)); // 物体颜色 橙色
    // shaderProgram->setUniformValue("objectColor", QVector3D(1.0f, 1.0f, 1.0f)); // 物体颜色 白色
    shaderProgram->release();

    // 加载纹理图像
    QImage textureImage(":/textures/003.png");  // 替换为你的纹理图片路径
    textureImage = textureImage.convertToFormat(QImage::Format_RGBA8888);  // 将图像转换为 RGBA 格式

    // 创建纹理对象
    // GLuint texture;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);  // GL_TEXTURE_2D表示正在使用二维纹理

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 将图像数据传递给纹理对象
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.width(), textureImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);  // 解绑纹理
}

void MyOpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MyOpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 计算模型矩阵
    QMatrix4x4 model;  // 模型矩阵通常用于描述物体在场景中的位置、旋转和缩放
    model.setToIdentity(); // 设置为单位矩阵（可根据需要进行变换）物体坐标系就是世界坐标系
    // model.translate(0.5f, 0.0f, 0.0f);  // 图片向右了

    // 计算视图矩阵
    QMatrix4x4 view;
    view.setToIdentity();  // 将视图矩阵初始化为单位矩阵。
    view.translate(0.0f, 0.0f, -3.0f); // 移动相机在z轴上平移-3
    view.translate(0.0f, 0.0f, 1.0f); // 移动相机在z轴上平移1,平移是累加的

    // 计算投影矩阵
    QMatrix4x4 projection;
    projection.setToIdentity();
    // 视野角（45.0f）通常以度为单位,角度越大，视场越宽
    // 宽高比（width() / (float)height()）用于确定投影的纵横比，确保图像不失真。
    // 近裁剪面（0.1f）离相机最近的可见距离，所有在此距离以内的物体不会被渲染。
    // 远裁剪面（100.0f）离相机最远的可见距离，超出此范围的物体也不会被渲染。
    projection.perspective(45.0f, 1.0f / 1.0f, 1.0f, 100.0f); // 透视投影
    // projection.perspective(45.0f, width() / (float)height(), 0.1f, 100.0f); // 透视投影

    // 绑定着色器程序
    shaderProgram->bind();
    shaderProgram->setUniformValue("model", model);
    shaderProgram->setUniformValue("view", view);
    shaderProgram->setUniformValue("projection", projection);
    shaderProgram->setUniformValue("lightPos", QVector3D(1.0f, 1.0f, 2.0f)); // 光源位置
    shaderProgram->setUniformValue("viewPos", QVector3D(0.0f, 0.0f, 3.0f)); // 观察者位置  // 0，0，3

    glBindTexture(GL_TEXTURE_2D, texture);  // 绑定纹理
    // 绑定vao
    vao.bind();

    // 画三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 解绑
    vao.release();
    glBindTexture(GL_TEXTURE_2D, 0);  // 解绑纹理
    shaderProgram->release();  // 释放着色器程序
}
