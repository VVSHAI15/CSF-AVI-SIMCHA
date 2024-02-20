/*
 * Assembly-language implementations of the drawing functions
 */

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/* Offsets of struct Rect fields */
#define RECT_X_OFFSET        0
#define RECT_Y_OFFSET        4
#define RECT_WIDTH_OFFSET    8
#define RECT_HEIGHT_OFFSET   12

	.section .text

/***********************************************************************
   Helper functions
   You should declare these as .globl so that
   they can be called from the unit test code.
 ***********************************************************************/

/* TODO: implement your helper functions here */
/* Function in_bounds
 * Ensure that variables x and y fall within the given bounds of the image
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate (pixel column)
 *   %edx     - y coordinate (pixel row)
 *
 * Returns:
 *   %rax - 1 if coordinates are within bounds, 0 if fail
 */


 .global in_bounds
 
 in_bounds:
 	
	movl    IMAGE_WIDTH_OFFSET(%rdi), %r8d //image now pointing to width 
 	cmp		$0, %esi	//compare y to 0
 	jl		.out_of_bounds		//jump to out_of_bounds if y < 0
 	cmp		$0, %edx	//compare x to 0
 	jl		.out_of_bounds
 	cmp		%r8d, %esi		//comparing y to image width
 	jge		.out_of_bounds  //Jump if y >= img->width
 	movl    IMAGE_HEIGHT_OFFSET(%rdi), %r8d//image now pointing to height
	cmp     %r8d, %edx     //compre x to image height
	jge		.out_of_bounds
	mov      $1, %rax
	ret

  .out_of_bounds:
    mov     $0, %rax
	ret


	.globl compute_index
compute_index:
	movl 	IMAGE_WIDTH_OFFSET(%rdi), %r8d //pass in image width
	imul    %edx, %r8d //y is the third parameter, multiplied by the image
	add     %esi, %r8d //x is the second parameter, added to the image
	mov     %r8d, %eax 
	ret

	.globl clamp
clamp:
	/* TODO: implement */
	ret


	.globl get_r
get_r:
	mov     %edi, %eax //pass in parameter 
	shr     $24, %eax//shift right 24 bits
	ret

		.globl get_g
get_g:
	mov     %edi, %eax
	shr     $16, %eax
	ret


		.globl get_b
get_b:
	mov     %edi, %eax
	shr     $8, %eax
	ret


		.globl get_a
get_a:
	mov     %edi, %eax
	ret


		.globl blend_components
blend_components:
	mov     %edi, %eax //moving fg into eax
	mul     %eax //alpha * fg, or 3rd paramater * 1st parameter
	mov     $255, %ecx
	sub     %edx, %ecx //255 - alpha
	mov     %ecx, %edx //move to allow for mul
	mul     %esi //multiply sum from above by background
	add     %edx, %eax //add the sums
	mov     $255, %r10d
	mov     $0, %edx //need to move 0 into rdx for idiv
	div     %r10d //divide
	ret

			.globl blend_colors
blend_colors:
	/* TODO: implement */
	ret


		.globl set_pixel
set_pixel:
	/* TODO: implement */
	ret

		.globl square
square:
	/* TODO: implement */
	ret
	
		.globl square_dist
square_dist:
	/* TODO: implement */
	ret


/***********************************************************************
   Public API functions
 ***********************************************************************/

/*
 * Draw a pixel.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate (pixel column)
 *   %edx     - y coordinate (pixel row)
 *   %ecx     - uint32_t color value
 */
	.globl draw_pixel
draw_pixel:
	/* TODO: implement */
	ret


/*
 * Draw a rectangle.
 * The rectangle has rect->x,rect->y as its upper left corner,
 * is rect->width pixels wide, and rect->height pixels high.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %rsi     - pointer to struct Rect
 *   %edx     - uint32_t color value
 */
	.globl draw_rect
draw_rect:
	/* TODO: implement */
	ret

/*
 * Draw a circle.
 * The circle has x,y as its center and has r as its radius.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate of circle's center
 *   %edx     - y coordinate of circle's center
 *   %ecx     - radius of circle
 *   %r8d     - uint32_t color value
 */
	.globl draw_circle
draw_circle:
	/* TODO: implement */
	ret

/*
 * Draw a tile by copying all pixels in the region
 * enclosed by the tile parameter in the tilemap image
 * to the specified x/y coordinates of the destination image.
 * No blending of the tile pixel colors with the background
 * colors should be done.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where tile should be copied
 *   %edx - y coordinate of location where tile should be copied
 *   %rcx - pointer to Image (the tilemap)
 *   %r8  - pointer to Rect (the tile)
 */
	.globl draw_tile
draw_tile:
	/* TODO: implement */
	ret

/*
 * Draw a sprite by copying all pixels in the region
 * enclosed by the sprite parameter in the spritemap image
 * to the specified x/y coordinates of the destination image.
 * The alpha values of the sprite pixels should be used to
 * blend the sprite pixel colors with the background
 * pixel colors.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where sprite should be copied
 *   %edx - y coordinate of location where sprite should be copied
 *   %rcx - pointer to Image (the spritemap)
 *   %r8  - pointer to Rect (the sprite)
 */
	.globl draw_sprite
draw_sprite:
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
