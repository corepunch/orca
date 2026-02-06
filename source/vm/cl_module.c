#if 0

#include <code/common/vcore_local.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define VMU(N) (*((unsigned int*)vm->r + N))
#define VMI(N) (*((int*)vm->r + N))
#define VMF(N) (*((float*)vm->r + N))
#define VMD(N) (*((double*)vm->r + N))
#define VMA(A) ((void*)(vm->memory + vm->r[A]))
#define SYSCALL(name) uint32_t _##name(PVM vm)
#define VM_ALLOC(vm, size) VM_PtrToVM(vm, vm_allocmem(vm, size))
#define VM_REALLOC(vm, mem, size) VM_PtrToVM(vm, vm_reallocmem(vm, mem, size))
#define VM_FREE(vm, mem) vm_freemem(vm, mem);

uint32_t VM_PtrToVM(PVM vm, void * ptr) {
    if (ptr) {
        return (uint32_t)((void *)(ptr) - (void *)vm->memory);
    } else {
        return 0;
    }
}

inline static int __modeStringToFlags(lpcString_t  mode) {
    int flags = 0;
    if (strcmp(mode, "r") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "w") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "r+") == 0) {
        flags = O_RDWR;
    } else if (strcmp(mode, "w+") == 0) {
        flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a+") == 0) {
        flags = O_RDWR | O_CREAT | O_APPEND;
    }
    return flags;
}

inline static uint32_t FloatAsInt(float f) {
    int temp;
    *(float *)&temp = f;
    return temp;
}

SYSCALL(fopen) {
    lpcString_t  name = VMA(0);
    lpcString_t  mode = VMA(1);
    uint32_t f = open(name, __modeStringToFlags(mode));
    return f == -1 ? 0 : f;
}

SYSCALL(freopen) {
    close(VMU(2));
    _fopen(vm);
    return *vm->r;
}

SYSCALL(feof) {
    off_t currentOffset = lseek(VMU(0), 0, SEEK_CUR);
    off_t endOffset = lseek(VMU(0), 0, SEEK_END);
    if (currentOffset == (off_t)-1 || endOffset == (off_t)-1) {
        perror("Error checking end of file");
        return 0;
    }
    lseek(VMU(0), currentOffset, SEEK_SET);
    return (currentOffset == endOffset) ? 1 : 0;
}

SYSCALL(ferror) { return errno; }

SYSCALL(fflush) {
    uint32_t fp = VMU(0);
    if (tcflush(fp, TCIFLUSH) == -1) {
        perror("Error flushing input buffer");
    }
    return *vm->r;
}

SYSCALL(fgets) {
    LPSTR  str = VMA(0);
    uint32_t file = VMU(2);
    uint32_t bytesread = 0;
    for (uint32_t size = VMU(1); size > 0; size--) {
        char ch;
        ssize_t result = read(file, &ch, 1);
        if (result == 0 || ch == '\n') {
            break;
        }
        str[bytesread++] = ch;
    }
    str[bytesread] = 0;
    return *vm->r;
}

SYSCALL(fprintf) { puts(VMA(0)); return 0; }

SYSCALL(fputc) {
    char ch = VMU(1);
    write(VMU(0), &ch, 1);
    return *vm->r;
}

SYSCALL(fputs) {
    lpcString_t  str = VMA(1);
    write(VMU(0), str, strlen(str));
    return *vm->r;
}

SYSCALL(fseek) { return (uint32_t)lseek(VMU(0), VMU(1), VMU(2)); }

SYSCALL(ftell) { return (uint32_t)lseek(VMU(0), 0, SEEK_CUR); }

SYSCALL(fwrite) {
    int bwritten = (uint32_t)write(VMU(3), VMA(0), VMU(1) * VMU(2));
    if (bwritten == -1) {
        perror("Error writing to file");
        return 0;
    } else {
        return bwritten / VMU(1);
    }
}

SYSCALL(fread) {
    int bread = (uint32_t)read(VMU(3), VMA(0), VMU(1) * VMU(2));
    if (bread == -1) {
        perror("Error reading from file");
        return 0;
    } else {
        return bread / VMU(1);
    }
}

SYSCALL(fclose) {
    close(VMU(0));
    return *vm->r;
}

SYSCALL(remove) {
    lpcString_t  pathname = VMA(0);
    return remove(pathname);
}

