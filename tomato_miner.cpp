// Bounty Cycle, an attempt to disprove John's claims

#include "tomato_miner.h"
#include <unistd.h>

int main(int argc, char **argv) {
  int nthreads = 1;
  int maxsols  = 8;
  int nparts = NVPARTS / (PROOFSIZE/2);
  const char *header = "";
  int c;
  while ((c = getopt (argc, argv, "h:m:n:t:")) != -1) {
    switch (c) {
      case 'h':
        header = optarg;
        break;
      case 'm':
        maxsols = atoi(optarg);
        break;
      case 'n':
        nparts = atoi(optarg);
        break;
      case 't':
        nthreads = atoi(optarg);
        break;
    }
  }
  printf("Looking for %d-cycle on cuckoo%d(\"%s\") with 50%% edges, 1/%d memory, %d/%d parts, %d threads\n",
               PROOFSIZE, SIZESHIFT, header, 1<<SAVEMEM_BITS, nparts, NVPARTS, nthreads);
  u64 nodeBytes = CUCKOO_SIZE*sizeof(u64);
  int nodeUnit;
  for (nodeUnit=0; nodeBytes >= 1024; nodeBytes>>=10,nodeUnit++) ;
  printf("Using %d%cB node memory.\n", (int)nodeBytes, " KMGT"[nodeUnit]);
  cuckoo_ctx ctx(header, nthreads, nparts, maxsols);
  thread_ctx *threads = (thread_ctx *)calloc(nthreads, sizeof(thread_ctx));
  assert(threads);
  for (int t = 0; t < nthreads; t++) {
    threads[t].id = t;
    threads[t].ctx = &ctx;
    assert(pthread_create(&threads[t].thread, NULL, worker, (void *)&threads[t]) == 0);
  }
  for (int t = 0; t < nthreads; t++)
    assert(pthread_join(threads[t].thread, NULL) == 0);
  free(threads);
  for (unsigned s = 0; s < ctx.nsols; s++) {
    printf("Solution");
    for (int i = 0; i < PROOFSIZE; i++)
      printf(" %lx", (long)ctx.sols[s][i]);
    printf("\n");
  }
  return 0;
}