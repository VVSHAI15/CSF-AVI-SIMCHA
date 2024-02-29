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
 	
	movl    IMAGE_WIDTH_OFFSET(%rdi), %r8d /*image now pointing to width  */
 	cmp		$0, %esi	/*compare y to 0 */
 	jl		.out_of_bounds		/*jump to out_of_bounds if y < 0 */
 	cmp		$0, %edx	/*compare x to 0 */
 	jl		.out_of_bounds
 	cmp		%r8d, %esi		/*comparing y to image width */
 	jge		.out_of_bounds  /*Jump if y >= img->width */
 	movl    IMAGE_HEIGHT_OFFSET(%rdi), %r8d/*image now pointing to height */
	cmp     %r8d, %edx     /*compre x to image height */
	jge		.out_of_bounds
	mov      $1, %rax
	ret

  .out_of_bounds:
    mov     $0, %rax
	ret


/* Function compute_index
 * Compute the index of the Data array to edit
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate (pixel column)
 *   %edx     - y coordinate (pixel row)
 *
 * Returns:
 *   %rax - The Index
 */
	.globl compute_index
compute_index:
	movl 	IMAGE_WIDTH_OFFSET(%rdi), %r8d /*pass in image width */
	imul    %edx, %r8d /*y is the third parameter, multiplied by the image */
	add     %esi, %r8d /*/x is the second parameter, added to the image */
	mov     %r8d, %eax 
	ret


/* Function get_r
 * Gets the red component of a color
 *
 * Parameters:
 *   %rdi     - Color pointer 
 *
 * Returns:
 *   %rax - The red component 
 */
	.globl get_r
get_r:
  pushq   %rbp 
  movq    %rsp, %rbp
	mov     %edi, %eax /*pass in parameter */
	shr     $24, %eax /*shift right 24 bits */
  popq    %rbp
	ret

/* Function get_g
 * Gets the green component of a color
 *
 * Parameters:
 *   %rdi     - Color pointer 
 *
 * Returns:
 *   %rax - The Green component 
 */
		.globl get_g
get_g:
  pushq   %rbp 
  movq    %rsp, %rbp
	mov     %edi, %eax
	shr     $16, %eax
  popq    %rbp
	ret

/* Function get_b
 * Gets the blue component of a color
 *
 * Parameters:
 *   %rdi     - Color pointer 
 *
 * Returns:
 *   %rax - The blue component 
 */
		.globl get_b
get_b:
  pushq   %rbp 
  movq    %rsp, %rbp
	mov     %edi, %eax
  shr     $8, %eax
  popq    %rbp
	ret

/* Function get_a
 * Gets the alpha component of a color
 *
 * Parameters:
 *   %rdi     - Color pointer 
 *
 * Returns:
 *   %rax - The alpha component 
 */
		.globl get_a
get_a:
  pushq   %rbp 
  movq    %rsp, %rbp
	mov     %edi, %eax
  popq    %rbp
	ret

/* Function blend_components 
 * Blends foreground and background color components together
 *
 * Parameters:
 *   %edi     - Fg component
 *   %esi     - Bg component 
 *   %edx     - alpha
 *
 * Returns:
 *   %rax - The blended component
 */
		.globl blend_components
blend_components:
  pushq   %rbp 
  movq    %rsp, %rbp
  mov      %edx, %r10d /* //moving alpha into r10d */ 
  imul     %edi, %r10d /* //alpha * fg, or 3rd paramater * 1st parameter  */
  movl     $255, %ecx  /* move 255 into ecx*/
  sub      %edx, %ecx /* //255 - alpha */
  imul     %esi, %ecx /* /multiply sum from above by background */
  movl     %ecx, %eax /* store 255 - alpha * bg into eax */
  add     %r10d, %eax /*/add the sums*/
  movl    $255, %r10d 
  movl    $0, %edx  /* need to move 0 into rdx for idiv */
  div     %r10d 
  popq    %rbp
  ret

/* Function blend_colors
 * Blends the foreground and background colors together
 *
 * Parameters:
 *   %edi     - Fg color
 *   %esi     - Bg color
 *
 * Returns:
 *   %rax - The blended color 
 */
		.globl blend_colors