SYSCALL(rename) {
    lpcString_t  old_filename = VMA(0);
    lpcString_t  new_filename = VMA(1);
    return rename(old_filename, new_filename);
}

SYSCALL(tmpfile) {
    char tmp[1024] = {0};
    mkstemp(tmp);
    return open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 644);
}

SYSCALL(tmpnam) {
    char tmp[1024] = {0};
    mkstemp(tmp);
    strcpy(VMA(0), tmp);
    return *vm->r;
}

SYSCALL(_exit) {
    vm->r[PC_REG] = vm->progsize;
    return *vm->r;
}

SYSCALL(system) {
    Con_Error("system: %s", (lpcString_t )VMA(0));
    return *vm->r;
}

SYSCALL(clock) { return (uint32_t)clock(); }
SYSCALL(difftime) { return difftime(VMU(0), VMU(1)); }
SYSCALL(time) { return (uint32_t)time(NULL); }
SYSCALL(gmtime) {
    longTime_t time = VMU(0);
    struct tm *result = gmtime(&time);
    // Copy the result to the user's buffer
    memcpy(VMA(1), result, sizeof(struct tm));
    return *vm->r;
}
SYSCALL(localtime) {
    longTime_t time = VMU(0);
    struct tm *result = localtime(&time);
    // Copy the result to the user's buffer
    memcpy(VMA(1), result, sizeof(struct tm));
    return *vm->r;
}
SYSCALL(mktime) {
    struct tm *timeinfo = VMA(0);
    return (uint32_t)mktime(timeinfo);
}

SYSCALL(strtof) {
    LPSTR  str = VMA(0);
    uint32_t *endptr = VMA(1);
    LPSTR  native = NULL;
    float value = strtof(str, &native);
    if (endptr) {
        *endptr = VM_PtrToVM(vm, native);
    }
    return FloatAsInt(value);
}

SYSCALL(strtok) {
    LPSTR  str = VMA(0);
    lpcString_t  delimiters = VMA(1);
    static LPSTR  lastToken = NULL; // Holds the position of the next token
    if (str != NULL) {
        lastToken = str;
    } else if (lastToken == NULL) {
        return 0;
    }
    LPSTR  tokenStart = lastToken + strspn(lastToken, delimiters);
    // If the string is empty or only contains delimiters, return NULL
    if (*tokenStart == '\0') {
        lastToken = NULL;
        return 0;
    }
    // Find the end of the token
    LPSTR  tokenEnd = tokenStart + strcspn(tokenStart, delimiters);
    // If this is not the end of the string, replace the delimiter with null
    // character
    if (*tokenEnd != '\0') {
        *tokenEnd = '\0';
        lastToken = tokenEnd + 1;
    } else {
        lastToken = NULL;
    }
    return VM_PtrToVM(vm, tokenStart);
}

static int count_symbol(lpcString_t  str, int symbol) {
    int count = 0;
    while ((str = strchr(str, symbol)) != NULL) {
        count++;
        str++;  // Move to the next character after the found symbol
    }
    return count;
}

SYSCALL(snprintf) {
    LPSTR  str = VMA(0);
    uint32_t strlen = VMU(1);
    LPSTR  format = VMA(2);
    uint32_t arg = 3;
    memset(str, 0, strlen);
    for (LPSTR  fmt = format, s = str; *fmt; fmt++) {
        uint32_t len = strlen - (uint32_t)(s - str);
        if (*fmt == '%') {
            switch (*(++fmt)) {
                case 's': s += snprintf(s, len, "%s", (lpcString_t )VMA(arg++)); break;
                case 'd': s += snprintf(s, len, "%d", VMU(arg++)); break;
                case 'f': s += snprintf(s, len, "%f", VMF(arg++)); break;
                default:
                    Con_Error("Uknown format in vm_snprintf %%%c", *fmt);
                    assert(0);
                    break;
            }
        } else {
            *(s++) = *fmt;
        }
    }
    return 0;
}

SYSCALL(sscanf) {
    LPSTR  str = VMA(0);
    LPSTR  format = VMA(1);
    switch (count_symbol(format, '%')) {
        case 1: return sscanf(str, format, VMA(2));
        case 2: return sscanf(str, format, VMA(2), VMA(3));
        case 3: return sscanf(str, format, VMA(2), VMA(3), VMA(4));
        case 4: return sscanf(str, format, VMA(2), VMA(3), VMA(4), VMA(5));
        default:
            assert(0);
            return 0;
    }
}


