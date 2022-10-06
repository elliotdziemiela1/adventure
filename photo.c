/*									tab:8
 *
 * photo.c - photo display functions
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
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:	    photo.c
 * History:
 *	SL	1	Fri Sep  9 21:44:10 2011
 *		First written (based on mazegame code).
 *	SL	2	Sun Sep 11 14:57:59 2011
 *		Completed initial implementation of functions.
 *	SL	3	Wed Sep 14 21:49:44 2011
 *		Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"


struct Node {
	/* numerators in caluclation of averages (sum of values for rgb 
	pixels mapped to this node in octree) */
	int rSum_; int gSum_; int bSum_;
	/* denominator in caluclation of averages (number of pixels mapped to 
	this node in octree) */
	int count_;
	int parent_; /* index of parent in level 2 of octree. =0 if this is a level 2 node*/
};

/* types local to this file (declared in types.h) */

/* 
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as 
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/* 
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have 
 * been stored as 2:2:2-bit RGB values (one byte each), including 
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos, 
 * pixel data are stored as one-byte values starting from the upper 
 * left and traversing the top row before returning to the left of the 
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};


/* file-scope variables */

/* 
 * The room currently shown on the screen.  This value is not known to 
 * the mode X code, but is needed when filling buffers in callbacks from 
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set 
 * by calling prep_room.
 */
static const room_t* cur_room = NULL; 


