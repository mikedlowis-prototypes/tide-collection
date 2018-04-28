#define _XOPEN_SOURCE 700
#include <stdc.h>
#include <x11.h>
#include <unistd.h>
#include <regex.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef __MACH__
    #define OPENCMD "open"
#else
    #define OPENCMD "xdg-open"
#endif

Atom XA_PLUMB;

typedef struct {
    enum {
        COMPLETE=0, MATCHES, IS, ISSET, ISDIR, ISFILE,
        SET, UNSET, EXEC, LAUNCH, NTYPES
    } type;
    char* arg1;
    char* arg2;
} Rule;

char* TypeNames[NTYPES] = {
    "complete", "matches", "is", "isset", "isdir", "isfile", "set",
    "unset", "exec", "launch"
};

char* Matches[10];

Rule* BuiltinRules[] = {
    (Rule[]){ // Look up .c or .h files in Code/
        { ISSET, "EDITOR", NULL },
        { MATCHES, "data", "\\.[ch]$" },
        { ISDIR, "Code", NULL },
        { EXEC, "[[ $(find Code -type f -name '*$data') ]]", NULL },
        { LAUNCH, "find Code -type f -name '*$data' | xargs -r $EDITOR", NULL },
        { COMPLETE, NULL, NULL }
    },
    (Rule[]){ // Match URLS and open them with the browser
        { ISSET, "BROWSER", NULL },
        { MATCHES, "data", "^(https?|ftp)://.*" },
        { LAUNCH, "$BROWSER $0", NULL },
        { COMPLETE, NULL, NULL }
    },
    (Rule[]){ // Open files with addresses in the editor
        { ISSET, "EDITOR", NULL },
        { MATCHES, "data", "^([^:]+):([0-9]+)" },
        { ISFILE, "$1", NULL },
        { LAUNCH, "tctl $0", NULL },
        { COMPLETE, NULL, NULL }
    },
    (Rule[]){ // If it's an existing text file, open it with editor
        { ISSET, "EDITOR", NULL },
        { ISFILE, "$data", NULL },
        { EXEC, "file --mime '$file' | grep -q 'text/'", NULL },
        { LAUNCH, "$EDITOR '$file'", NULL },
        { COMPLETE, NULL, NULL }
    },
    (Rule[]){ // Look it up in ctags database
        { ISSET, "EDITOR", NULL },
        { ISFILE, "tags", NULL },
        { EXEC, "grep -q '^$data\\s\\+' tags", NULL },
        { LAUNCH, "picktag fetch tags '$data' | xargs -r tide", NULL },
        { COMPLETE, NULL, NULL }
    },
    (Rule[]){ // If it's an existing directory, open it with system default
        { ISDIR, "$data", NULL },
        { LAUNCH, OPENCMD " $data", NULL },
        { COMPLETE, NULL, NULL }
    },
};

char* getvar(char* val) {
    if (strlen(val) == 1 && isdigit(*val))
        val = Matches[*val - '0'];
    else
        val = getenv(val);
    return (val ? val : "");
}

char* strappend(char* rs, size_t* sz, char c) {
    *sz += 1;
    rs = realloc(rs, *sz);
    rs[*sz - 1] = c;
    return rs;
}

char* eval(char* s) {
    size_t sz = 0;
    char* rs = NULL;
    while (s && *s) {
        char c = *s++;
        if ('$' == c && '$' == *s) {
            rs = strappend(rs, &sz, *s++);
        } else if ('$' == c) {
            size_t vsz = 0;
            char* var = calloc(1,1);
            while (*s && (isalnum(*s) || *s == '_'))
                var = strappend(var, &vsz, *s++);
            char* val = getvar(var);
            while (val && *val)
                rs = strappend(rs, &sz, *val++);
            free(var);
        } else {
            rs = strappend(rs, &sz, c);
        }
    }
    rs = strappend(rs, &sz, '\0');
    return rs;
}

bool matches(char* var, char* patt) {
    regex_t regex = {0};
    regmatch_t matches[10] = {0};
    if (regcomp(&regex, patt, REG_EXTENDED) == 0) {
        var = getvar(var);
        memset(Matches, 0, sizeof(Matches));
        int err = regexec(&regex, var, nelem(matches), matches, 0);
        for (int i = 0; i < 10 && matches[i].rm_so >= 0; i++) {
            Matches[i] = strndup(var+matches[i].rm_so, matches[i].rm_eo-matches[i].rm_so);
        }
        return (err == 0);
    }
    return false;
}

