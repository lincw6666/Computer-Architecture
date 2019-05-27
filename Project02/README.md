Project 2:  Understanding Cache Memories
===

# Introduction

- Two parts in this project.
- **Part A**
    - we need to **implement a cache simulator**, which accept three kinds of operation: load, store and modify.
    - Cache replacement policy: **LRU**
    - **Goal**: ++calculate the number of **hit**, **miss** and **eviction**++ caused by these operations.
- **Part B**
    - **Goal**: ++lower the cache miss++ of the matrix transposition of 32 x 32, 64 x 64 and 67 x 61 matrices.

----

# Part A

- Data structure of the cache entry
    ```c=45
    typedef struct cache_entry_struct {
        uint64_t cache_entry;
        struct cache_entry_struct *prev;
        struct cache_entry_struct *next;
    } cache_entry_t;
    ```
- **Control flow**
    1. Get inputs: I, L, S or M.
        - **I**: Ignore it.
        - **L**: Load operation. Causes hit or miss.
        - **S**: Store operation. Causes hit or miss.
        - **M**: Modify: Load then Store. Three cases.
            (1) hit hit
            (2) miss hit
            (3) miss eviction hit
    2. Check cache hits or not.
    3. Update LRU list.
- **LRU** replacement policy
    - Use **double linked list** with list head.
        ```c=75
        cache_entry_t **cache, *lru_head;
        ```
    - `lru_head->next` points to the LRU cache entry.
    - `lru_head->prev` points to the MRU cache entry.
    - Related functions:
        - ***update_LRU***: Do it when a cache miss occurs.
        - ***update_MRU***: Do it every time.
        - These two functions do two operations: remove the cache entry from the list then insert it to the front or to the end of the list.

----

# Part B

- Used techniques: **Blocking** and **Neighbor block**.

## Blocking

:::info
**Goal**: Do not flush the data in the cache entry before it is no longer used
:::
- In part B, the cache block size is 32 bytes, which means that ++8 elements in the matrix share the same cache entry++.
- There are 32 entries in the cache. This tells us that **the element and the (32 x 8)th element follows it occupy the same cache entry**.
    - For example, A[0][0] to A[0][7] occupy the same cache entry as A[8][0] to A[8][7]. B[0][0] to B[0][7] and B[8][0] to B[8][7] use that cache entry, too.
- As the reasons above, I divide the matrix into **16 8 x 8** blocks.
    - The blocks on the **same row use different cache entries**.
    - The blocks on the **same column use the same cache entries**.
    - **Goal**: Avoid using the blocks on the same column.
- However, **the transposition of the block on the diagonal is undesirable**.
    - Therefore, I use a trick which I call it **neighbor block**.

----

## Neighbor Block

:::info
Store the result in the neighbor block ++if it casues extra misses during the matrix transposition++.
:::
- The block **right next to the destination block** on the ==**same row**==.
-  After the transposition, we **store** the result from the neighbor block **back** to the destination block.
    - The neighbor block is on the ++same row++ as the destination block.
    - There are no cache evictions during copying the data.
- Beware of the block **doesn't have a neighbor block**.
    - **Solution**: Deal with ++the elements on the diagonal++ **after** all the other elements on the same row are done.
    - For example, I do the transposition from A[24][25] to A[24][31] first then I deal with A[24][24].

----

## 32 x 32 Matrix Transposition

- Divide the matrix into 16 8 x 8 blocks.
    ![](https://i.imgur.com/h6Z6qzl.jpg)
- Use **neighbor block technique** for blocks **on the diagonal**. (That is block 1, 6 and 11)
- **Block 16** uses the method for blocks without neighbor block.


----

## 64 x 64 Matrix Transposition

- The block size of the cache entry is still 32 bytes so we still divide the matrix into **16 8 x 8** blocks.
- However, the **upper half** of the block occupies the **same cache entries** as the **bottom half** of the block.
    - Divide each block into **4 4 x 4** sub-blocks.
    - Just as the rule of the block, the sub-block on the same column occupy the same cache entries.
- **Sub-block together with the neighbor block**.
    ![](https://i.imgur.com/Hcnwfgc.jpg)

    - Store the transposition matrix of the right half (red and green blocks) of matrix A into the upper half of the neighbor block in matrix B.
    - Store them back to the lower half of the destination block in matrix B. 
- 5 different cases which cause extra cache misses during the transposition of 64 x 64 matrix.
    ![](https://i.imgur.com/ZCV3YC0.jpg)
    1. **The yellow block**.
        The block in A occupy the same cache memory as the destination block in B. Even though we use the neighbor block technique.
    2. **The red blocks**.
        It's almost the same as the yellow box. The difference between them is that in the red block, we add 1 extra cache miss because we ++flush the data of the top left corner of the red block in the cache before it is no longer used++.
        Due to the neighbor block technique, we have already loaded it into the cache entry. However, we will flush it and reload it during the matrix transposition of the block.
    3. **The green blocks**.
        The problem is that the block in A ++uses the same cache entry as the neighbor block in B++.
    4. **The blue blocks**.
        These blocks ++do not have a neighbor block++.
    5. **The brown block**.
        It ++doesn't have a neighbor block++ and ++it's the block on the diagonal++.

----

## Other Matrix Size

- **The neighbor block technique is useless**.
    - It'll increase the cache misses significantly.
- **Only use the blocking technique**.
    - According to the relation of the cache misses and the block size, I use 16 as the block size.
    ![](https://i.imgur.com/grokGwI.jpg)