blend_colors:
/* Save the registers that we will use */
    pushq     %rbp /* push base pointer onto the stack */
    movq    %rsp, %rbp  /* move stack pointer to bp */
    pushq   %r12 /* will serve as calle saved register */
    pushq   %rbx /* will also serve as a calle saved register */
    subq    $24, %rsp /*allocate the space on the stack */


    movl    %edi, -36(%rbp) /*store fg (first arg) as a local variable */
    movl    %esi, -40(%rbp) /* store bg (second arg) as a local variable */
    
    /*BLEND RED*/
    movl    -36(%rbp), %eax /*move fg into eax*/
    movl    %eax, %edi /* prep fg for get_a by moving into edi*/
    call    get_a  /*call get a to get alpha componenet*/
    andl    $0xFF, %eax /*isolate the lower 8 bits of eax*/
    movl    %eax, %r12d

    movl    -40(%rbp), %eax  /*move bg in eax*/
    movl    %eax, %edi  /*prep bg for get_r by moving into edi*/
    call    get_r  /*call get_r */
    andl    $0xFF, %eax /*isolate the lower 8 bits of eax*/
    movl    %eax, %ebx

    movl    -36(%rbp), %eax /*move fg into eax*/
    movl    %eax, %edi
    call    get_r
    andl    $0xFF, %eax
 

    movl    %r12d, %edx /*Alpha value*/
    movl    %ebx, %esi  /*Background Red*/
    movl    %eax, %edi  /*Foreground Red*/
    call    blend_components
    movl    %eax, -17(%rbp) /*Store red*/

    /*BLEND GREEN*/ /*(Comments are the same)*/
    movl    -36(%rbp), %eax 
    movl    %eax, %edi
    call    get_a
    andl    $0xFF, %eax
    movl    %eax, %r12d 

    movl    -40(%rbp), %eax
    movl    %eax, %edi
    call    get_g
    andl    $0xFF, %eax
    movl    %eax, %ebx 


    movl    -36(%rbp), %eax
    movl    %eax, %edi
    call    get_g
    andl    $0xFF, %eax

    movl    %r12d, %edx
    movl    %ebx, %esi
    movl    %eax, %edi
    call    blend_components
    movl    %eax, -23(%rbp)

     /*BLEND BLUE*/  /*(Comments are the same)*/
    movl    -36(%rbp), %eax
    movl    %eax, %edi
    call    get_a
    andl    $0xFF, %eax
    movl    %eax, %r12d 

    movl    -40(%rbp), %eax
    movl    %eax, %edi
    call    get_b
    andl    $0xFF, %eax
    movl    %eax, %ebx 

    movl    -36(%rbp), %eax
    movl    %eax, %edi
    call    get_b
    andl    $0xFF, %eax
    
    movl    %r12d, %edx
    movl    %ebx, %esi
    movl    %eax, %edi
    call    blend_components
    movl    %eax, -27(%rbp)

    /* Assemble final color */
    movl  -17(%rbp), %eax /*move red into eax*/
    shll    $24, %eax  /*shift red to the correct bit position */
    movl    %eax, %edx  /*move shifted red into new color storage */

    movl  -23(%rbp), %eax /*move green into eax*/
    shll    $16, %eax /*shift green to the correct bit position */
    orl     %eax, %edx /*move shifted green into new color storage but due to combination, use or*/

    movl  -27(%rbp), %eax /*move blue into eax*/
    shll    $8, %eax /*shift blue to the correct bit position */
    orl     %edx, %eax /*combine blue and the rest via bitwise or*/
    orl     $0xFF, %eax /*ensure alpha is fully opaque */
    
    /*free everything else*/
    addq    $24, %rsp 
    popq    %rbx
    popq    %r12
    popq    %rbp
    ret

/* Function set_pixel
 * Sets a specific pixel in the Data array to a given color, blending it with the previous color 
 *
 * Parameters:
 *   %rdi     - img pointer 
 *   %rsi     - in
 *   %rdx     - color
 *
 * Returns:
 *  Nothing
 */
		.globl set_pixel
