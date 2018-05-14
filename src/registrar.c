#include <x11.h>
#include <unistd.h>

typedef struct TWindow {
    struct TWindow* next;
    Window win;
} TWindow;

Atom XA_REGISTRAR, XA_ADD, XA_DEL, XA_OPEN;
TWindow* Windows = NULL;

static void win_add(Window id) {
    TWindow* win = calloc(1, sizeof(TWindow));
    win->win = id;
    win->next = Windows;
    Windows = win;
}

static void win_del(Window id) {
    if (!Windows) return;
    if (Windows->win == id) {
        TWindow* deadite = Windows;
        Windows = deadite->next;
        free(deadite);
    } else {
        TWindow* w = Windows;
        for (;w && w->next && (w->next->win != id); w = w->next);
        if (w && w->next) {
            TWindow* deadite = w->next;
            w->next = deadite->next;
            free(deadite);
        }
    }
}

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void clientmsg(XConf* x, XEvent* e) {
    if (XA_REGISTRAR != e->xclient.message_type)
        return;
    if (XA_ADD == e->xclient.data.l[0])
        win_add(e->xclient.window);
    else if (XA_DEL == e->xclient.data.l[0])
        win_del(e->xclient.window);
    else if (XA_OPEN == e->xclient.data.l[0])
        puts("open");
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    XA_REGISTRAR = XInternAtom(x.display, "TIDE_REGISTRAR", 0);
    XA_ADD = XInternAtom(x.display, "ADD", 0);
    XA_DEL = XInternAtom(x.display, "DEL", 0);
    XA_OPEN = XInternAtom(x.display, "OPEN", 0);
    x.eventfns[SelectionClear] = selclear;
    x.eventfns[ClientMessage] = clientmsg;
    if (None == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
        XSetSelectionOwner(x.display, XA_REGISTRAR, x.self, CurrentTime);
        if (x.self == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
            x11_event_loop(&x);
        }
    }
    return 1;
}
