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
    /*initializeGL作用：

    initializeGL() 是在 OpenGL 上下文（OpenGL context）被创建并准备好之后调用的函数。
    它通常用于初始化OpenGL 资源和状态，例如加载着色器、创建 VBO（顶点缓冲对象）或 VAO（顶点数组
    对象）、设置清除颜色等。这些操作只需要执行一次，在整个绘制过程中不会频繁变化。

    常见用途：
    初始化 OpenGL 资源，例如创建和绑定 VAO/VBO。
    加载着色器程序。
    设置 OpenGL 状态，例如深度测试、混合模式等。
    设置初始清除颜色，如 glClearColor()。
     */
    initializeOpenGLFunctions();
    // 设置颜色缓冲区颜色   设置---》缓冲区---》屏幕，如果没有缓冲区而直接往屏幕上写，图像刷新时颜色会闪
    // 这里将各个颜色设为0，透明度设为1
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // 红，绿，蓝，透明度

    // Vertex data for a simple triangle
    GLfloat vertices[] = {
        // 位置          // 纹理坐标
        0.0f,  0.5f, 0.0f,  0.5f, 1.0f,   // 顶点 1
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,   // 顶点 2
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f    // 顶点 3
    };

    /* VAO对象是什么情况：
     * vao.create() 和 vbo.create() 这两个函数创建了VAO对象vao以及VBO对象vbo
     * 它们实际上是在 GPU 上为 VAO 和 VBO 分配资源
     *
     * 可以在创建了一个 VAO 后再次创建其他 VAO 对象。
     * 例如，在执行 vao.create() 之后，可以再调用 vao2.create() 来创建一个新的 VAO 对象
     * 每个 VAO 都有自己独立的状态，它们不会相互影响
     */

    /* VAO是用来干什么的：
     * 在vao中存储的是很多不同的属性（存在方式/存储格式），
     * 这些属性可以是一个点集的位置的存在方式/存储格式（属性）也可以是点集的颜色的存在方式/存储格式（属性）。
     * 这些属性由glVertexAttribPointer函数来创建/管理，
     * 创建新属性时要给新属性设置一个唯一的index来标识它。
     */

    /* VAO的使用方法/注意事项
     * ######也就是说每个vao对象都是一个属性库######
     * 只能同时生效一个 VAO对象
     *
     * 在代码中使用vao.bind()表示现在使用vao中的属性，vao.release()表示现在不用这个属性库了
     * vao.bind()之后，后面的代码就不用显式的调用vao了（不用往参数里面写）
     * 如：
        // 绑定 vao
        vao.bind();  // OpenGL 现在将使用 vao 记录的顶点属性
        glDrawArrays(GL_TRIANGLES, 0, numVertices);  // 使用 vao1 中的数据进行绘制
        vao.release();  // 解绑 vao
        // 绑定 vao2
        vao2.bind();  // OpenGL 现在将使用 vao2 记录的顶点属性
        glDrawArrays(GL_TRIANGLES, 0, numVertices);  // 使用 vao2 中的数据进行绘制
        vao2.release();  // 解绑 vao2
        // 其中glDrawArrays就不用指出用vao还是vao2。
     *
     * 注意切换VAO对象时要先release再bind，
     * 不过当你调用 vao1.bind() 时，即使没有先执行 vao.release() 来解绑先前的 VAO，
     * OpenGL也会自动替换当前的 VAO。
     *
     * 那么 vao.release() 什么时候有用？
     * 1.有时你可能希望在某些操作之后不绑定任何 VAO。
     * 这时调用 vao.release() （等效于 glBindVertexArray(0)) 会使当前 VAO 解绑，
     * 使得接下来的操作不再影响当前的顶点数组配置。
     * 2.防止意外修改：在某些场景中，解除 VAO 绑定可以防止后续代码意外修改当前 VAO 的状态。
     */

    /* VAO中属性的创建和管理
     * glVertexAttribPointer()在当前vao中创建属性，创建时还要给属性一个唯一标号index（不同vao间可以重复）
     * 通过glEnableVertexAttribArray(index)来使属性生效
     * 一个vao中可以同时生效多个属性，这样程序到时候就会加载这些生效的属性
     *
     * 属性创建了就不能删除了，但可以用glDisableVertexAttribArray(index)禁用属性，
     * 禁用后，尽管属性仍然存储在 VAO 中，但 OpenGL 不会使用该属性进行绘制。
     * 禁用属性后可以避免不必要的数据处理：
     * 如果某些顶点属性（如颜色、法线或纹理坐标）在特定的绘制操作中不需要使用，
     * 那么禁用它们可以避免 GPU 对这些不必要的属性进行处理，节省处理时间和内存带宽。
     *
        示例：1.设置属性
        // 设置顶点位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);  // 第一个参数为index
        glEnableVertexAttribArray(0);  // 启用位置属性

        // 设置顶点颜色属性
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);  // 启用颜色属性
        2.禁用属性
        // 绘制阶段1：只启用位置属性
        glEnableVertexAttribArray(0);  // 启用位置属性
        glDisableVertexAttribArray(1); // 禁用颜色属性
        glDisableVertexAttribArray(2); // 禁用法线属性
        glDrawArrays(GL_TRIANGLES, 0, numVertices);  // 绘制

        // 绘制阶段2：启用位置和颜色属性
        glEnableVertexAttribArray(1);  // 启用颜色属性
        glDrawArrays(GL_TRIANGLES, 0, numVertices);  // 绘制
     */

    /* vao和vbo的联动
    在vao1.bind()，vbo1.bind()下，
    在执行glVertexAttribPointer(index3,....)时，系统会自动将自动将vao1中的index3属性与vbo1关联起来，
    不需要人为将vbo2写在glVertexAttribPointer的参数里。
    画图的时候只需要在vao1.bind()下使用index3属性,就知道使用的时vbo1，不需要写vbo1.bind()。
    而且在不同的vbo下，同一个vao的不同属性可以关联到不同的vbo。
     */

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

    /* vbo.allocate()函数注意事项
    每次调用 vbo.allocate() 都会分配新的内存空间并将数据复制到 VBO 中。
    如果 VBO 已经有数据，新的 allocate() 会覆盖之前的数据。

    如果新的数据大小不同于之前的数据大小，vbo.allocate() 会重新分配缓冲区，并将新的数据复制到缓冲区中。
    如果新的大小小于之前分配的大小，旧数据可能会被截断。
    例子：

        // 第一次分配顶点数据
        GLfloat vertices1[] = {
            0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };
        vbo.allocate(vertices1, sizeof(vertices1));  // 分配第一个顶点数据

        // 第二次分配不同的顶点数据
        GLfloat vertices2[] = {
            -0.2f,  0.4f, 0.0f,
            -0.6f, -0.6f, 0.0f,
            0.4f, -0.4f, 0.0f
        };
        vbo.allocate(vertices2, sizeof(vertices2));  // 会覆盖之前的数据
    */

    /* vbo如何存储多组数据
        // 定义顶点位置和颜色（每个顶点有3个坐标，和3个颜色分量）
        GLfloat vertices[] = {
            // 位置         // 颜色
            0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 顶点1：位置(x, y, z)，颜色(r, g, b)
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 顶点2
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // 顶点3
            // 第二个三角形
            0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   // 顶点4
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 1.0f,   // 顶点5
            0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 1.0f    // 顶点6
        };

        vbo.allocate(vertices, sizeof(vertices));  // 将所有顶点和颜色数据都存储到同一个VBO中

        // 设置顶点属性指针 (位置属性)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);  // 位置属性从第一个分量开始
        glEnableVertexAttribArray(0);

        // 设置颜色属性指针 (颜色属性)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));  // 颜色属性从第4个分量开始
        glEnableVertexAttribArray(1);
     */

    // 设置顶点属性指针
    /*glVertexAttribPointer()的各个参数的含义：
     *
     *index: 顶点属性的索引（这里是0）
     *
     *size:n维向量（这里是3）1: 表示一个标量，如 float。2: 表示二维向量，
     *如 vec2（x, y）。3: 表示三维向量，如 vec3（x, y, z）
     *
     *type：每个分量的类型（GL_FLOAT 表示每个顶点的 x, y, z 坐标是 float 类型)
     *此外常见的还有GL_INT、GL_UNSIGNED_BYTE分别表示int和unsigned byte无符号字节类型
     *
     *normalized：是否需要将数据归一化，即如果数据类型是整数类型，
     *是否需要将其映射到 [0, 1] 或 [-1, 1] 的范围。GL_TRUE，整数数据会被归一化；
     *如果是 GL_FALSE，则不归一化，直接使用原始数据，这里是浮点数通常不需要归一化
     *
     *stride：步长，即每个顶点属性之间的字节偏移量。简单来说，
     *它表示从一个顶点到下一个顶点在缓冲区中的距离（单位是字节），这里用的是3维的点，
     *所以距离是 3 * sizeof(float) 字节
     *
     *pointer：这个参数是指向顶点缓冲区中数据的 偏移量。
     *它指定顶点数据在缓冲区中的起始位置。(void*)0 表示数据从缓冲区的第一个字节开始存储，也就是偏移量为 0。
     *当缓冲区内有多个顶点属性时（如位置和颜色），这个偏移量可以用来指定某个属性在顶点数据中的相对位置。
     */
    //修改步长（stride）和偏移量（offset），以便正确读取纹理坐标
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  // 顶点位置
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));  // 纹理坐标
    glEnableVertexAttribArray(1);

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

    /*顶点着色器（Vertex Shader）：
    用于计算每个顶点的位置。代码中 gl_Position 用于将顶点坐标传递给 OpenGL。

    #字符串中这些代码语法是 GLSL（OpenGL Shading Language）独有的
    GLSL 是一种专门为 OpenGL 编写的着色器语言，它有自己独特的语法和结构，适用于 GPU 计算。

代码解析：
    #version 330 core 是 GLSL（OpenGL Shading Language）中的一个指令，用于指定着色器代码的版本和上下文。具体含义如下：
    1. 版本号
        330 表示 GLSL 的版本号是 3.30。这一版本是与 OpenGL 3.3 相对应的，意味着你可以使用这个版本的特性和语法。
        版本号的前两位表示主版本号，后两位表示次版本号。3.30 是第三个主要版本的第 30 个小版本。
    2. 上下文
        core 表示使用的是 OpenGL 的核心上下文。在 OpenGL 中，有两种上下文：核心模式（Core Profile）和兼容模式（Compatibility Profile）。
        核心模式只包含现代 OpenGL 的特性，删除了一些旧版功能（如固定功能管线）。
        兼容模式则支持旧版 OpenGL 的所有特性，允许使用过时的功能。

    layout(location = 0) in vec3 position;
    在 layout(location = 0) in vec3 position; 这行 GLSL 代码中，以下是各部分的解释：
    固定语句部分：
        layout(location = x):
            这是 GLSL 的 布局限定符，用于指定变量在着色器中绑定的位置。location = x 表示这个变量将在位置索引 x 处使用。
            注意:
                glVertexAttribPointer(0, ...) 中的 index 参数和 GLSL 中的 layout(location = 0) 是一一对应的。
                x 值必须与 index 一致，这样着色器中的变量才能正确接收从 VBO 传递来的数据。
                同一个着色器，在不同vao对象下，会读取到不同的属性：
                    layout(location = 0) 会从当前绑定的 VAO 的 0 号属性读取数据。
                    假设你先绑定 VAO1，然后调用渲染函数，着色器会从 VAO1 的 0 号属性（顶点位置）中读取数据：
                    glBindVertexArray(VAO1);  // glBindVertexArray(VAO1) 和 VAO1.bind() 在功能上是等价的
                    shaderProgram->bind();
                    glDrawArrays(GL_TRIANGLES, 0, 3);  // 从 VAO1 的 0 号属性（顶点位置）读取数据
                    如果你接下来绑定 VAO2 再进行渲染，着色器会从 VAO2 的 0 号属性（顶点颜色）中读取数据：
                    glBindVertexArray(VAO2);
                    shaderProgram->bind();
                    glDrawArrays(GL_TRIANGLES, 0, 3);  // 从 VAO2 的 0 号属性（顶点颜色）读取数据
            layout 和 location 都是 GLSL 的关键字，用来指定着色器变量的属性。
        in:
            这是 GLSL 的关键字，表示该变量是从外部输入到着色器的（通常是顶点着色器中的输入数据）。
            在顶点着色器中，in 表示该变量从 CPU 端的顶点缓冲区传递进来。
        vec3:
            这是 GLSL 中的数据类型，表示一个包含 3 个浮点数（x, y, z）的向量，通常用于表示顶点的 3D 位置或颜色等属性。
    变量部分：
        position:
            这是用户定义的变量名称（自定义变量）。它在这里是一个三维向量类型的输入变量，接收传递进来的顶点数据。
            position 可以是任意有效的变量名，用于接收从应用程序中传递过来的顶点坐标信息。
    总结：
        固定语句: layout(location = 0), in, vec3（它们是 GLSL 的关键字和数据类型）。
        变量: position（这是用户定义的变量，用于接收顶点位置数据）。

    void main() {
        gl_Position = vec4(position, 1.0);  // 设置顶点位置
    }
    在 GLSL（OpenGL 着色语言）中，void main() 是片段或顶点着色器的入口点，它表示着色器的主执行逻辑。
    所有在这个函数体内的代码都会在绘制时被执行，而在 main() 函数外的部分则是对输入、输出、全局变量、常量等的声明。
    gl_Position 是 OpenGL 内置的一个顶点着色器变量，用来表示每个顶点的坐标位置。你必须在顶点着色器中给它赋值，来指定当前顶点的坐标。
    position 是通过 layout(location = 0) in vec3 position; 从外部传入的顶点位置数据，它通常从你的顶点缓冲对象（VBO）中获取。
    vec4(position, 1.0) 将 position 这个三维向量（vec3）扩展为一个四维向量（vec4），其中第四个分量设置为 1.0，这是齐次坐标的标准形式，用来表示三维空间的点。
    */

    /* 注意:
        glVertexAttribPointer(index, ...) 中的 index 参数和 GLSL 中的 layout(location = x) 是一一对应的。
        x 值必须与 index 一致，这样着色器中的变量才能正确接收从 VBO 传递来的数据。
        同一个着色器（x不变），在不同vao对象的上下文中，会读取到不同的属性
     */
    // 顶点着色器（传递纹理坐标
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
    #version 460 core
    layout(location = 0) in vec3 position;  // 顶点位置
    layout(location = 1) in vec2 texCoord;  // 纹理坐标

    out vec2 TexCoord;  // 向片段着色器传递纹理坐标

    void main() {
        gl_Position = vec4(position, 1.0);  // 设置顶点位置
        TexCoord = texCoord;  // 传递纹理坐标
    }
)");


    /*片段着色器（Fragment Shader）：
    用于计算每个片段（像素）的颜色。在这个示例中，输出为红色 (vec4(1.0, 0.0, 0.0, 1.0))。
    out vec4 fragColor;指定fragColor作为片段着色器的输出，fragColor的具体值在main()中获得
     */
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
    in vec2 TexCoord;  // 从顶点着色器接收纹理坐标

    out vec4 fragColor;  // 片段颜色输出

    uniform sampler2D texture1;  // 纹理采样器

    void main() {
        fragColor = texture(texture1, TexCoord);  // 从纹理中采样颜色
    }
)");


    shaderProgram->link();  // 链接着色器
    shaderProgram->bind();  // 绑定着色器程序

    // 加载纹理图像
    QImage textureImage(":/textures/001.png");  // 替换为你的纹理图片路径
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
    /*resizeGL作用：
    resizeGL() 是在窗口大小发生变化时被调用的。它通常用于调整视口（Viewport），
    以确保 OpenGL 渲染的内容正确地映射到窗口大小的变化上。
    典型操作是调用 glViewport，设置 OpenGL 的可绘制区域，使其与窗口尺寸相匹配。

    常见用途：
    调整视口：当窗口大小发生改变时，调用 glViewport() 确保 OpenGL 绘制的图形在整个窗口中正确显示。
    重新计算投影矩阵：如果使用的是透视投影或者其他与窗口尺寸有关的投影矩阵，可以在这里重新设置。
     */
    glViewport(0, 0, w, h);
}


