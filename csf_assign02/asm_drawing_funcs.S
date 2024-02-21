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
    // Args: %edi = fg, %esi = bg, %edx = alpha
blend_components:
    movl %edx, %eax         // First We move alpha into eax for multiplication with fg
    imull %edi, %eax        // Now we multiply alpha * fg, the result fits in eax 
    movl %edx, %ebx         // We copy alpha to ebx so we can subtract
    subl $255, %ebx         // ebx now holds 255 - alpha
    imull %esi, %ebx        // We multiply (255 - alpha) * bg
    addl %ebx, %eax         // Now we can add the two products
    movl $255, %ebx         // We need to load 255 into ebx for division
    cltd                    // Also we need to sign extend eax into edx:eax for division
    idivl %ebx              // Finally we divide by 255 to normalize the blended component
    ret                     // Return the blended component in eax

		.globl blend_colors
	// Args: %edi = fg, %esi = bg
blend_colors:
    pushq %rbp                   
    movq %rsp, %rbp

    // Save the registers that we will use
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    // Extract alpha value from the foreground color
    movl %edi, %eax               // Copy the foreground color to %eax
    call get_a                    // Get the alpha component
    movl %eax, %r12d              // Store the alpha component in %r12d 

    // Extract and blend Red component
    movl %edi, %eax               // Move fg color to %eax to call get_r
    call get_r                    // Get the red component of the fg color
    movl %eax, %r13d              // Store result in %r13d
    movl %esi, %eax               // Move bg color to %eax to call get_r
    call get_r                    // Get the red component of the bg color
    movl %r12d, %edx              // Move the alpha value to %edx
    movl %r13d, %edi              // Move the fg red component to %edi
    movl %eax, %esi               // Move the bg red component to %esi
    call blend_components         // Blend the red components 
    movl %eax, %r13d              // Store the blended red component

    // Extract and blend Green component : Repeat the same process as red
    movl %edi, %eax             
    call get_g                    
    movl %eax, %r14d            
    movl %esi, %eax              
    call get_g                    
    movl %r14d, %edi             
    movl %eax, %esi               
    call blend_components         
    movl %eax, %r14d            

    // Extract and blend Blue component : Same as Green and Red
    movl %edi, %eax               
    call get_b                    
    movl %eax, %r15d             
    movl %esi, %eax               
    call get_b                    
    movl %r15d, %edi             
    movl %eax, %esi             
    call blend_components       
    movl %eax, %r15d             

    // Now that we have all the blended components, we can eeassemble the color
    movl $0xFF, %eax              // Start with the alpha component (We set Opacity to Fully Opaque)
    shll $8, %eax                 // Shift alpha to its correct position
    orl %r15d, %eax               // Add the blue component
    shll $8, %eax                 // Shift blue and alpha to their correct positions
    orl %r14d, %eax               // Add the green component
    shll $8, %eax                 // Shift green, blue, and alpha to their correct positions
    orl %r13d, %eax               // Add the red component

    // Restore the registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12

    popq %rbp                    
    ret                           // Return with the blended color in %eax

		.globl set_pixel
	// Args: %rdi = img, %rsi = index, %rdx = color
set_pixel:
    movq (%rdi), %rax        // We load the address of the data array into rax
    movl %edx, (%rax,%rsi,4) // We store the new color at data[index] There is a scale factor of 4 for 32-bit data.
    ret                      // Return

		.globl square
	// Args: %rdi = x
square:
	movq %rdi, %rax    // Move x into rax
    imulq %rax, %rax   // Multiply rax by itself
	ret				   // Return the result in rax
	
		.globl square_dist
	// Args: %rdi = x1, %rsi = y1, %rdx = x2, %rcx = y2
square_dist:
	movq %rdi, %rax    // First we move x1 into rax
    subq %rdx, %rax    // Next we Subtract x2, rax now holds (x1 - x2)
    imulq %rax, %rax   // Square (x1 - x2), result is stored in rax

    movq %rsi, %rdx    // We do the same process for y
    subq %rcx, %rdx    
    imulq %rdx, %rdx   

    addq %rdx, %rax    // Add the two squares together
    ret                // Return the result in rax


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