SYSCALL(printf) {
    uint32_t arg = 1;
    for (lpcString_t  fmt = VMA(0); *fmt; fmt++) {
        if (*fmt == '%') {
            switch (*(++fmt)) {
                case 's': Con_Error("%s", (lpcString_t )VMA(arg++)); break;
                case 'd': Con_Error("%d", VMU(arg++)); break;
                case 'f': Con_Error("%f", VMF(arg++)); break;
                default:
                    Con_Error("Uknown format in vm_printf %%%c", *fmt);
                    assert(0);
                    break;
            }
        } else {
            putchar(*fmt);
        }
    }
    putchar('\n');
    return 0;
}

bool_t rr = 0;

uint32_t G_SystemCalls(PVM vm, uint32_t f) {
    uint32_t r = 0;
    Con_Error("%d", f);
    switch (f) {
        case f___addsf3: return FloatAsInt(VMF(0) + VMF(1));
        case f___subsf3: return FloatAsInt(VMF(0) - VMF(1));
        case f___mulsf3: return FloatAsInt(VMF(0) * VMF(1));
        case f___divsf3: return FloatAsInt(VMF(0) / VMF(1));
        case f___gtsf2: return VMF(0) > VMF(1); // greater than zero if neither argument is NaN, and a is strictly greater than b
        case f___gesf2: return VMF(0) >= VMF(1) ? 1 : -1; // greater than or equal to zero if neither argument is NaN, and a is greater than or equal to b.
        case f___ltsf2: return VMF(0) < VMF(1) ? -1 : 0; // less than zero if neither argument is NaN, and a is strictly less than b.
        case f___lesf2: return VMF(0) <= VMF(1) ? -1 : 1; // less than or equal to zero if neither argument is NaN, and a is less than or equal to b.
        case f___eqsf2: return VMF(0) != VMF(1); // zero if neither argument is NaN, and a and b are equal.
        case f___nesf2: return VMF(0) != VMF(1); // nonzero value if either argument is NaN, or if a and b are unequal.
        case f___fixsfsi: return (int)VMF(0);
        case f___fixunssfsi: return VMF(0);
        case f___floatsisf: return FloatAsInt(VMI(0));
        case f___floatunsisf: return FloatAsInt(VMU(0));
        case f___unordsf2: return isnan(VMF(0)) || isnan(VMF(1));
        case f___modsi3: return VMI(0) % VMI(1);
        case f___divsi3: return VMI(0) / VMI(1);
        case f___udivsi3: return VMI(0) / VMU(1);
        case f___umodsi3: return VMI(0) % VMU(1);
        case f___adddf3: VMD(0) = VMD(0) + VMD(1); return *vm->r;
        case f___muldf3: VMD(0) = VMD(0) * VMD(1); return *vm->r;
        case f___divdf3: VMD(0) = VMD(0) / VMD(1); return *vm->r;
        case f___extendsfdf2: VMD(0) = VMF(0); return *vm->r;
        case f___fixsfdi: return VMF(0);
        case f___floatdisf: return FloatAsInt(VMI(0));
        case f___floatundisf: return FloatAsInt(VMU(0));
        case f___floatunsidf: VMD(0) = VMU(0); return *vm->r;
        case f___eqdf2: return VMD(0) == VMD(1);
        case f___gedf2: return VMD(0) >= VMD(1) ? 1 : -1;
        case f___gtdf2: return VMD(0) > VMD(1);
        case f___ltdf2: return VMD(0) < VMD(1) ? -1 : 0;
        case f___truncdfsf2: return FloatAsInt(VMD(0));
        case f___fixdfsi: return VMD(0);
        case f___floatsidf: VMD(0) = VMI(0); return *vm->r;
        case f_floor: VMD(0) = floor(VMD(0)); return *vm->r;
        case f_round: VMD(0) = round(VMD(0)); return *vm->r;
        case f_cos: VMD(0) = cos(VMD(0)); return *vm->r;
        case f_sin: VMD(0) = sin(VMD(0)); return *vm->r;
        case f_sqrt: VMD(0) = sqrt(VMD(0)); return *vm->r;

            
        case f_isalnum: return isalnum(VMU(0));
        case f_isalpha: return isalpha(VMU(0));
        case f_iscntrl: return iscntrl(VMU(0));
        case f_isdigit: return isdigit(VMU(0));
        case f_isgraph: return isgraph(VMU(0));
        case f_islower: return islower(VMU(0));
        case f_ispunct: return ispunct(VMU(0));
        case f_isspace: return isspace(VMU(0));
        case f_isupper: return isupper(VMU(0));
        case f_isxdigit: return isxdigit(VMU(0));

        case f_atof: return FloatAsInt(atof(VMA(0)));
        case f_snprintf: return _snprintf(vm);
        case f_sprintf: assert(0); return 0;
        case f_sscanf: return _sscanf(vm);
        case f_strchr: return VM_PtrToVM(vm, strchr(VMA(0), VMU(1)));
        case f_strcmp: return strcmp(VMA(0), VMA(1));
        case f_strcoll: return strcoll(VMA(0), VMA(1));
        case f_strcpy: strcpy(VMA(0), VMA(1)); return *vm->r;
        case f_strncpy: strncpy(VMA(0), VMA(1), VMU(2)); return *vm->r;
        case f_strdup: assert(0); return 0;
            
        case f_strerror: assert(0); return 0;
        case f_strftime: return (uint32_t)strftime(VMA(0), VMU(1), VMA(2), VMA(3));
        case f_strlen: return (uint32_t)strlen(VMA(0));
        case f_strncmp: return strncmp(VMA(0), VMA(1), VMU(2));
        case f_strpbrk: return VM_PtrToVM(vm, strpbrk(VMA(0), VMA(1)));
        case f_strspn: return (uint32_t)strspn(VMA(0), VMA(1));
        case f_strstr: return VM_PtrToVM(vm, strstr(VMA(0), VMA(1)));
        case f_strtof: return _strtof(vm);
        case f_strtok: return _strtok(vm);

        case f_fopen: return _fopen(vm);
        case f_fwrite: return _fwrite(vm);
        case f_fread: return _fread(vm);
        case f_fclose: return _fclose(vm);
        case f_feof: return _feof(vm);
        case f_ferror: return _ferror(vm);
        case f_fflush: return _fflush(vm);
        case f_fgets: return _fgets(vm);
        case f_fprintf: return _fprintf(vm);
        case f_fputc: return _fputc(vm);
        case f_fputs: return _fputs(vm);
        case f_freopen: return _freopen(vm);
        case f_fseek: return _fseek(vm);
        case f_ftell: return _ftell(vm);
        case f_remove: return _remove(vm);
        case f_rename: return _rename(vm);
        case f_tmpfile: return _tmpfile(vm);
        case f_tmpnam: return _tmpnam(vm);
        case f_ungetc: assert(0); return 0;
        case f_getc: assert(0); return 0;
        case f_puts: assert(0); return 0;
        case f_putchar: assert(0); return 0;
        case f_read: return (uint32_t)read(VMU(0), VMA(1), VMU(2));
        case f_write: return (uint32_t)write(VMU(0), VMA(1), VMU(2));
        case f_getenv: return 0;
        case f_clearerr: errno = 0; return *vm->r;
        case f_memchr: return VM_PtrToVM(vm, memchr(VMA(0), VMU(1), VMU(2)));
        case f_memcmp: return memcmp(VMA(0), VMA(1), VMU(2));
        case f_memcpy: return VM_PtrToVM(vm, memcpy(VMA(0), VMA(1), VMU(2)));
        case f_memset: memset(VMA(0), VMU(1), VMU(2)); return 0;
        case f_abs: return abs(VMI(0));

        case f_frexpf: return FloatAsInt(frexpf(VMF(0), VMA(1)));
        case f_floorf: return FloatAsInt(floorf(VMF(0)));
        case f_fmaxf: return FloatAsInt(fmaxf(VMF(0), VMF(1)));
        case f_fminf: return FloatAsInt(fminf(VMF(0), VMF(1)));
        case f_acosf: return FloatAsInt(acosf(VMF(0)));
        case f_asinf: return FloatAsInt(asinf(VMF(0)));
        case f_atan2f: return FloatAsInt(atan2f(VMF(0), VMF(1)));
        case f_ceilf: return FloatAsInt(ceilf(VMF(0)));
        case f_cosf: return FloatAsInt(cosf(VMF(0)));
        case f_expf: return FloatAsInt(expf(VMF(0)));
        case f_fmax: return FloatAsInt(fmax(VMF(0), VMF(1)));
        case f_fmin: return FloatAsInt(fmin(VMF(0), VMF(1)));
        case f_fmodf: return FloatAsInt(fmodf(VMF(0), VMF(1)));
        case f_log10f: return FloatAsInt(log10f(VMF(0)));
        case f_log2f: return FloatAsInt(log2f(VMF(0)));
        case f_logf: return FloatAsInt(logf(VMF(0)));
        case f_tanf: return FloatAsInt(tanf(VMF(0)));
        case f_sqrtf: return FloatAsInt(sqrtf(VMF(0)));
        case f_sinf: return FloatAsInt(sinf(VMF(0)));
        case f_powf: return FloatAsInt(powf(VMF(0), VMF(1)));
        case f_roundf: return FloatAsInt(roundf(VMF(0)));
        case f___assert_rtn: assert(0); return 0;
        case f_malloc:  VMU(0) = VM_ALLOC(vm, VMU(0)); return *vm->r;
        case f_realloc: VMU(0) = VM_REALLOC(vm, VMA(0), VMU(1)); return *vm->r;
        case f_free: VM_FREE(vm, VMA(0)); return 0;
        case f_clock: return _clock(vm);
        case f_difftime: return _difftime(vm);
        case f_time: return _time(vm);
        case f_gmtime: return _gmtime(vm);
        case f_localtime: return _localtime(vm);
        case f_mktime: return _mktime(vm);
        case f_setlocale: assert(0); return 0;
        case f_localeconv: assert(0); return 0;
        case f___error: vm->location = vm->progsize; return 0;
        case f_exit: vm->location = vm->progsize; return 0;
        case f_abort: vm->location = vm->progsize; return 0;
        case f_tolower: return tolower(VMI(0));
        case f_toupper: return toupper(VMI(0));
        case f_setjmp:
            vm->r[PC_REG] = vm->location;
            memcpy(VMA(0), vm->r, sizeof(vm->r));
            return 0;
        case f_longjmp: 
            memcpy(vm->r, VMA(0), sizeof(vm->r));
            vm->location = vm->r[PC_REG];
            return 1;
        case f_system: assert(0); return 0;
        case f_setvbuf: assert(0); return 0;
        case f_accept: return accept(VMU(0), VMA(1), VMA(2));
        case f_bind: return bind(VMU(0), VMA(1), VMU(2));
        case f_close: return close(VMU(0));
        case f_connect: return connect(VMU(0), VMA(1), VMU(2));
        case f_inet_addr: return inet_addr(VMA(0));
        case f_listen: return listen(VMU(0), VMU(1));
        case f_perror: perror(VMA(0)); return 0;
        case f_printf: return _printf(vm);
        case f_socket: return socket(VMU(0), VMU(1), VMU(2));
        case f_unlink: return unlink(VMA(0));
    }
    return 0;
}