/*为什么没看见paintGL()被调用，但是图还是画出来了?
paintGL() 是自动调用的函数，由 Qt 的事件循环和 OpenGL 的上下文管理系统控制。
paintGL() 函数会在需要重绘窗口时被调用。具体来说，以下情况会触发 paintGL() 的调用：

窗口首次显示时：当窗口首次显示出来时，Qt 会请求 OpenGL 绘制其内容，从而调用 paintGL()。

窗口大小改变时：如果窗口的大小发生变化，通常会触发重绘，这时 paintGL() 也会被调用。

窗口被遮挡后重新显示：如果窗口被其他窗口遮挡，然后再显示出来，Qt 可能会调用 paintGL() 来重新绘制被遮挡的部分。

定时器或事件触发：如果有定时器事件（例如使用 QTimer），或者通过其他用户交互（例如鼠标移动、键盘输入等）需要更新界面，也会触发 paintGL()。

调用 update() 或 updateGL()：在代码中显式调用 update() 或 updateGL() 时，Qt 会标记该小部件为需要重绘，并在合适的时机调用 paintGL()。
 */
void MyOpenGLWidget::paintGL()
{
    /*paintGL作用：
    paintGL() 是每次需要重绘窗口时调用的函数。这个函数是你实际执行绘制指令的地方。
    每次窗口刷新、用户交互或其他导致需要重新绘制的操作都会触发这个函数的调用。
    在这个函数中，你将使用 OpenGL 提供的绘制命令（如 glDrawArrays 或 glDrawElements）来绘制你的图形。

    常见用途：
    清除缓冲区：通常使用 glClear() 清除颜色缓冲区和深度缓冲区，为新的绘制做准备。
    绘制图形：在这里调用 OpenGL 的绘制命令来实际渲染内容。
    交换缓冲区：窗口系统会自动处理，双缓冲机制下的缓冲区交换，以确保绘制的内容不会闪烁。
     */

    // 清除当前的绘图缓冲区，以准备进行新的绘制。
    /*glClear参数的含义
    GL_COLOR_BUFFER_BIT：表示要清除颜色缓冲区（也就是屏幕上的颜色图像）。
    当这个标志被设置时，OpenGL 会将颜色缓冲区的内容清除为指定的清除颜色（由 glClearColor 设置）。

    GL_DEPTH_BUFFER_BIT：表示要清除深度缓冲区。深度缓冲区用于记录每个像素的深度信息，
    确保正确的图形遮挡关系。当这个标志被设置时，OpenGL 会将深度缓冲区的内容清除为
    默认深度值（通常是最大的深度值，表示最远的可视点）。
     */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 绑定着色器程序
    shaderProgram->bind();
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
