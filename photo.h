/*									tab:8
 *
 * photo.h - photo display header file
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    3
 * Creation Date:   Fri Sep  9 21:45:34 2011
 * Filename:	    photo.h
 * History:
 *	SL	1	Fri Sep  9 21:45:34 2011
 *		First written.
 *	SL	2	Sun Sep 11 09:59:23 2011
 *		Completed initial implementation.
 *	SL	3	Wed Sep 14 21:56:08 2011
 *		Cleaned up code for distribution.
 */
#ifndef PHOTO_H
#define PHOTO_H


#include <stdint.h>

#include "types.h"
#include "modex.h"
#include "photo_headers.h"
#include "world.h"


/* limits on allowed size of room photos and object images */
#define MAX_PHOTO_WIDTH   1024
#define MAX_PHOTO_HEIGHT  1024
#define MAX_OBJECT_WIDTH  160
#define MAX_OBJECT_HEIGHT 100

struct Node {
	/* numerators in caluclation of averages (sum of values for rgb 
	pixels mapped to this node in octree) */
	int rSum_; int gSum_; int bSum_;
	/* denominator in caluclation of averages (number of pixels mapped to 
	this node in octree) */
	int count_;
	int parent_; /* index of parent in level 2 of octree. =0 if this is a level 2 node*/
	int index_; /* index of this node to be preserved after qsort */
};


/* Fill a buffer with the pixels for a horizontal line of current room. */
extern void fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM]);

/* Fill a buffer with the pixels for a vertical line of current room. */
extern void fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM]);

/* Get height of object image in pixels. */
extern uint32_t image_height (const image_t* im);

/* Get width of object image in pixels. */
extern uint32_t image_width (const image_t* im);

/* Get height of room photo in pixels. */
extern uint32_t photo_height (const photo_t* p);

/* Get width of room photo in pixels. */
extern uint32_t photo_width (const photo_t* p);

/* 
 * Prepare room for display (record pointer for use by callbacks, set up
 * VGA palette, etc.). 
 */
extern void prep_room (const room_t* r);

/* Read object image from a file into a dynamically allocated structure. */
extern image_t* read_obj_image (const char* fname);

/* Read room photo from a file into a dynamically allocated structure. */
extern photo_t* read_photo (const char* fname);

/* compare function for qsort */
int cmpfunc (const void * a, const void * b);

/* helper function that determines which palette color an rgb pixel should have */
uint8_t determinePaletteValue(uint16_t pixel, uint8_t palette[192][3], struct Node level4[8*8*8*8]);

/* 
 * N.B.  I'm aware that Valgrind and similar tools will report the fact that
 * I chose not to bother freeing image data before terminating the program.
 * It's probably a bad habit, but ... maybe in a future release (FIXME).
 * (The data are needed until the program terminates, and all data are freed
 * when a program terminates.)
 */

#endif /* PHOTO_H */
