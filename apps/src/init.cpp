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

#include <librt.h>
size_t gets_feedback( char* buffer, size_t buffer_size )
{
  uint ptr = 0;
  char c;
  while (( (c = getc()) != '\n') && ptr < (buffer_size) - 1 )
  {
    if (c == 127) {
      if (ptr > 0) {
        --ptr;
        puts( "\b \b" );
      }
    } else {
      buffer[ptr++] = c;
      putc( c );
    }
  }
  buffer[ptr] = '\0';
  return ptr;
}
/*----------------------------------------------------------------------------*/
process_t exec( const char* file_name)
{
	file_t fd;
	if (!fopen( &fd, file_name, OPEN_R ) == EOK) {
		printf( "Opening of file %s failed.\n", file_name );
		return 0;
	}

	const size_t size = fseek( fd, POS_END, 0 );
	fseek( fd, POS_START, 0 );

	if (size == 0){
		printf( "File is empty.\n" );
		return 0;
	}

	void* image = malloc(size);
	if (!image) {
		printf( "Space allocation failed!\n" );
		return false;
	}

	int res;
	if ( (res = fread( fd, image, size )) < 0) {
		printf( "File reading failed. %d \n", res);
		free(image);
		return 0;
	}
	process_t child_pid;
	if (process_create( &child_pid, image, size ) != EOK) {
		printf( "Process creation failed" );
		return 0;
	}
	return child_pid;
}
/*----------------------------------------------------------------------------*/
int main ()
{
	printf( "Welcome to the Simple Shell.\n" );
	file_t curr_dir;

	if (fopen( &curr_dir, ".", 0) != EOK) {
		printf( "Failed to open dir.\n" );
		return -1;
	}
	const size_t BUFF_SIZE = 50;
	char buffer[BUFF_SIZE];
	while (true) {
		printf( "Select one of the files to run or type \'q\' to exit:\n" );
		DIR_ENTRY file;
		while (direntry( curr_dir, &file) == EOK){
			printf( " %s ", file.name );
		}
		puts("\n$ ");
		gets_feedback( buffer, BUFF_SIZE );
		printf( "\n" );
		if (buffer[0] == 'q' && buffer[1] == '\0')
			break;
		exec(buffer);
	}
	printf( "Exiting Simple Shell have fun.\n" );
	fclose (curr_dir );
	return 0;
}
