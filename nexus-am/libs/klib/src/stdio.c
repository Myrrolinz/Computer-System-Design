#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* PA2.2
 * Date: 2020/08
 */

/* macros */
#define	MAX_BUF	2000
#define		IsDigit(x)	( ((x) >= '0') && ((x) <= '9') )
#define		Ctod(x)		( (x) - '0')

/* forward declaration */
extern int PrintChar(char *, char, int, int);
extern int PrintString(char *, char *, int, int);
extern int PrintNum(char *, unsigned long, int, int, int, int, char, int);

int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char out[MAX_BUF];
	int len = vsprintf(out, fmt, ap);
	int i;
	for( i = 0; i < len; i ++){
    	_putc(out[i]);
  	}
	return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
	char buf[MAX_BUF];
	
	int total_len = 0;
	
    char c;
    char *s;
    long int num;

    int longFlag; 
    int negFlag; 
    int width; 		// output width
    int prec; 		// the precision of decimal
    int ladjust; 	// left align
    char padc; 		// the character used to fill extra positions

    int length;

    for(;;) {
	
        { 
            /* scan for the next '%' */
            while((*fmt) && *fmt != '%') {
                out[total_len ++] = *fmt;
                fmt ++ ;
            }  
            
            /* are we hitting the end? */
            if((*fmt) == 0 ) break;
        }
        /*init the variable */
        longFlag = 0;
        negFlag = 0;
        width = 0;
        ladjust = 0;
        prec = 0;
        padc = ' ';

        /* we found a '%' */
        fmt ++ ;

        /*check for flag */
        if(*fmt == '-') ladjust = 1, fmt ++;
        else if(*fmt == '0') padc = '0' , fmt ++;

        /*check for width */
        while( IsDigit(*fmt) ) {
            width = width * 10 + Ctod(*fmt) ;
            fmt ++ ;
        }

        /*check for precision */
        if(*fmt == '.') 
        {
            fmt ++ ;
            while(IsDigit(*fmt)) {
                prec = prec * 10 + Ctod(*fmt);
                fmt ++ ;
            }
        }
        
        /* check for long */
        if( *fmt == 'l' ) {
            longFlag = 1;
            fmt ++ ;
        } 
        
        length = 0;
        /* check format flag */
        negFlag = 0;
        switch (*fmt) {
        case 'b':
            if (longFlag) { 
                num = va_arg(ap, long int); 
            } else { 
                num = va_arg(ap, int);
            }
            length = PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
            break;

        case 'd':
        case 'D':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            if (num < 0) {
                num = - num;
                negFlag = 1;
            }
            length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0);
            break;

        case 'o':
        case 'O':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
            break;

        case 'u':
        case 'U':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
            break;
        case 'p':    
        case 'x':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
            break;

        case 'X':
            if (longFlag) { 
                num = va_arg(ap, long int);
            } else { 
                num = va_arg(ap, int); 
            }
            length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
            break;

        case 'c':
            c = (char)va_arg(ap, int);
            length = PrintChar(buf, c, width, ladjust);
            break;

        case 's':
            s = (char*)va_arg(ap, char *);
            length = PrintString(buf, s, width, ladjust);
            break;

        case '\0':
            fmt --;
            break;

        default:
            /* output this char as it is */
            buf[length ++] = *fmt;
        }	/* switch (*fmt) */

        fmt ++;
        int i;
        for(i = 0; i < length; ++ i) {
        	out[total_len ++] = buf[i];
        }
        
    }		

    /* special termination call */
    out[total_len] = '\0';
    return total_len;
}
/* --------------- local help functions --------------------- */
int PrintChar(char * buf, char c, int length, int ladjust)
{
    int i;
    
    if (length < 1) length = 1;
    if (ladjust) {
	    *buf = c;
	    for (i = 1; i < length; i ++) buf[i] = ' ';
    } else {
        for (i = 0; i < length - 1; i ++) buf[i] = ' ';
        buf[length - 1] = c;
    }
    return length;
}

int PrintString(char * buf, char* s, int length, int ladjust)
{
    int i;
    int len = 0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    if (ladjust) {
        for (i=0; i< len; i++) buf[i] = s[i];
        for (i=len; i< length; i++) buf[i] = ' ';
    } else {
        for (i=0; i< length-len; i++) buf[i] = ' ';
        for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    }
    return length;
}

int
PrintNum(char * buf, unsigned long u, int base, int negFlag, 
	 int length, int ladjust, char padc, int upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int actualLength =0;
    char *p = buf;
    int i;

    do {
        int tmp = u %base;
        if (tmp <= 9) {
            *p++ = '0' + tmp;
        } else if (upcase) {
            *p++ = 'A' + tmp - 10;
        } else {
            *p++ = 'a' + tmp - 10;
        }
        u /= base;
    } while (u != 0);

    if (negFlag) {
	    *p++ = '-';
    }
    
    // finally, we will reverse the whole string
    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    if (ladjust) {
	    padc = ' '; 
    }
    if (negFlag && !ladjust && (padc == '0')) {
        for (i = actualLength-1; i < length-1; i++) buf[i] = padc; 
        buf[length -1] = '-';
    } else {
	    for (i = actualLength; i < length; i++) buf[i] = padc;
    }
	    

    /* prepare to reverse the string */
    {
        int begin = 0;
        int end;
        if (ladjust) {
        	//left alogn，just reverse the numbers (including the negFlag:'-' )
            end = actualLength - 1; 
        } else {
            end = length -1;
        }

        while (end > begin) {
            char tmp = buf[begin];
            buf[begin] = buf[end];
            buf[end] = tmp;
            begin ++;
            end --;
        }
    }

    /* adjust the string pointer */
    return length;
}


int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int len = vsprintf(out, fmt, ap);
	va_end(ap);
	return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
	assert(0);
	return 0;
}

#endif
