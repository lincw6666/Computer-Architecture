#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include "cachelab.h"

# define MAX_BUF_LEN (1024)
# define ADDR_BITS (64UL)
# define __GET_VAL(addr, offset, mask) ((addr >> offset) & mask)

/* Variables for address. */
// Block offset
static uint64_t BLOCK_BITS;
// Index
static uint64_t INDEX_BITS;
#define ENTRY_NUM (1UL << INDEX_BITS)
#define INDEX_MASK (ENTRY_NUM - 1)
#define INDEX_OFFSET (BLOCK_BITS)
#define GET_ADDR_INDEX(addr) __GET_VAL(addr, INDEX_OFFSET, INDEX_MASK)
// Way
static uint64_t WAY_BITS;
#define WAY_NUM (1UL << WAY_BITS)
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

struct cache_entry_struct {
	uint64_t cache_entry;
	struct cache_entry_struct *prev;
	struct cache_entry_struct *next;
};
#define cache_entry_t struct cache_entry_struct

/* Exit when error occurs. */
static inline void invalid_input() {
	fprintf(stderr, "Invalid input!!\n");
	exit(1);
}

static inline void failed_open_file() {
	fprintf(stderr, "Can't open file %s!!\n", FILE_NAME);
	exit(1);
}

static inline void file_bad_action(const char ch) {
	fprintf(stderr, "Bad action '%c'!!\n", ch);
	exit(1);
}

/* Functions relate to parsing input file. */
static void skip_spaces(const char *buf, int *now) {
	while (*now < MAX_BUF_LEN && buf[*now] == ' ')
		(*now)++;
	if (*now >= MAX_BUF_LEN) {
		fprintf(stderr, "Bad content!!\n");
		exit(1);
	}
}

static inline bool is_valid_hex(const char hex) {
	return ('0' <= hex && hex <= '9') \
		|| ('a' <= hex && hex <= 'f') \
		|| ('A' <= hex && hex <= 'F');
}

static uint64_t hex2uint64(const char hex) {
	if ('0' <= hex && hex <= '9')
		return (uint64_t)hex - '0';
	else if ('a' <= hex && hex <= 'f')
		return (uint64_t)hex - 'a' + 10;
	else if ('A' <= hex && hex <= 'F')
		return (uint64_t)hex - 'A' + 10;
	return 0;
}

static uint64_t get_address(const char *buf, int now) {
	uint64_t ret = 0;

	while (now < MAX_BUF_LEN && is_valid_hex(buf[now])) {
		ret = (ret << 4) + hex2uint64(buf[now]);
		now++;
	}

	return ret;
}

/* Cache mechanism */
//static bool is_cache_hit(const cache_entry_t ** const cache, const uint64_t address);

int main(int argc, char** argv)
{
	cache_entry_t **cache, *lru_head;
	FILE *fp;
	char ch, buf[MAX_BUF_LEN];

	/* Get inputs. */
	if (argc != 9) invalid_input();
	// Get number of set index bits.
	if ((ch = getopt(argc, argv, "s:")) != -1) INDEX_BITS = atoi(optarg);
	else invalid_input();
	if ((ch = getopt(argc, argv, "E:")) != -1) WAY_BITS = atoi(optarg);
	else invalid_input();
	if ((ch = getopt(argc, argv, "b:")) != -1) BLOCK_BITS = atoi(optarg);
	else invalid_input();
	if ((ch = getopt(argc, argv, "t:")) != -1) FILE_NAME = optarg;
	else invalid_input();

	/* Allocate cache memory. */
	cache = (cache_entry_t **)malloc(sizeof(cache_entry_t) * ENTRY_NUM);
	lru_head = (cache_entry_t *)malloc(sizeof(cache_entry_t) * ENTRY_NUM);
	for (int i = 0; i < ENTRY_NUM; i++) {
		cache[i] = (cache_entry_t *)malloc(sizeof(cache_entry_t) * WAY_NUM);
		// Initialize cache entries.
		for (int j = 0; j < WAY_NUM; j++) {
			cache[i][j].prev = NULL;
			cache[i][j].next = NULL;
		}
		lru_head[i].prev = cache[i];	// The most recently used cache entry.
		lru_head[i].next = cache[i];	// The least recently used cache entry.
	}

	/* Parse the input file. */
	if (!(fp = fopen(FILE_NAME, "r"))) failed_open_file();
	while (fgets(buf, MAX_BUF_LEN, fp)) {
		int now_id_in_buf = 0;
		char now_action;		// 'S', 'L' or 'M'
		uint64_t address;

		// Skip 'I' instructions, which start with a space in the line.
		if (buf[0] != ' ') continue;

		// Get action: 'S', 'L' or 'M'.
		skip_spaces(buf, &now_id_in_buf);
		now_action = buf[now_id_in_buf++];
		// Get address.
		skip_spaces(buf, &now_id_in_buf);
		address = get_address(buf, now_id_in_buf);
		switch (now_action) {
			// Store and Load do the same action.
			case 'S':
			case 'L':
				break;
			// Modify
			case 'M':
				break;
			// Something wrong in the input file.
			default:
				file_bad_action(now_action);
		}
		printf("%c, %lu\n", now_action, address);
	}
    
	printSummary(0, 0, 0);

	/* Close the opened file. */
	fclose(fp);

	/* Free cache memory. */
	for (int i = 0; i < ENTRY_NUM; i++) {
		free(cache[i]);
	}
	free(cache);
	free(lru_head);

    return 0;
}