void *read_whole_file(FILE *fp, size_t *psize) {
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void *program = malloc(size);
    fread(program, size, 1, fp);
    if (psize) *psize = size;
    return program;
}

PVM VM_Init(void) {
    FILE *fp = fopen("/Users/igor/Developer/ui-framework/Build/bin/orca.vm", "rb");
    if (!fp) {
        Con_Error("Can't find vm file\n");
        return NULL;
    }
    PVMPROGRAM program = read_whole_file(fp, NULL);
    fclose(fp);
    
    if (program->magic != ID_ORCA) {
        Con_Error("%s has incorrect header\n", program->magic);
        return NULL;
    }
    PVM vm = vm_create(G_SystemCalls, VM_STACK_SIZE, VM_HEAP_SIZE, program);

    FILE *d_fp = fopen("/Users/igor/Developer/ui-framework/Build/bin/orca.vm_d", "rb");
    if (d_fp) {
        size_t dsize = 0;
        PVMDEBUG debug = read_whole_file(d_fp, &dsize);
        fclose(d_fp);
        vm_attach_debug(vm, debug, (uint32_t)dsize / 32);
    }
    
    return vm;
}

//bool_t net_socket_has_data(int sockfd) {
//    fd_set read_fds;
//    FD_ZERO(&read_fds);
//    FD_SET(sockfd, &read_fds);
//    struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
//
//    // Use select() to wait until data is available to be read from the socket
//    if (select(sockfd + 1, &read_fds, NULL, NULL, &tv) == -1) {
//        perror("select");
//        exit(0);
//    }
//    
//    // Check if there is data to read from the socket
//    return FD_ISSET(sockfd, &read_fds);
//}

#endif
