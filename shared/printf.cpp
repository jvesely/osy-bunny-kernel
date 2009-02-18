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
#include "putc_a.h"

extern "C" size_t puts( const char * str );

/*! prints number as unsigned decimal
 * @param number number to be printed
 * @return number of printed decimal digits
 */
inline size_t print_udecimal( uint32_t number )
{
  /* too many digits for the reverting algorithm */
  if ( number > ( (uint)(-1) / 10 ))
    return print_udecimal( number / 10) + putc_a( number % 10 + '0');
  uint32_t reverse;
  int size;
  size_t count = 0;
  if (number == 0)
    return putc_a('0');
  for(size = reverse = 0; number != 0; ++size){
    reverse *= 10;
    reverse += number % 10;
    number /= 10;
  }
  for (; size; reverse/=10, --size)
    count += putc_a('0' + reverse % 10);

  return count;
};
/*----------------------------------------------------------------------------*/
/*! prints number as signed decimal
 * @param number number to be printed
 * @return number of printed decimal digits (+ sign)
 */

inline size_t print_decimal( const int32_t number )
{
  if (number < 0){
    return putc_a('-') + print_udecimal(-number);
  }
  return print_udecimal(number);

}
/*----------------------------------------------------------------------------*/
/*! prints number as unsigned hexadecimal
 * @param number number to be printed
 * @param align set to @a true to ouput leading zeros
 * @return number of printed hexadigits
 */
inline size_t print_hexa( uint32_t number, bool align )
{
  putc_a('0'); putc_a('x');
  size_t count = 2;
  if(number == 0 && !align)
    return putc_a('0') + count;
  size_t size;
  uint32_t reverse;
  for (size = reverse = 0; number != 0 ; ++size){
    reverse *= 16;
    reverse += number % 16;
    number /= 16;
  }
  for (uint i = 8; align && i > size ; --i)
      count += putc_a('0');
  int res;
  for(; size; --size, reverse /= 16)
    if((res = reverse % 16) >= 10)
      count += putc_a('a' + res % 10);
    else
      count += putc_a('0' + res);
  return count;
}
/*----------------------------------------------------------------------------*/
/*! prints string
 * @param str string to be printed
 * @return number of printed characters
 */
inline size_t print_string( const char* str )
{
	size_t count = 0;
	char* pos = (char*)str;
	while (*pos != '\0') {
		count += putc_a(*(pos++));
	}
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
size_t vprintf( const char * format, va_list args )
{
  bool align = false;
  size_t count = 0;
  while (*format) {
    if (*format == '%') {
      switch (*(format + 1)) {
        case 'c': count += putc_a(va_arg(args, int));
                  ++format;
                  break;
        case 's': count += print_string(va_arg(args, const char*));
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
          count += putc_a('%');
      }
    }else
      count += putc_a(*format);
    ++format;
  }
  return count;
}
/*----------------------------------------------------------------------------*/
extern "C"
size_t printf( const char * format, ... )
{
  if (!format) return 0;

  va_list args;
  va_start(args, format);
  size_t written = vprintf(format, args);
  va_end(args);

  return written;
}
