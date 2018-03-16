#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <signal.h>
#include <locale.h>

typedef struct XConf {
    int fd, screen, width, height;
    Window root;
    Display* display;
    Visual* visual;
    Colormap colormap;
    unsigned depth;
    Window self;
    XftDraw* xft;
    Pixmap pixmap;
    XIC xic;
    XIM xim;
    GC gc;
    void (*eventfns[LASTEvent])(struct XConf*, XEvent*);
} XConf;

static int x11_init(XConf* x) {
    signal(SIGPIPE, SIG_IGN); // Ignore the SIGPIPE signal
    setlocale(LC_CTYPE, "");
    XSetLocaleModifiers("");
    /* open the X display and get basic attributes */
    if (!(x->display = XOpenDisplay(0)))
        return -1;
    x->root = DefaultRootWindow(x->display);
    XWindowAttributes wa;
    XGetWindowAttributes(x->display, x->root, &wa);
    x->visual   = wa.visual;
    x->colormap = wa.colormap;
    x->screen   = DefaultScreen(x->display);
    x->depth    = DefaultDepth(x->display, x->screen);
    return 0;
}

static void x11_mkwin(XConf* x, int width, int height, int evmask) {
    /* create the main window */
    x->width = width, x->height = height;
    x->self = XCreateSimpleWindow(
        x->display, x->root, 0, 0, x->width, x->height, 0, x->depth, -1);
    /* register interest in the delete window message */
    Atom wmDeleteMessage = XInternAtom(x->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x->display, x->self, &wmDeleteMessage, 1);
    /* setup window attributes and events */
    XSetWindowAttributes swa;
    swa.backing_store = WhenMapped;
    swa.bit_gravity = NorthWestGravity;
    XChangeWindowAttributes(x->display, x->self, CWBackingStore|CWBitGravity, &swa);
    XSelectInput(x->display, x->self, evmask);
}

static void x11_init_gc(XConf* x) {
    /* set input methods */
    if ((x->xim = XOpenIM(x->display, 0, 0, 0)))
        x->xic = XCreateIC(x->xim, XNInputStyle, XIMPreeditNothing|XIMStatusNothing, XNClientWindow, x->self, XNFocusWindow, x->self, NULL);
    /* initialize pixmap and drawing context */
    x->pixmap = XCreatePixmap(x->display, x->self, x->width, x->height, x->depth);
    x->xft    = XftDrawCreate(x->display, x->pixmap, x->visual, x->colormap);
    /* initialize the graphics context */
    XGCValues gcv;
    gcv.foreground = WhitePixel(x->display, x->screen);
    gcv.graphics_exposures = False;
    x->gc = XCreateGC(x->display, x->self, GCForeground|GCGraphicsExposures, &gcv);
}

static void x11_show(XConf* x) {
    /* simulate an initial resize and map the window */
    XConfigureEvent ce;
    ce.type   = ConfigureNotify;
    ce.width  = x->width;
    ce.height = x->height;
    XSendEvent(x->display, x->self, False, StructureNotifyMask, (XEvent *)&ce);
    XMapWindow(x->display, x->self);
}

static void x11_process_events(XConf* x) {
    for (XEvent e; XPending(x->display);) {
        XNextEvent(x->display, &e);
        if (!XFilterEvent(&e, None) && x->eventfns[e.type])
            x->eventfns[e.type](x, &e);
    }
}



