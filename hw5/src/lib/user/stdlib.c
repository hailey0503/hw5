#include <stdlib.h>
#include <syscall.h>

void*
malloc (size_t size)
{
  /* Homework 5, Part B: YOUR CODE HERE */
  
  return sbrk(size);
}

void free (void* ptr)
{
  /* Homework 5, Part B: YOUR CODE HERE */
  
}

void* calloc (size_t nmemb, size_t size)
{
  /* Homework 5, Part B: YOUR CODE HERE */
  return NULL;
}

void* realloc (void* ptr, size_t size)
{
  /* Homework 5, Part B: YOUR CODE HERE */
  return NULL;
}