set_pixel:
    pushq %rbp                   
    movq %rsp, %rbp
    subq $8, %rsp
    pushq %r12
    pushq %rdx                  // Save the new color value on the stack for later
    
    mov 8(%rdi), %rax           // Load the address of the data array into %rax
    lea (%rax,%rsi,4), %rax     // find the adress of the old color 
    mov (%rax), %esi
    mov %rax, %r12             // store the old color in r12 
    mov %edx, %edi
    call blend_colors          // Blend the colors 
    movl %eax, (%r12)          // Finally, put the blended color back at data[index]

    popq %rdx
    popq %r12
    add $8, %rsp
    popq %rbp                   
    ret

/* Function square
 * Squares a given value
 *
 * Parameters:
 *   %rdi     - x
 *
 * Returns:
 *   %rax - The squared value 
 */
	.globl square
square:
	movq %rdi, %rax    // Move x into rax
    imulq %rax, %rax   // Multiply rax by itself
	ret				   // Return the result in rax
	
/* Function square_dist 
 * calculates the square distance between two points 
 *
 * Parameters:
 *   %rdi     - x1
 *   %rsi     - y1 
 *   %rdx     - x2
 *   %rcx     - y2
 *
 * Returns:
 *   %rax - The squared distance 
 */
		.globl square_dist
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
    pushq %rbp                   // Save base pointer
    movq %rsp, %rbp              // Establish stack frame

    // Save registers that we will modify and need to be preserved
    subq $8, %rsp
    pushq %rdi
    pushq %r12
    pushq %r13
    pushq %r14

    mov %rsi, %r12
    mov %rdx, %r13
    mov %rcx, %r14

    // First we call in_bounds to check if the pixel is within the image bounds
    call in_bounds               // Check the bounds
    cmpl  $0, %eax            // Test the return value
    jz .Lpixel_out_of_bounds     // Jump if out of bounds

    // Next, we compute the index for the pixel
    movl %r12d, %esi              
    movl %r13d, %edx           
    call compute_index           // Compute index
    movl %eax, %esi              // Move computed index to %esi to call set_pixel

    // Prepare to call set_pixel
    movl %r14d, %edx           
    call set_pixel               // Set the pixel

.Lpixel_out_of_bounds:
    // Restore registers
    popq %r14
    popq %r13
    popq %r12
    popq %rdi
    addq $8, %rsp

    popq %rbp                    
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
  pushq   %rbp                              // Save base pointer
  movq    %rsp, %rbp                        // Set base pointer to current stack pointer
  subq    $48, %rsp                         // Allocate space for local variables

  movq    %rdi, -24(%rbp)                   // Store Image pointer
  movq    %rsi, -32(%rbp)                   // Store Rect pointer
  movl    %edx, -36(%rbp)                   // Store color value
  movq    -32(%rbp), %rax                   // Load Rect pointer
  movl    4(%rax), %eax                     // Load Rect.y into %eax
  movl    %eax, -4(%rbp)                    // Store Rect.y in local variable
  jmp     .Lcheck_height_condition          // Jump to height check loop

.Lstart_height_loop:                              // Start of loop for rectangle height
  movq    -32(%rbp), %rax                   // Load Rect pointer
  movl    (%rax), %eax                      // Load Rect.x into %eax
  movl    %eax, -8(%rbp)                    // Store Rect.x in local variable
  jmp     .Lcheck_width_condition           // Jump to width check loop

.Lstart_width_loop:                               // Start of loop for rectangle width
  movl    -36(%rbp), %ecx                   // Load color value into %ecx
  movl    -4(%rbp), %edx                    // Load current y coordinate into %edx
  movl    -8(%rbp), %esi                    // Load current x coordinate into %esi
  movq    -24(%rbp), %rax                   // Load Image pointer into %rax
  movq    %rax, %rdi                        // Set Image pointer as first argument
  movl    $0, %eax                          // Zero out %eax (for some calling conventions)
  call    draw_pixel                        // Call draw_pixel function
  addl    $1, -8(%rbp)                      // Increment x coordinate

.Lcheck_width_condition:                          // Check if current x is within rectangle width
  movq    -32(%rbp), %rax                   // Load Rect pointer
  movl    (%rax), %edx                      // Load Rect.x into %edx
  movq    -32(%rbp), %rax                   // Load Rect pointer again
  movl    8(%rax), %eax                     // Load Rect.width into %eax
  addl    %edx, %eax                        // Add Rect.x to Rect.width
  cmpl    %eax, -8(%rbp)                    // Compare with current x
  jl      .Lstart_width_loop                // If current x is less, loop

  addl    $1, -4(%rbp)                      // Increment y coordinate

