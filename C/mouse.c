#include <X11/Xlib.h>
#include <stdio.h>

int main()
{
    XEvent ev;

    Display *display = XOpenDisplay(NULL);
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 10, 10,
                                        640, 480, 1, BlackPixel(display, 0), WhitePixel(display, 0));

    XMapWindow(display, window);
    XSelectInput(display, window, PointerMotionMask);

    while (1)
    {
        XNextEvent(display, &ev);
        if (ev.type == MotionNotify)
        {
            printf("X: %d | Y: %d\n", ev.xmotion.x, ev.xmotion.y);
        }
        else if (ev.type == ButtonPress)
        {
            // Evento de pressionamento de botão do mouse
            if (ev.xbutton.button == LEFT_BUTTON)
            {
                printf("Botão esquerdo pressionado! X: %d | Y: %d\n", ev.xbutton.x, ev.xbutton.y);
            }
            else if (ev.xbutton.button == RIGHT_BUTTON)
            {
                printf("Botão direito pressionado! X: %d | Y: %d\n", ev.xbutton.x, ev.xbutton.y);
            }
        }

    }
}




