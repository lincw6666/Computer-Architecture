#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "cachelab.h"

# define ADDR_BITS (64UL)
# define __GET_VAL(addr, offset, mask) ((addr >> offset) & mask)

/* Variables for address. */
// Block offset
static uint64_t BLOCK_BITS;
// Index
static uint64_t INDEX_BITS;
#define INDEX_MASK ((1UL << INDEX_BITS) - 1)
#define INDEX_OFFSET (BLOCK_BITS)
#define GET_ADDR_INDEX(addr) __GET_VAL(addr, INDEX_OFFSET, INDEX_MASK)
// Way
static uint64_t WAY_BITS;
// Tag
#define TAG_BITS (ADDR_BITS - INDEX_BITS - BLOCK_BITS)
#define TAG_MASK ((1UL << TAG_BITS) - 1)
#define TAG_OFFSET (INDEX_OFFSET + INDEX_BITS)
#define GET_ADDR_TAG(addr) __GET_VAL(addr, TAG_OFFSET, TAG_MASK)

/* Variables for cache. */
#define GET_CACHE_TAG(data) __GET_VAL(data, 0, TAG_MASK)
#define IS_CACHE_VALID(data) ((data >> TAG_BITS) & 1UL)

/* File name. */
static char *FILE_NAME;

#define INVALID_INPUT { printf("Invalid input!!\n"); return -1; }

int main(int argc, char** argv)
{
	char ch;

	/* Get inputs. */
	if (argc != 9) INVALID_INPUT
	// Get number of set index bits.
	if ((ch = getopt(argc, argv, "s:")) != -1) INDEX_BITS = atoi(optarg);
	else INVALID_INPUT
	if ((ch = getopt(argc, argv, "E:")) != -1) WAY_BITS = atoi(optarg);
	else INVALID_INPUT
	if ((ch = getopt(argc, argv, "b:")) != -1) BLOCK_BITS = atoi(optarg);
	else INVALID_INPUT
	if ((ch = getopt(argc, argv, "t:")) != -1) FILE_NAME = optarg;
	else INVALID_INPUT

    printSummary(0, 0, 0);
    return 0;
}
