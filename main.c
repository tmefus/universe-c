#include <GL/glut.h>
#include "res/universe.c"
#include "res/array.c"
#include "res/random.c"

#define Color_Black .15f, .15f, .15f, 1.0f

static const uint16_t Width = 1500;          // 显示范围宽
static const uint16_t Height = 900;          // 显示范围高

Universe *universe = NULL;
char string[20];

static void draw_civi() {
  Star *s = NULL;
  Civi *c = NULL;
  glPointSize(16);
  glBegin(GL_POINTS);
  for (int i = 0; i < universe->Star_List->size; ++i) {
    s = universe->Star_List->items[i];
    if (s->occupied == NULL) {
      glColor3ub(250, 250, 250);
    } else {
      c = s->occupied;
      glColor3ub(c->color.r, c->color.g, c->color.b);
    }
    glVertex2f(s->pos.x, s->pos.y);
  }
  glEnd();
}

static void draw_army() {
  glPointSize(3);
  glBegin(GL_POINTS);
  Army *army;
  for (int i = 0; i < universe->Army_List->size; ++i) {
    army = universe->Army_List->items[i];
    glColor3ub(army->civi->color.r, army->civi->color.g, army->civi->color.b);
    glVertex2f(army->pos.x, army->pos.y);
  }
  glEnd();
}

static void draw_universe() {
  glClear(GL_COLOR_BUFFER_BIT);  // 设置 glClearColor 所定义的颜色为背景色

  uvs_refresh(universe);

  glColor3ub(250, 250, 250);
  glRasterPos2i(10, Height - 24);
  itoa((int) universe->Now_Years, string, 10);
  size_t len = strlen(string);
  for (int i = 0; i < len; ++i) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
  }

  draw_civi();

  draw_army();

  glutSwapBuffers();
}

static void onDraw() {
  glClearColor(Color_Black);
  glClear(GL_COLOR_BUFFER_BIT);  // 设置 glClearColor 所定义的颜色为背景色
  glEnable(GL_POINT_SMOOTH);      // 启动点抗锯齿
  glHint(GL_POINT_SMOOTH, GL_FASTEST); // 图像渲染质量
  draw_universe();
}

static void onTimer() {
  draw_universe();
  glutPostRedisplay();
  glutTimerFunc(16, onTimer, 1);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);    // GLUT初始化
  Init_Rand();                   // 初始化随机数生成器

  universe = create_universe(Width, Height);

  int x = (glutGet(GLUT_SCREEN_WIDTH) - Width) / 2;
  int y = (glutGet(GLUT_SCREEN_HEIGHT) - Height) / 2;
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);    // 使用RGB颜色,使用双缓冲
  glutInitWindowSize(Width, Height);                        // 指定窗口大小
  glutInitWindowPosition(x, y);                             // 指定窗口位置
  glutCreateWindow("Civilization");                     // 新窗口标题
  glMatrixMode(GL_PROJECTION);                        // 正交投影
  gluOrtho2D(0, Width, 0, Height);      // 可绘制坐标范围[左下角0,0]
  glutDisplayFunc(onDraw);                          // 进入绘制方法
  glutTimerFunc(1, onTimer, 1);
  glutMainLoop();                                          // 显示窗口，窗口关闭时跳出循环
}