/* 
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost 
 *                pixel of a line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */ 
    int            yoff;  /* y offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
		    view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (y < obj_y || y >= obj_y + img->hdr.height ||
	    x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
	    continue;
	}

	/* The y offset of drawing is fixed. */
	yoff = (y - obj_y) * img->hdr.width;

	/* 
	 * The x offsets depend on whether the object starts to the left
	 * or to the right of the starting point for the line being drawn.
	 */
	if (x <= obj_x) {
	    idx = obj_x - x;
	    imgx = 0;
	} else {
	    idx = 0;
	    imgx = x - obj_x;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
	    pixel = img->img[yoff + imgx];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of 
 *                a vertical line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */ 
    int            xoff;  /* x offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
		    view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (x < obj_x || x >= obj_x + img->hdr.width ||
	    y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
	    continue;
	}

	/* The x offset of drawing is fixed. */
	xoff = x - obj_x;

	/* 
	 * The y offsets depend on whether the object starts below or 
	 * above the starting point for the line being drawn.
	 */
	if (y <= obj_y) {
	    idx = obj_y - y;
	    imgy = 0;
	} else {
	    idx = 0;
	    imgy = y - obj_y;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
	    pixel = img->img[xoff + img->hdr.width * imgy];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_height (const image_t* im)
{
    return im->hdr.height;
}


/* 
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_width (const image_t* im)
{
    return im->hdr.width;
}

/* 
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/* 
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/* 
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
    cur_room = r;
}


/* 
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;		/* input file               */
    image_t* img = NULL;	/* image structure          */
    uint16_t x;			/* index over image columns */
    uint16_t y;			/* index over image rows    */
    uint8_t  pixel;		/* one pixel from the file  */

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (img = malloc (sizeof (*img))) ||
	NULL != (img->img = NULL) || /* false clause for initialization */
	1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
	MAX_OBJECT_WIDTH < img->hdr.width ||
	MAX_OBJECT_HEIGHT < img->hdr.height ||
	NULL == (img->img = malloc 
		 (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
	if (NULL != img) {
	    if (NULL != img->img) {
	        free (img->img);
	    }
	    free (img);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; img->hdr.width > x; x++) {

	    /* 
	     * Try to read one 8-bit pixel.  On failure, clean up and 
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (img->img);
		free (img);
	        (void)fclose (in);
		return NULL;
	    }

	    /* Store the pixel in the image data. */
	    img->img[img->hdr.width * y + x] = pixel;
	}
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}


/* 
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;	/* input file               */
    photo_t* p = NULL;	/* photo structure          */
    uint16_t x;		/* index over image columns */
    uint16_t y;		/* index over image rows    */
    uint16_t pixel;	/* one pixel from the file  */

	/* initialize nodes of octree levels. Not sure if this syntax works. If not, use for loop */
	struct Node init;
	init.rSum=0;init.gSum=0;init.bSum=0;init.count_=0;init.parent_=0;
	struct Node octree4[8*8*8*8] = { [0 . . . (8*8*8*8)] = init }; // 4th level of octree
	struct Node octree2[8*8] = { [0 . . . (8*8)] = init }; // 4th level of octree


    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (p = malloc (sizeof (*p))) ||
	NULL != (p->img = NULL) || /* false clause for initialization */
	1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
	MAX_PHOTO_WIDTH < p->hdr.width ||
	MAX_PHOTO_HEIGHT < p->hdr.height ||
	NULL == (p->img = malloc 
		 (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
	if (NULL != p) {
	    if (NULL != p->img) {
	        free (p->img);
	    }
	    free (p);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) {

		/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++) {

			/* 
			* Try to read one 16-bit pixel.  On failure, clean up and 
			* return NULL.
			*/
			if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
			free (p->img);
			free (p);
				(void)fclose (in);
			return NULL;

			}
			/* 
			* 16-bit pixel is coded as 5:6:5 RGB (5 bits red, 6 bits green,
			* and 5 bits blue).
			*/
			// expression evaluates to 0000RRRRGGGGBBBB, which will be the index into level 4 of the octree
			uint16_t index = ((((pixel>>12)&0xF)<<8)|(((pixel>>7)&0xF)<<4)|((pixel>>1)&0xF)); 
			/* 12=start of 4 red msbs. xF=masks shifted red and dont care bits. 8=makes space for 8 G and B bits. 
			7=start of 4 green msbs. xF=masks shifted red and dont care bits. 4=makes space for 4 B bits. 
			1=start of 4 blue msbs. xF=masks shifted red,green, and dont care bits.  */

			octree4[index].count_++; 
			octree4[index].rSum_+=(pixel>>11)&0x001F; //11=start of R bits. x001F=masks first 5 bits
			octree4[index].gSum_+=(pixel>>5)&0x003F; //5=start of R bits. x003F=masks first 6 bits
			octree4[index].bSum_+=pixel&0x001F; //x001F=masks first 5 bits

			// expression evaluates to 0000000000RRGGBB, which will be the index into level 2 of the octree
			uint16_t index2 = ((((pixel>>14)&0x3)<<4)|(((pixel>>9)&0x3)<<2)|((pixel>>3)&0x3));
			/* 14=start of 2 red msbs. x3=masks shifted red and dont care bits. 4=makes space for 4 G and B bits. 
			9=start of 2 green msbs. x3=masks shifted red and dont care bits. 2=makes space for 2 B bits. 
			3=start of 2 blue msbs. x3=masks shifted red,green, and dont care bits. */
			octree4[index].parent_ = index2;
		
			octree2[index2].count_++; 
			octree2[index2].rSum_+=(pixel>>11)&0x001F; //11=start of R bits. x001F=masks first 5 bits
			octree2[index2].gSum_+=(pixel>>5)&0x003F; //5=start of R bits. x003F=masks first 6 bits
			octree2[index2].bSum_+=pixel&0x001F; //x001F=masks first 5 bits
			octree2[index2].parent_ = 0;
		} 
    }
	// now sort level 4. When determining if an rgb pixel lies in the 128 level 4 palette values, I will iterate
	// through the first 128 values of octree4 and compare to each. 
	qsort(octree4, (8*8*8*8), sizeof(Node), cmpfunc);

	int i;
	for (i=0;i<128;i++){ // code to remove top 128 4th level nodes contribution from second level. 128=top 128 
	// highest count level 4 nodes
		octree2[octree4[i].parent_].rSum_ -= octree4[i].rSum_;
		octree2[octree4[i].parent_].gSum_ -= octree4[i].gSum_;
		octree2[octree4[i].parent_].bSum_ -= octree4[i].bSum_;
		octree2[octree4[i].parent_].count_ -= octree4[i].count_;
	}

	// fill array of new pallete data with consecutive rgb values
	for (i=0;i<(3*192);i++){ // 3=bytes of palette value. 64=space for level 2 palette values. 192=end of array
		if (i<(3*64)){
			if (i%3 = 0)
				*(p->palette+i) = octree2[i/3].rSum_/octree2[i/3].count_;
			if (i%3 = 1)
				*(p->palette+i) = octree2[i/3].gSum_/octree2[i/3].count_;
			if (i%3 = 2)
				*(p->palette+i) = octree2[i/3].bSum_/octree2[i/3].count_;
		} else {
			if (i%3 = 0)
				*(p->palette+i) = octree4[(i/3)-64].rSum_/octree4[(i/3)-64].count_;
			if (i%3 = 1)
				*(p->palette+i) = octree4[(i/3)-64].gSum_/octree4[(i/3)-64].count_;
			if (i%3 = 2)
				*(p->palette+i) = octree4[(i/3)-64].bSum_/octree4[(i/3)-64].count_;
		}
	}

	rewind(in); // I think this resets the filestream pointer back to the start of the file

	// writes image data of returned photo
	for (y = p->hdr.height; y-- > 0; ) {
		/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++) {
			if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
				free (p->img);
				free (p);
					(void)fclose (in);
				return NULL;
			}
			p->img[p->hdr.width * y + x] = determinePaletteValue(pixel);
		} 
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return p;
}

int cmpfunc (const void * a, const void * b) { // with this compare, highest count nodes will be in front
   return (((struct Node*)b)->count_ - ((struct Node*)a)->count_);
}

uint8_t determinePaletteValue(uint16_t & pixel, uint8_t * palette){
	int i; int j;
	for (i = 0; i < 192; i++){ // 192 =size of newly defined palette section
		/* 16-bit pixel is coded as 5:6:5 RGB (5 bits red, 6 bits green,
		and 5 bits blue). */
		// if we find a match for this RGB value in the palette
		if (palette[i*3] == ((pixel>>11)&0x001F)){ //11=start of R bits. x001F=masks first 5 bits
			if (palette[(i*3)+1] == ((pixel>>5)&0x003F)){ //5=start of R bits. x003F=masks first 6 bits
				if (palette[(i*3)+2] == (pixel&0x001F)){ //x001F=masks first 5 bits
					return i + 64; //64=offset from start of palette to skip colors for status bar
				}
			}
		} else { // else we return it's level 2 parent
			// expression evaluates to 00RRGGBB, which will be the index into level 2 of the octree
			return (uint8_t)((((pixel>>14)&0x3)<<4)|(((pixel>>9)&0x3)<<2)|((pixel>>3)&0x3)) + 64;
			/* 64=offset from start of palette to skip colors for status bar 
			14=start of 2 red msbs. x3=masks shifted red and dont care bits. 4=makes space for 4 G and B bits. 
			9=start of 2 green msbs. x3=masks shifted red and dont care bits. 2=makes space for 2 B bits. 
			3=start of 2 blue msbs. x3=masks shifted red,green, and dont care bits. */
		}
	}
}

