/*									tab:8
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
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
 * Version:	    2
 * Creation Date:   Thu Sep  9 22:08:16 2004
 * Filename:	    text.h
 * History:
 *	SL	1	Thu Sep  9 22:08:16 2004
 *		First written.
 *	SL	2	Sat Sep 12 13:40:11 2009
 *		Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT 16

/* Standard VGA text font. */
extern unsigned char font_data[256][16];

/*
 * text_to_image
 *   DESCRIPTION: given a string str, return an array of bytes that is the pixel data. Does this by 
 *                  looping through each row i of the output data, inside looping through each character j in string,
 *                  inside looping through each bit k in row i of character j's bit map and writes it the output.
 *                  Calls helper function plane_order to format output to 3210 plane order before returning.
 *   INPUTS: str -- string to turn convert into pixel data
 *   OUTPUTS: none
 *   RETURN VALUE: pixel data
 *   SIDE EFFECTS: none
 */  
unsigned char* text_to_image(char* str);

/*
 * plane_order
 *   DESCRIPTION: Takes in-order bar image data in formats it into 3210 plane 
 *                  order (identical to display buffer data). Does this by looping through 4 planes (i) of out,
 *                  inside that looping through each row (j) in img/the current out plane, inside that looping
 *                  through every pixel (k) of plane i in row j of img, inside that writing pixel k to index
 *                  planeIndex of plane i of out.
 *   INPUTS: img -- image data to reformat
 *   OUTPUTS: none
 *   RETURN VALUE: reformatted bar image data
 *   SIDE EFFECTS: none
 */  
unsigned char* plane_order(unsigned char* img);

#endif /* TEXT_H */
