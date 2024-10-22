#include "MyOpenGLWidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

MyOpenGLWidget::~MyOpenGLWidget()
{
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
    GLfloat vertices[] = {  // 简单三角形的顶点坐标
        0.0f,  0.5f, 0.0f,  // 上顶点
        -0.5f, -0.5f, 0.0f, // 左顶点
        0.5f, -0.5f, 0.0f   // 右顶点
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  // 启用顶点属性，参数是上面的index，上面为0这里就是0

    // 解绑 VBO 和 VAO
    vbo.release();
    vao.release();
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

    // 绑定vao
    vao.bind();

    // 画三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 解绑
    vao.release();
}
