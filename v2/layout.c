#ifndef LAYOUT_C
#define LAYOUT_C

typedef struct Layout
{
    float x, y, width, height;
    float runningY;
    float offsetY, pageHeight;
} Layout;


inline void DrawMyRectGL(f32 x, f32 y, f32 w, f32 h)
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x, y + h);

    glVertex2f(x, y);

    glVertex2f(x + w, y + h);

    glVertex2f(x + w, y);
    glEnd();
}

inline void DrawSquareCentered(f32 x, f32 y, f32 side)
{
    DrawMyRectGL(x - side / 2, y - side / 2, side, side);
}

inline void FillLayout(Layout* layout, V3f color)
{
    glColor3f(color.x, color.y, color.z);
    DrawMyRectGL(layout->x, layout->y, layout->width, layout->height);
}


inline void DrawScrollbar(Layout *layout, float scrollWidth)
{
    if(layout->pageHeight > layout->height)
    {
        i32 scrollY = layout->offsetY * (layout->height / layout->pageHeight);
        i32 scrollHeight = layout->height * (layout->height / layout->pageHeight);
        
        glColor3f(0.3f, 0.3f, 0.3f);
        DrawMyRectGL(layout->x + layout->width - scrollWidth, 
                     layout->y + layout->height - scrollHeight + scrollY, 
                     scrollWidth,
                     scrollHeight);
    }
}

i32 IsMouseOverCurrentLayout(Layout *layout, UserInput *input)
{
    return layout->x <= input->mouseX && (layout->x + layout->width) > input->mouseX &&
           layout->y <= input->mouseY && (layout->y + layout->height) > input->mouseY;
}


void ClipLayout(Layout *layout){
    glEnable(GL_SCISSOR_TEST);
    glScissor(layout->x, layout->y, layout->width, layout->height);
}

void EndCliping()
{
    glDisable(GL_SCISSOR_TEST);
}


#endif