bool var_is(char* var, char* val) {
    return (strcmp(getvar(var), eval(val)) == 0);
}

bool var_isset(char* var) {
    return (getenv(var) != NULL);
}

bool var_isdir(char* var) {
    struct stat st = {0};
    char* path = eval(var);
    if ((stat(path, &st) < 0) && (errno == ENOENT)) {
        return false;
    } else if (S_ISDIR(st.st_mode)) {
        setenv("dir", var, 1);
        return true;
    } else {
        return false;
    }
}

bool var_isfile(char* var) {
    struct stat st = {0};
    char* path = eval(var);
    if ((stat(eval(var), &st) < 0) && (errno == ENOENT)) {
        return false;
    } else if (!S_ISDIR(st.st_mode)) {
        setenv("file", path, 1);
        return true;
    } else {
        return false;
    }
}

bool var_set(char* var, char* val) {
    return (setenv(var, eval(val), 1) == 0);
}

bool var_unset(char* var) {
    return (unsetenv(var) == 0);
}

void runcmd(char* cmd) {
    char* shellcmd[] = { getvar("SHELL"), "-c", NULL, NULL };
    if (!shellcmd[0]) shellcmd[0] = "/bin/sh";
    shellcmd[2] = eval(cmd);
    _exit(execvp(shellcmd[0], shellcmd));
}

bool exec(char* cmd) {
    int pid, status, outpipe[2];
    if ((pid = fork()) < 0) return false;
    if (pid == 0) {
        runcmd(cmd);
    } else {
        waitpid(pid, &status, 0);
        return (status == 0);
    }
    return false;
}

bool launch(char* cmd) {
    int pid = fork();
    if (pid > 0)
        return true;
    else if (pid == 0)
        runcmd(cmd);
    return false;
}

bool apply_rule(Rule* rule) {
    switch (rule->type) {
        case COMPLETE: exit(0);
        case MATCHES:  return matches(rule->arg1, rule->arg2);
        case IS:       return var_is(rule->arg1, rule->arg2);
        case ISSET:    return var_isset(rule->arg1);
        case ISDIR:    return var_isdir(rule->arg1);
        case ISFILE:   return var_isfile(rule->arg1);
        case SET:      return var_set(rule->arg1, rule->arg2);
        case UNSET:    return var_unset(rule->arg1);
        case EXEC:     return exec(rule->arg1);
        case LAUNCH:   return launch(rule->arg1);
        default:       return false;
    }
}

void plumb(char* str) {
    if (!str || !str[0]) return;
    if (fork() == 0) {
        setenv("data", str, 1);
        for (int i = 0; i < nelem(BuiltinRules); i++) {
            Rule* rule = BuiltinRules[i];
            for (; rule->type != COMPLETE; rule++) {
                printf("%s '%s' '%s'\n", TypeNames[rule->type], rule->arg1, rule->arg2);
                if (!apply_rule(rule))
                    break;
            }
            puts("");
            if (rule->type == COMPLETE)
                exit(0);
        }
        exit(1);
    }
}

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void propnotify(XConf* x, XEvent* e) {
    int success = 0, nreturn = 0;
    unsigned long nleft;
    XTextProperty prop = {0};
    if (e->xproperty.atom != XA_PLUMB) return;
    success = XGetWindowProperty(
        x->display, x->self, XA_PLUMB, 0, -1, True, AnyPropertyType,
        &prop.encoding, &prop.format, &prop.nitems, &nleft, &prop.value
    );
    if (Success == success) {
        char** strlist = NULL;
        if (Success == XmbTextPropertyToTextList(x->display, &prop, &strlist, &nreturn))
            for (int i = 0; i < nreturn; i++)
                plumb(strlist[i]);
        if (strlist) XFreeStringList(strlist);
    }
    if (prop.value) XFree(prop.value);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    XA_PLUMB = XInternAtom(x.display, "PLUMB", 0);
    x.eventfns[SelectionClear] = selclear;
    x.eventfns[PropertyNotify] = propnotify;
    if (None == XGetSelectionOwner(x.display, XA_PLUMB)) {
        XSetSelectionOwner(x.display, XA_PLUMB, x.self, CurrentTime);
        if (x.self == XGetSelectionOwner(x.display, XA_PLUMB)) {
            x11_event_loop(&x);
        }
    }
    return 1;
}
