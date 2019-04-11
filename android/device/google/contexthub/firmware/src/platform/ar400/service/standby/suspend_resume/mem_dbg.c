#include <plat/inc/include.h>
#if MEM_USED

//#define MEM_DEBUG
#ifdef MEM_DEBUG
#define UART_REG_BASE   (R_UART_REG_BASE)
#define UART_REG_RBR    (UART_REG_BASE + 0x00)
#define UART_REG_THR    (UART_REG_BASE + 0x00)
#define UART_REG_DLL    (UART_REG_BASE + 0x00)
#define UART_REG_DLH    (UART_REG_BASE + 0x04)
#define UART_REG_IER    (UART_REG_BASE + 0x04)
#define UART_REG_IIR    (UART_REG_BASE + 0x08)
#define UART_REG_FCR    (UART_REG_BASE + 0x08)
#define UART_REG_LCR    (UART_REG_BASE + 0x0c)
#define UART_REG_MCR    (UART_REG_BASE + 0x10)
#define UART_REG_LSR    (UART_REG_BASE + 0x14)
#define UART_REG_MSR    (UART_REG_BASE + 0x18)
#define UART_REG_SCH    (UART_REG_BASE + 0x1c)
#define UART_REG_USR    (UART_REG_BASE + 0x7c)
#define UART_REG_TFL    (UART_REG_BASE + 0x80)
#define UART_REG_RFL    (UART_REG_BASE + 0x84)
#define UART_REG_HALT   (UART_REG_BASE + 0xa4)

u32 mem_dbg_level = DEBUG_LEVEL;
static char digit_string[] = "0123456789ABCDEF";
static char fill_ch[] = "                ";
extern u32 uart_pin_not_used;
extern u32 uart_lock;

static s32 mem_uart_putc(char ch)
{
	if (uart_lock || uart_pin_not_used)
		return -EACCES;

	while (!(readl(UART_REG_USR) & 2)) /* fifo is full, check again */
		;

	/* write out charset to transmit fifo */
	writeb(ch, UART_REG_THR);

	return OK;
}

static s32 mem_uart_puts(const char *string)
{
	if (uart_lock || uart_pin_not_used)
		return -EACCES;

	ASSERT(string != NULL);

	while(*string != '\0') {
		if(*string == '\n') {
			/* if current character is '\n', insert output with '\r' */
			mem_uart_putc('\r');
		}
		mem_uart_putc(*string++);
	}

	return OK;
}

#define mem_debugger_puts(string) mem_uart_puts(string)

char *mem_strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

char *mem_strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

int mem_strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		;

	return sc - s;
}


static char *mem_utoa(unsigned int value, char *string)
{
	char stack[16];
	int  i;
	int  j;

	if (value == 0) {
		//zero
		string[0] = '0';
		string[1] = '\0';
		return string;
	}
	for (i = 0; value > 0; ++i) {
		// characters in reverse order are put in 'stack'.
		stack[i] = digit_string[value & 0x0f];
		value >>= 4;
	}

	//restore reversed order result to user string
	for (--i, j = 0; i >= 0; --i, ++j) {
		string[j] = stack[i];
	}
	//must end with '\0'.
	string[j] = '\0';

	return string;
}

static s32 mem_print_align(char *string, s32 len, s32 align)
{
	//fill with space ' ' when align request,
	//the max align length is 16 byte.
	if (len < align)
	{
		//fill at right
		mem_strncat(string, fill_ch, align - len);
		return align - len;
	}
	//not fill anything
	return 0;
}

