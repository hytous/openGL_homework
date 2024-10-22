#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget();

// protected:这些函数只适配此类，别的类用不来，确保这些函数不会被外部类调用，而只能在类的内部或派生类中使用。
/*protected 访问修饰符
访问权限：protected 成员可以被类的内部访问，同时也可以被派生类访问。
        但是，外部对象仍然无法访问 protected 成员。
继承行为：当一个类继承了另一个类时，基类的 protected 成员可以被派生类直接访问。
        这种访问权限允许派生类在继承和扩展基类功能时有更多的自由，尤其是当基类的某些实现细节需要派生类协作时。
典型使用场景：protected 通常用于允许派生类访问基类的某些实现细节，但仍然对外部隐藏这些细节。
            通过这种方式，派生类可以利用这些细节而不暴露给外部对象。
 */
protected:
    /*为什么要加 override？
    确保正确重载父类的虚函数：override 告诉编译器该函数是对父类中虚函数的重载（override）。
    如果函数签名不完全匹配父类的虚函数，编译器会报错。这可以防止由于函数签名错误导致的意外行为。

    例如，如果父类中有一个虚函数 void initializeGL()，而子类中你误写为 void initializeGl()（大小写
    错误），如果不加 override，编译器不会认为这是重载，会把它当作一个新的函数。
    而如果加了 override，编译器会发现这个函数并没有与父类中的虚函数匹配，从而报错，提醒你修正函数名。

    提高代码可读性：加上 override 可以让代码的读者明确知道这个函数是对父类虚函数的重载，
    而不是定义一个新的函数。这有助于理解类之间的继承关系和设计意图。
     */
    void initializeGL() override;  // 它通常用于初始化OpenGL 资源和状态，例如加载着色器、创建 VBO（顶点缓冲对象）或 VAO（顶点数组对象）、设置清除颜色等。这些操作只需要执行一次，在整个绘制过程中不会频繁变化。
    void resizeGL(int w, int h) override;  // 常见用途如：1.调整视口：当窗口大小发生改变时，调用 glViewport() 确保 OpenGL 绘制的图形在整个窗口中正确显示。2.重新计算投影矩阵：如果使用的是透视投影或者其他与窗口尺寸有关的投影矩阵，可以在这里重新设置。
    void paintGL() override;  // 是每次需要重绘窗口时调用的函数。清除缓冲区、绘制图形、交换缓冲区等功能

private:  // 只在此类中会用到的东西
    /*private 访问修饰符
    访问权限：private 成员只能被类的内部访问，类的派生类以及外部对象无法访问 private 成员。
    继承行为：当一个类继承了另一个类时，基类的 private 成员不会被派生类访问到，即使它是通过公共继承。
    典型使用场景：private 通常用于隐藏实现细节，保护类的内部状态，不允许外部或派生类直接访问。
                这样可以强制使用类的接口函数（public 函数）来修改或访问私有成员，从而保证类的封装性。
     */
    QOpenGLVertexArrayObject vao;  // VAO 成员变量
    QOpenGLBuffer vbo;             // VBO 成员变量
    /*
     * QOpenGLVertexArrayObject vao：用于存储顶点数组对象的状态。这个对象会记录顶点属性布局，避免在每次绘制时重新设置。
     * QOpenGLBuffer vbo：用于存储顶点数据的缓冲区对象。顶点数据会上传到显存中，以供 OpenGL 使用。
     */
};

#endif // MYOPENGLWIDGET_H
