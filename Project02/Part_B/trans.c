/*************************************************
 * Project 02: Part B
 *
 * Name:		林正偉
 * Student ID:	716030220009
 ************************************************/


/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int block_i, block_j, sub_block_i, sub_block_j, row, col, diagonal_pos;

	/* N X 32 matrix. */
	if (N == 32 && M == 32) {
		/*
		 * Divide the 32 X 32 matrix into several 8 X 8 blocks.
		 * For N which is not the multiple of 8, divide the matrix from
		 * row 0 to N-(N%8)-2 into several 8 X 8 blocks. Then, divide
		 * the remaining part into some (N%8) X 8 blocks.
		 */
		for (block_j = 0; block_j < M; block_j += 8) {
			for (block_i = 0; block_i < N; block_i += 8) {
				/* 
				 * Do the matrix transpose for each block.
				 * Beware of the termination condition of 'row'. If N
				 * is not the multiple of 8, row + 8 will exceed the 
				 * boundary, which causes segmentation fault. Thus, we
				 * add an extra condition 'row < N' to avoid it.
				 */
				for (row = block_i; row < block_i+8; row++) {
					for (col = block_j; col < block_j+8; col++) {
						/*
						 * The element on the diagonal will casue an extra miss
						 * because the data in A and B occupy the same cache entry.
						 *
						 * We can avoid this if the block in B has a neighbor
						 * block, e.x. B[row][col~col+7] and B[row][col+8~col+15].
						 * Store the transpose block A into the neighbor block of
						 * B. Then, store the data back to the correct block in B.
						 *
						 * Condition 'block_i+15 < N' is used for checking that
						 * does the block in B has a neighbor block.
						 */
						if ((block_i == block_j) && (block_i+15 < N)) {
							B[col][row+8] = A[row][col];
						}
						/*
						 * If the diagonal block in B does'nt have a neighbor block,
						 * deal with the element on the diagonal after accessing all
						 * the other elements in the row.
						 */
						else if (row == col) {
							diagonal_pos = row;
						}
						else {
							B[col][row] = A[row][col];
						}
					}
					/*
					 * row == col is possible only when block_i == block_j.
					 * Do it if the diagonal block doesn't have a neighbor block.
					 */
					if ((block_i == block_j) && (block_i+15 >= N))
						B[diagonal_pos][diagonal_pos] = A[diagonal_pos][diagonal_pos];
				}
				/* Deal with the diagonal block with neighbor block. */
				if ((block_i == block_j) && (block_i+15 < N)) {
					for (row = block_i; (row < block_i+8); row++) {
						for (col = block_j; col < block_j+8; col++) {
							B[col][row] = B[col][row+8];
						}
					}
				}
			}
		}
	}
	/* 64 X 64 matrix. */
	else if (N == 64 && M == 64) {
		/* Divide the matrix into several 8 X 8 big blocks. */
		for (block_j = 0; block_j < M; block_j += 8) {
			for (block_i = 0; block_i < N; block_i += 8) {
				/* If the block has a neighbor block. */
				if (block_i+15 < N) {
					for (row = block_i; row < block_i+4; row++) {
						for (col = block_j; col < block_j+8; col++) {
							/*
							 * Transpose the upper left of A's block to the upper left of B's 
							 * block.
							 */
							if (col-block_j < 4) {
								if (row == col) {
									diagonal_pos = row;
								}
								else {
									B[col][row] = A[row][col];
								}
							}
							/*
							 * Transpose the upper right of A's block to the upper left of B's
							 * neighbor block.
							 */
							else {
								if (row == col-12) {
									diagonal_pos = row;
								}
								else {
									B[col-4][row+8] = A[row][col];
								}
							}
						}
						if (block_i == block_j)
							B[diagonal_pos][diagonal_pos] = A[diagonal_pos][diagonal_pos];
						else if (block_i == block_j-8)
							B[diagonal_pos+8][diagonal_pos+8] = A[diagonal_pos][diagonal_pos+12];
					}
					for (row = block_i+4; row < block_i+8; row++) {
						for (col = block_j; col < block_j+8; col++) {
							/*
							 * Transpose the bottom left of A's block to the bottom left of B's
							 * block.
							 */
							if (col-block_j < 4) {
								if (row-4 == col) {
									diagonal_pos = row;
								}
								else {
									B[col][row] = A[row][col];
								}
							}
							/*
							 * Transpose the bottom right of A's block to the upper right of B's
							 * neighbor block.
							 */
							else {
								if (row == col-8) {
									diagonal_pos = row;
								}
								else {
									B[col-4][row+8] = A[row][col];
								}
							}
						}
						if (block_i == block_j)
							B[diagonal_pos-4][diagonal_pos] = A[diagonal_pos][diagonal_pos-4];
						else if (block_i == block_j-8)
							B[diagonal_pos+4][diagonal_pos+8] = A[diagonal_pos][diagonal_pos+8];
					}
					
					/* Copy the upper part of B's neighbor block to B's block. */
					for (col = block_j+4; col < block_j+8; col++) {
						for (row = block_i; row < block_i+8; row++) {
							B[col][row] = B[col-4][row+8];
						}
					}
				}
				else {
					/* Divide the big block into 4 4 X 4 small blocks. */
					for (sub_block_j = block_j; sub_block_j < block_j+8; sub_block_j += 4) {
						for (sub_block_i = block_i; sub_block_i < block_i+8; sub_block_i += 4) {
							/* The logic in the small block is the same as M = 32 */
							for (row = sub_block_i; (row < sub_block_i+4) && (row < N); row++) {
								for (col = sub_block_j; col < sub_block_j+4; col++) {
									if ((row-sub_block_i) == (col-sub_block_j)
										&& (block_i == block_j)) 
									{
										diagonal_pos = row - sub_block_i;
									}
									else {
										B[col][row] = A[row][col];
									}
								}
								if (block_i == block_j)
									B[sub_block_j+diagonal_pos][sub_block_i+diagonal_pos] = \
										A[sub_block_i+diagonal_pos][sub_block_j+diagonal_pos];
							}
						}
					}
				}
			}
		}
	}
	/* Other matrix size. */
	else {
		for (block_i = 0; block_i < N; block_i += 8) {
			for (block_j = 0; block_j < M; block_j += 8) {
				for (row = block_i; (row < block_i+8) && (row < N); row++) {
					for (col = block_j; (col < block_j+8) && (col < M); col++) {
						if (row == col) {
							diagonal_pos = row;
						}
						else {
							B[col][row] = A[row][col];
						}
					}
					if (block_i == block_j)
						B[diagonal_pos][diagonal_pos] = A[diagonal_pos][diagonal_pos];
				}
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);	
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