.Lcheck_height_condition:                         // Check if current y is within rectangle height
  movq    -32(%rbp), %rax                   // Load Rect pointer
  movl    4(%rax), %edx                     // Load Rect.y into %edx
  movq    -32(%rbp), %rax                   // Load Rect pointer again
  movl    12(%rax), %eax                    // Load Rect.height into %eax
  addl    %edx, %eax                        // Add Rect.y to Rect.height
  cmpl    %eax, -4(%rbp)                    // Compare with current y
  jl      .Lstart_height_loop               // If current y is less, loop

  addq    $48, %rsp                         // Deallocate space for all local variables
  popq    %rbp                              // Restore old base pointer
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
  pushq %rbp                           // Save old base pointer
  movq %rsp, %rbp                        // Set new base pointer
  pushq %rbx                             // Save RBX register (used for preserving callee-saved register)
  subq $56, %rsp                         // Allocate space for local variables

  movq %rdi, -40(%rbp)                   // Store img pointer
  movl %esi, -44(%rbp)                   // Store center x coordinate
  movl %edx, -48(%rbp)                   // Store center y coordinate
  movl %ecx, -52(%rbp)                   // Store radius
  movl %r8d, -56(%rbp)                   // Store color

  // Calculate bounding box coordinates
  movl -48(%rbp), %eax                   // Load center x
  subl -52(%rbp), %eax                   // Subtract radius from center x to get start x
  movl %eax, -20(%rbp)                   // Store start x for bounding box
  jmp .Lcheck_y                          // Jump to check if current y is within the bounding box

.Literate_y:                             
  movl -44(%rbp), %eax                   // Load center y
  subl -52(%rbp), %eax                   // Subtract radius from center y to get start y
  movl %eax, -24(%rbp)                   // Store start y for bounding box
  jmp .Literate_x                        // Jump to iterate over x coordinates

  // Calculate square distance from the center to the current point
.Ldraw_check:                            
  movl -48(%rbp), %ecx                   // Load center x
  movl -44(%rbp), %edx                   // Load center y
  movl -20(%rbp), %esi                   // Load current x
  movl -24(%rbp), %eax                   // Load current y
  movl %eax, %edi                        // Set current y as the first argument for square_dist
  movl $0, %eax                          // Clear eax register (for some calling conventions)
  call square_dist                       // Call square_dist function
  movl %eax, %ebx                        // Store square distance result in ebx

  // Compare square distance with square of the radius
  movl -52(%rbp), %eax                   // Load radius
  movl %eax, %edi                        // Set radius as the first argument for square
  movl $0, %eax                          // Clear EAX register
  call square                            // Call square function to get square of the radius
  cmpl %eax, %ebx                        // Compare square of radius with square distance
  jg .Lnext_pixel                        // Jump to the next pixel if square distance is greater

  // Draw pixel if it is within the circle
  movl -56(%rbp), %ecx                   // Load color
  movl -20(%rbp), %edx                   // Load current x
  movl -24(%rbp), %esi                   // Load current y
  movq -40(%rbp), %rax                   // Load img pointer
  movq %rax, %rdi                        // Set img pointer as the first argument
  movl $0, %eax                          // Clear the eax register
  call draw_pixel                       

.Lnext_pixel:                           
  addl $1, -24(%rbp)                     // Increment current y

.Literate_x:                            
  movl -44(%rbp), %edx                   // Load center y
  movl -52(%rbp), %eax                   // Load radius
  addl %edx, %eax                        // Add radius to center y to calculate end y of bounding box
  cmpl %eax, -24(%rbp)                   // Compare current y with end y of bounding box
  jle .Ldraw_check                       // If current y is less or equal, continue to draw_check
  addl $1, -20(%rbp)                     // Increment current x

