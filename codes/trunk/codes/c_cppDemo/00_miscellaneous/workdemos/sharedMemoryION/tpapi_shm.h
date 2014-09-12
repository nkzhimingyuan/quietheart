#ifndef _TPAPI_SHM_H
#define _TPAPI_SHM_H

#include "tpapi_types.h"

#ifdef __cplusplus
extern "C"{
#endif

/** @brief Shared memory handle data type*/
typedef unsigned int TPAPI_SHM_HANDLE;

/** @brief Shared memory error codes*/
#define	TPAPI_SHM_OK							(0)
#define	TPAPI_SHM_MEMORY_ALLOCATION_FAILED		(1)
#define	TPAPI_SHM_ERROR							(-1)

/** @brief Shared memory initialisation*/  
/** detail description: 
  * Shared memory initialisation */
/** @return TPAPI_SHM_OK indicates success. Otherwise, return error code.*/
extern int tpapi_shm_Init(void);

/** @brief Reverse of Shared memory init*/  
/** detail description: 
  * frees the datastructures allocated as part of  tpapi_shm_Init call*/
/** @return TPAPI_SHM_OK indicates success. Otherwise, return error code.*/
extern int tpapi_shm_Exit(void);

/** @brief Allocates memory from shared memory region */
/** detail description: */
/** @param[in] size  Amount of memory to allocate */
/** @param[in] align   Alignment boundary */
/** @param[out] shmhandle   shared memory handle */
/** @return  TPAPI_SHM_OK indicates success. Otherwise, return TPAPI_SHM_MEMORY_ALLOCATION_FAILED*/
extern int tpapi_shm_Alloc(unsigned int size, unsigned int align, TPAPI_SHM_HANDLE* shmhandle);

/** @brief Frees memory allocated with tpapi_shm_Alloc call*/
/** detail description: */
/** @param[in] shmhandle   shared memory handle returned from tpapi_shm_Alloc call*/
/** @return  TPAPI_SHM_OK indicates success. Otherwise, return TPAPI_SHM_ERROR*/
extern int tpapi_shm_Free(TPAPI_SHM_HANDLE shmhandle);

/** @brief Synchronizes main memory with Cache ( writes from cache to main memory ) */
/** detail description: */
/** @param[in] shmhandle   shared memory handle returned from tpapi_shm_Alloc call*/
/** @param[in] offset  offset from base of shmhandle */
/** @param[in] size   number of bytes */
/** @return  TPAPI_SHM_OK indicates success. Otherwise, return TPAPI_SHM_ERROR*/
extern int  tpapi_shm_Flush(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size);

/** @brief Synchronizes Cache with main memory ( writes from main memory to cache ) */
/** detail description: */
/** @param[in] shmhandle   shared memory handle returned from tpapi_shm_Alloc call*/
/** @param[in] offset  offset from base of shmhandle */
/** @param[in] size   number of bytes */
/** @return  TPAPI_SHM_OK indicates success. Otherwise, return TPAPI_SHM_ERROR*/
extern int  tpapi_shm_Invalidate(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size);

/** @brief virtual address of shmhandle*/
/** detail description: Maps the shared memory into process's address space and returns the virtual address at offset*/
/** @param[in] shmhandle   shared memory handle returned from tpapi_shm_Alloc call*/
/** @param[in] offset  offset from base of shmhandle */
/** @param[in] size  number of bytes for virtual address */
/** @param[out] virtaddr  Holds the address of mapped shared memory region */
/** @return  TPAPI_SHM_OK indicates success. Otherwise, return TPAPI_SHM_ERROR*/
extern int  tpapi_shm_GetVirtualAddress(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size, void **virtaddr);

#ifdef __cplusplus
}
#endif

#endif //_TPAPI_SHM_H
