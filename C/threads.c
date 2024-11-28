#include <stdio.h>
#include <pthread.h>

void    *thread_function(void *arg);

int     main(void) {
  pthread_t     thread1;
  pthread_t       thread2;
  int           i;

  pthread_create(&thread1, NULL, thread_function, NULL);
  pthread_create(&thread2, NULL, thread_function, NULL);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("Threads finalizadas.\n");

  return 0;
}

void    *thread_function(void *arg) {
  int       i;

  for (i = 1; i <= 10; i++) {
    printf("Thread ID: %ld, Número: %d\n", pthread_self(), i);
  }

  pthread_exit(NULL);
}