.Lcheck_y:                               
  movl -48(%rbp), %edx                   // Load center x
  movl -52(%rbp), %eax                   // Load radius
  addl %edx, %eax                        // Add radius to center x to calculate end x of bounding box
  cmpl %eax, -20(%rbp)                   // Compare current x with end x of bounding box
  jle .Literate_y                        // If current x is less or equal, continue to iterate_y
  movq -8(%rbp), %rbx                    // Restore original value of RBX

  addq $56, %rsp                         // Deallocate space for local variables
  popq %rbx                             
  popq %rbp                              // Restore old base pointer
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
  pushq   %rbp                              // Save the old base pointer
  movq    %rsp, %rbp                        // Set the new base pointer
  subq    $64, %rsp                         // Allocate space for local variables

  // Store function arguments in local variables
  movq    %rdi, -40(%rbp)                   // Store destination image pointer
  movl    %esi, -44(%rbp)                   // Store x position
  movl    %edx, -48(%rbp)                   // Store y position
  movq    %rcx, -56(%rbp)                   // Store pointer to dimensions/size
  movq    %r8, -64(%rbp)                    // Store source image/tile pointer

  // Validate width and height of the source tile to be positive
  movq    -64(%rbp), %rax
  movl    (%rax), %eax
  testl   %eax, %eax
  js      .Lskip_drawing                    // Jump if source width is negative

  movq    -64(%rbp), %rax
  movl    4(%rax), %eax
  testl   %eax, %eax
  js      .Lskip_drawing                    // Jump if source height is negative

  // Check if the source tile fits within the destination image dimensions
  movq    -64(%rbp), %rax
  movl    (%rax), %edx                      // Load source width
  movq    -64(%rbp), %rax
  movl    8(%rax), %eax                     // Load x offset
  addl    %edx, %eax                        // Calculate end x position
  movl    %eax, %edx
  movq    -56(%rbp), %rax
  movl    (%rax), %eax                      // Load destination width
  cmpl    %edx, %eax
  jb      .Lskip_drawing                    // Jump if end x is out of bounds

  movq    -64(%rbp), %rax
  movl    4(%rax), %edx                     // Load source height
  movq    -64(%rbp), %rax
  movl    12(%rax), %eax                    // Load y offset
  addl    %edx, %eax                        // Calculate end y position
  movl    %eax, %edx
  movq    -56(%rbp), %rax
  movl    4(%rax), %eax                     // Load destination height
  cmpl    %edx, %eax
  jb      .Lskip_drawing                    // Jump if end y is out of bounds

  movl    $0, -4(%rbp)                      // Set initial y loop counter

.Literate_rows:
  movl    -48(%rbp), %edx                   // Load y position
  movl    -4(%rbp), %eax                    // Load current y loop counter
  addl    %edx, %eax                        // Add y position to loop counter
  movl    %eax, -12(%rbp)                   // Store global y position
  cmpl    $0, -12(%rbp)                     // Check if global y position is valid
  js      .Lnext_row                        // Skip to next row if invalid

  movq    -40(%rbp), %rax
  movl    4(%rax), %eax                     // Load destination height
  movl    -12(%rbp), %edx                   // Load global y position
  cmpl    %eax, %edx                        // Compare global y position with destination height
  jnb     .Lnext_row                        // Skip to next row if out of bounds

  // Initialize loop variables for x-axis iteration
  movl    $0, -8(%rbp)                      // Set initial x loop counter

.Literate_columns:
  movl    -44(%rbp), %edx                   // Load x position
  movl    -8(%rbp), %eax                    // Load current x loop counter
  addl    %edx, %eax                        // Add x position to loop counter
  movl    %eax, -16(%rbp)                   // Store global x position
  cmpl    $0, -16(%rbp)                     // Check if global x position is valid
  js      .Lnext_column                     // Skip to next column if invalid

  movq    -40(%rbp), %rax
  movl    (%rax), %eax                      // Load destination width
  movl    -16(%rbp), %edx                   // Load global x position
  cmpl    %eax, %edx                        // Compare global x position with destination width
  jnb     .Lnext_column                     // Skip to next column if out of bounds

.Lnext_column:
  addl    $1, -8(%rbp)                      // Increment x loop counter
  movq    -64(%rbp), %rax
  movl    8(%rax), %eax                     // Load source width
  cmpl    %eax, -8(%rbp)                    // Check if loop counter exceeds source width
  jl      .Literate_columns                 // Continue looping over columns if not exceeded