/*
*********************************************************************************************************
*                                           FORMATTED PRINTF
*
* Description:  print out a formatted string, similar to ANSI-C function printf().
*               This function can support and only support the following conversion specifiers:
*               %d  signed decimal integer.
*               %u  unsigned decimal integer.
*               %x  unsigned hexadecimal integer, using hex digits 0x.
*               %c  single character.
*               %s  character string.
*
* Arguments  :  format  : format control.
*               ...     : arguments.
*
* Returns    :  the number of characters printed out.
*
* Note       :  the usage refer to ANSI-C function printf().
*********************************************************************************************************
*/
extern char debugger_buffer[];
s32 mem_debugger_printf(u32 level, const char *format, ...)
{
	va_list  args;
	char     string[16];    //align by cpu word
	char    *pdest;
	char    *psrc;
	s32      align;
	s32      len = 0;

	if (level & (0xf0 >> (mem_dbg_level +1))) {
		pdest = debugger_buffer;
		va_start(args, format);
		while (*format) {
			if (*format == '%') {
				++format;
				if (('0' < (*format)) && ((*format) <= '9')) {
					//we just suport wide from 1 to 9.
					align = *format - '0';
					++format;
				} else {
					align = 0;
				}
				switch(*format) {
					case 'x':
					case 'p':
					{
						//hex
						mem_utoa(va_arg(args, unsigned long), string);
						len = mem_strlen(string);
						len += mem_print_align(string, len, align);
						mem_strcpy(pdest, string);
						pdest += len;
						break;
					}
					case 'c':
					{
						//charset, aligned by cpu word
						*pdest = (char)va_arg(args, int);
						break;
					}
					case 's':
					{
						//string
						psrc = va_arg(args, char *);
						mem_strcpy(pdest, psrc);
						pdest += mem_strlen(psrc);
						break;
					}
					default :
					{
						//no-conversion
						*pdest++ = '%';
						*pdest++ = *format;
					}
				}
			} else {
				*pdest++ = *format;
			}
			//parse next token
			++format;
		}
		va_end(args);

		//must end with '\0'
		*pdest = '\0';
		pdest++;
		mem_debugger_puts(debugger_buffer);

		return (pdest - debugger_buffer);
	}
	return OK;
}


void mem_reg_debug(const char *name, const unsigned int *start, unsigned int len)
{
	int i;

	MEM_LOG("%s:", name);
	for (i = 0; i < len; i++) {
		if ((i & 3) == 0)
			MEM_LOG("\n[%x]:", start + i);
		MEM_LOG("%x", *(start + i));
	}
	MEM_LOG("\n");
}

void mem_hexdump(char* name, char* base, int len)
{
	u32 i;
	MEM_LOG("%s :\n", name);
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
		MEM_LOG("\n0x%8x : ", base + i);
		MEM_LOG("%8x ", readl(base + i));
		}
	MEM_LOG("\n");
}
#endif
void *mem_memcpy(void *dest, const void *src, size_t n)
{
	char *cs;
	char *cd;

	/* check if 'src' and 'dest' are on LONG boundaries */
	if (0x03 & ((unsigned long)dest | (unsigned long)src)) { /* only for 32 archtecture plarform */
	//if (1) {
		/* no, do a byte-wide copy */
		cs = (char *)src;
		cd = (char *)dest;

		while (n--)
			*cd++ = *cs++;
	} else {
		/* yes, speed up copy process */
		/* copy as many LONGs as possible */
		long *ls = (long *)src;
		long *ld = (long *)dest;

		size_t cnt = n >> 2;
		while (cnt--)
			*ld++ = *ls++;

		/* finally copy the remaining bytes */
		cs = (char *)((unsigned int)src + (n & ~0x03));
		cd = (char *)((unsigned int)dest + (n & ~0x03));

		cnt = n & 0x03;
		while (cnt--)
			*cd++ = *cs++;
	}

	return dest;
}

void *mem_memset(void *s, int c, size_t n)
{
	char *p = s;

	while (n--)
		*p++ = c;

	return s;
}

void mem_reg_save(unsigned int *dest, const unsigned int *src, \
                  unsigned int n, unsigned int skip)
{
	//ASSERT(dest != NULL);
	//ASSERT(src != NULL);

	while (n--) {
		*(dest + n) = *(src + n * skip);
		//printk("ad:%p as:%p dd:%x ds:%x\n", dest + n, src + n * skip, *(dest + n), *(src + n * skip));
	}
}

void mem_reg_restore(unsigned int *dest, const unsigned int *src, \
                     unsigned int n, unsigned int skip)
{
	//ASSERT(dest != NULL);
	//ASSERT(src != NULL);

	while (n--) {
		*(dest + n * skip) = *(src + n);
		//printk("ad:%p as:%p dd:%x ds:%x\n", dest + n * skip, src + n, *(dest + n * skip), *(src + n));
	}
}

#endif /* MEM_USED */
