/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
  /* TODO */
  list_init(&wclist->lst);
  //printf("word_count.c: init \n");

}

size_t len_words(word_count_list_t *wclist) {
  /* TODO */
  //printf("word_count.c: len ");
  return list_size(&wclist->lst);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list_elem *e;
  //printf("word_count.c: find_word ");
  
  for (e = list_begin (&wclist->lst); e != list_end (&wclist->lst);
           e = list_next (e)) {
          word_count_t *wc = list_entry (e, word_count_t, elem);
          //printf("word_count.c: find_word_for %d ", e);
          if (strcmp(word, wc->word) == 0) {
            //printf("word_count.c: find_word_for %s/n ", word);
            return wc;
            
          }
          
        }
  return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  //printf("word_count.c: add_word \n ");
  pthread_mutex_lock(&wclist->lock); 
  word_count_t *wc = find_word(wclist, word);
  //printf("word_count.c: add_word: after calling find_word \n");
  
  if (wc != NULL) {
    wc->count++;
    free(word); //??
  } else if ((wc = malloc(sizeof(word_count_t))) != NULL) {
    wc->word = word;
    wc->count = 1;
    //printf("word_count.c: add_word before length %ld ", len_words(wclist));
    list_push_front(&wclist->lst, &wc->elem);
    //printf("word_count.c: add_word after length %ld ", len_words(wclist));
  } else {
    perror("malloc");
  }
  pthread_mutex_unlock(&wclist->lock);
  return wc;
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
  /* TODO */
  
  //printf("word_count.c: find_word_for length %ld ", len_words(wclist));
  struct list_elem *e;
  for (e = list_begin (&wclist->lst); e != list_end (&wclist->lst);
    e = list_next (e)) {
    word_count_t *wc = list_entry (e, word_count_t, elem);
          //printf("word_count.c: fprint_word : inside for loop ");
    fprintf(outfile, "%8d\t%s\n", wc->count, wc->word);
          
  }
}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
  /* TODO */
  word_count_t *wc1 = list_entry (ewc1, word_count_t, elem);
  word_count_t *wc2 = list_entry (ewc2, word_count_t, elem);
  //cast aux
  //printf("word_count.c: less_lst");
  // ((bool (const word_count_t *, const word_count_t *)())aux)
  
  return ((bool(*)(const word_count_t *, const word_count_t *)) aux)(wc1, wc2);

}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
  /* TODO */
  pthread_mutex_lock(&wclist->lock); 
  list_sort(&wclist->lst, less_list, less);
  pthread_mutex_unlock(&wclist->lock);
}