.Lnext_row:
  addl    $1, -4(%rbp)                      // Increment y loop counter
  movq    -64(%rbp), %rax
  movl    12(%rax), %eax                    // Load source height
  cmpl    %eax, -4(%rbp)                    // Check if loop counter exceeds source height
  jl      .Literate_rows                    // Continue looping over rows if not exceeded

.Lskip_drawing:
  addq $64, %rsp                           // Deallocate space for local variables
  popq    %rbp                             // Restore the old base pointer
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
  pushq   %rbp
  movq    %rsp, %rbp
  subq    $64, %rsp
  movq    %rdi, -40(%rbp)
  movl    %esi, -44(%rbp)
  movl    %edx, -48(%rbp)
  movq    %rcx, -56(%rbp)
  movq    %r8, -64(%rbp)

  movq    -64(%rbp), %rax
  movl    (%rax), %eax
  testl   %eax, %eax
  js      .L1


  movq    -64(%rbp), %rax
  movl    (%rax), %edx
  movq    -64(%rbp), %rax
  movl    8(%rax), %eax
  addl    %edx, %eax
  movl    %eax, %edx
  movq    -56(%rbp), %rax
  movl    (%rax), %eax
  cmpl    %edx, %eax
  jb      .L1

  movl    $0, -4(%rbp)
  jmp     .L50
.L14:
  movl    -48(%rbp), %edx
  movl    -4(%rbp), %eax
  addl    %edx, %eax
  movl    %eax, -12(%rbp)
  cmpl    $0, -12(%rbp)
  js      .L8

  movq    -40(%rbp), %rax
  movl    4(%rax), %eax
  movl    -12(%rbp), %edx
  cmpl    %eax, %edx
  jnb     .L8
  movl    $0, -8(%rbp)
  jmp     .L9
.L13:
  movl    -44(%rbp), %edx
  movl    -8(%rbp), %eax
  addl    %edx, %eax
  movl    %eax, -16(%rbp)
  cmpl    $0, -16(%rbp)
  js      .L12

  movq    -40(%rbp), %rax
  movl    (%rax), %eax
  movl    -16(%rbp), %edx
  cmpl    %eax, %edx
  jnb     .L12
  
  movq    -64(%rbp), %rax
  movl    4(%rax), %edx
  movl    -4(%rbp), %eax
  addl    %edx, %eax
  movl    %eax, %edx
  movq    -56(%rbp), %rax
  movl    (%rax), %eax
  imull   %eax, %edx
  movq    -64(%rbp), %rax
  movl    (%rax), %ecx
  movl    -8(%rbp), %eax
  addl    %ecx, %eax
  addl    %edx, %eax
  movl    %eax, -20(%rbp)
  movq    -40(%rbp), %rax
  movl    (%rax), %edx
  movl    -12(%rbp), %eax
  imull   %eax, %edx
  movl    -16(%rbp), %eax
  addl    %edx, %eax
  movl    %eax, -24(%rbp)
  movq    -40(%rbp), %rax
  movq    8(%rax), %rax
  movl    -24(%rbp), %edx
  salq    $2, %rdx
  addq    %rdx, %rax
  movl    (%rax), %edx
  movq    -56(%rbp), %rax
  movq    8(%rax), %rax
  movl    -20(%rbp), %ecx
  salq    $2, %rcx
  addq    %rcx, %rax
  movl    (%rax), %eax
  movl    %edx, %esi
  movl    %eax, %edi
  movl    $0, %eax
  call    blend_colors
  movl    %eax, %ecx
  movq    -40(%rbp), %rax
  movq    8(%rax), %rax
  movl    -24(%rbp), %edx
  salq    $2, %rdx
  addq    %rdx, %rax
  movl    %ecx, %edx
  movl    %edx, (%rax)
  jmp     .L12

.L12:
  addl    $1, -8(%rbp)
.L9:
  movq    -64(%rbp), %rax
  movl    8(%rax), %eax
  cmpl    %eax, -8(%rbp)
  jl      .L13
  jmp     .L8
.L8:
  addl    $1, -4(%rbp)
.L50:
  movq    -64(%rbp), %rax
  movl    12(%rax), %eax
  cmpl    %eax, -4(%rbp)
  jl      .L14
  jmp     .L1

.L1:
    addq $64, %rsp                           // Deallocate space for local variables
    popq    %rbp                             // Restore the old base pointer
    ret