/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"


struct wclist_and_file
  {
    char* filename;    
    word_count_list_t *wc_list;   
  };

void *threadfun(void *wc_file) {
  //printf("pwords.c: before openfile \n");
  
  char* file = ((struct wclist_and_file*) wc_file)->filename;
  //printf("file %s \n",file);
  word_count_list_t *word_counts = ((struct wclist_and_file*) wc_file)->wc_list;

  FILE *infile = fopen((char*)file, "r");
  if (infile == NULL) {
    perror("fopen");
    return NULL;
  }
  //printf("pwords.c: before count_words \n");
  count_words(word_counts, infile);
  //printf("pwords.c: after count_words \n");
  fclose(infile);
  pthread_exit(NULL);
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    /* TODO */
    int nthreads = argc - 1;
    //printf("pwords.c: nthreads %d \n", nthreads);
    pthread_t threads[nthreads];
    pthread_mutex_init(&word_counts.lock, NULL);
    int t;
    int rc;
    
    //init_words(&gword_counts);
    for(t = 1; t < argc; t++) {
      struct wclist_and_file *wc_file;
      wc_file = (struct wclist_and_file*) malloc(sizeof(struct wclist_and_file));
    
      wc_file->filename = argv[t];
      wc_file->wc_list = &word_counts;

      //printf("pwords.c for loop: t %d \n", t);
      rc = pthread_create(&threads[t - 1], NULL, threadfun, (void *)wc_file);
      //printf("pwords.c: pthread_create  \n");
      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }
    int i;
    for (i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
  }
  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  //printf("pwords.c: aftersort \n ");
  fprint_words(&word_counts, stdout);
  //printf("pwords.c: after print  \n "); 
  pthread_exit(NULL);
}
