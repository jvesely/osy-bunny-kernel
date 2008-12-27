/*
 *          _     _
 *          \`\ /`/
 *           \ V /
 *           /. .\            Bunny Kernel for MIPS
 *          =\ T /=
 *           / ^ \
 *        {}/\\ //\
 *        __\ " " /__
 *   jgs (____/^\____)
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/*! 	 
 *   @author Matus Dekanek, Tomas Petrusek, Lubos Slovak, Jan Vesely
 *   @par "SVN Repository"
 *   svn://aiya.ms.mff.cuni.cz/osy0809-depeslve
 *   
 *   @version $Id$
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#include "types.h"

extern "C" size_t putc(const char c);
extern "C" size_t puts(const char * str);
extern "C" size_t printk(const char * format, ...);

/*! prints number as unsigned decimal
 * @param number number to be printed
 * @return number of printed decimal digits
 */
static size_t print_udecimal( uint32_t number)
{
  /* too many digits for the reverting algorithm */
  if ( number > ( (uint)(-1) / 10 ))
    return print_udecimal( number / 10) + putc( number % 10 + '0');
  uint32_t reverse;
  int size;
  size_t count = 0;
  if (number == 0)
    return putc('0');
  for(size = reverse = 0; number != 0;++size){
    reverse *= 10;
    reverse += number % 10;
    number /= 10;
  }
  for (;size;reverse/=10,--size)
    count += putc('0' + reverse % 10);

  return count;
};
/*----------------------------------------------------------------------------*/
/*! prints number as signed decimal
 * @param number number to be printed
 * @return number of printed decimal digits (+ sign)
 */

static size_t print_decimal(const int32_t number)
{
  if (number < 0){
    return putc('-') + print_udecimal(-number);
  }
  return print_udecimal(number);

}
/*----------------------------------------------------------------------------*/
/*! prints number as unsigned hexadecimal
 * @param number number to be printed
 * @param align set to @a true to ouput leading zeros
 * @return number of printed hexadigits
 */
static size_t print_hexa(uint32_t number, bool align)
{
  puts("0x");
  size_t count = 2;
  if(number == 0 && !align)
    return putc('0') + count;
  size_t size;
  uint32_t reverse;
  for (size = reverse = 0; number != 0 ; ++size){
    reverse *= 16;
    reverse += number % 16;
    number /= 16;
  }
  for (uint i = 8; align && i > size ; --i)
      count += putc('0');
  int res;
  for(;size;--size,reverse/=16)
    if((res = reverse % 16) >= 10)
      count += putc('a' + res%10);
    else
      count += putc('0' + res);
  return count;
}
/*----------------------------------------------------------------------------*/
/*! printk prints formated string on the console.
 * formating string may include:
 * %c: corresponding input variable is treated as char
 * %s: corresponding input variale is treated as char *
 * %d: corresponding input variale is treated as signed decimal number
 * %i: same as %d
 * %u: same as %d but the number is treated as unsigned
 * %x: corresponding input variale is treated as unsinged hexadecimal number
 * %p: corresponding input variale is treated as void *output is the same as %x
 * @param format formating string
 * @param args list of variables used in format
 * @return number of printed chars
 */
size_t vprintk(const char * format, va_list args)
{
  bool align = false;
  size_t count = 0;
  while (*format) {
    if (*format == '%') {
      switch (*(format + 1)) {
        case 'c': count += putc(va_arg(args, int));
                  ++format;
                  break;
        case 's': count += puts(va_arg(args, const char*));
                  ++format;
                  break;
        case 'i':
        case 'd': count += print_decimal(va_arg(args, int32_t));
                  ++format;
                  break;
        case 'u': count += print_udecimal(va_arg(args, uint32_t));
                  ++format;
                  break;
        case 'p':
                  align = true;
        case 'x': count += print_hexa(va_arg(args, uint32_t), align);
                  align = false;
                  ++format;
                  break;
        default:
          count += putc('%');
      }
    }else
      count += putc(*format);
    ++format;
  }
  return count;
}
/*----------------------------------------------------------------------------*/
size_t printk(const char * format, ...)
{
  if (!format) return 0;

  va_list args;
  va_start(args, format);
  size_t written = vprintk(format, args);
  va_end(args);

  return written;
}
