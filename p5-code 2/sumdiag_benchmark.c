#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include "sumdiag.h"

double total_points = 0;
double actual_score = 0;
int host_ok = 0;

void check_hostname();
#include "data.c"

int REPEATS = 10;

int sizes[] = {
  // 512,
  1024,
  1101,
  2048,
  // 4096,
  4099,
  // 6001, 
  8192,
};

int thread_counts[] = {
  1,
  2,
  3,
  4,
  /* 8, */
};

// use compile time size of global arrays to determine count of
// elements in them; this only works for global arrays
int nsizes = sizeof(sizes)/sizeof(int);
int nthread_counts = sizeof(thread_counts) / sizeof(int);

void print_result(long size, double cpu_time_BASE,
                  int thread_count, double cpu_time_OPTM,
                  double speedup_OPTM, double points)
{
  if(size == -1){               // print the header
    printf("%6s ","SIZE");
    printf("%6s ","BASE");
    printf("%2s ","#T");
    printf("%6s ","OPTM");
    printf("%6s ", "SPDUP");
    printf("%6s ", "POINTS");
  }
  else if(thread_count==1){     // print the normal stats
    printf("%6ld ", size);
    printf("%6.3f ",cpu_time_BASE);
    printf("%2d ",  thread_count);
    printf("%6.3f ",cpu_time_OPTM);
    printf("%6.2f ", speedup_OPTM);
    printf("%6.2f ",  points);
  }
  else{                         // omit the first two fields
    printf("%6s ", "");
    printf("%6s ","");
    printf("%2d ",  thread_count);
    printf("%6.3f ",cpu_time_OPTM);
    printf("%6.2f ", speedup_OPTM);
    printf("%6.2f ",  points);
  }
  printf("\n");
}


// Timing data and functions. The Wall time (real world time) is
// returned as this benchmark expects to use
// multi-threading. Multi-threaded computations have the same cpu_time
// as single-threaded but take less Wall time if they are effective.

struct timeval beg_time, end_time;
clock_t begin, end;

void timing_start(){
  // begin = clock();
  gettimeofday(&beg_time, NULL);
}

double timing_stop(){
  // end = clock();
  // double cpu_time = ((double) (end - begin)) / CLOCKS_PER_SEC;

  gettimeofday(&end_time, NULL);
  double wall_time = 
    ((end_time.tv_sec-beg_time.tv_sec)) + 
    ((end_time.tv_usec-beg_time.tv_usec) / 1000000.0);
  return wall_time;             // real-world time
}

int main(int argc, char *argv[]){
  check_hostname();


  if(argc > 1 && strcmp(argv[1],"-test")==0){
    nsizes = 3;                 // for valgrind testing
    REPEATS = 1;
    sizes[0] = 105;
    sizes[1] = 256;
    sizes[2] = 511;
  }

  printf("==== Matrix Diagonal Sum Benchmark Version 5 ====\n");
  printf("------ Tuned for ODD grace.umd.edu machines -----\n");
  printf("Running with %d sizes (max %d) and %d thread_counts (max %d)\n",
         nsizes, sizes[nsizes-1], nthread_counts, thread_counts[nthread_counts-1]);

  print_result(-1,0,0,0,0,0);  // print header

  pb_srand(1234567);

  // Iterate over different sizes of the matrix
  for(int sidx=0; sidx<nsizes; sidx++){
    long size = sizes[sidx];
    long rows=size, cols=size;

    matrix_t mat;
    vector_t res_BASE, res_OPTM;
    matrix_init(&mat,rows,cols);
    matrix_fill_random(mat, 100); // random values 0 to 99
    vector_init(&res_BASE, 2*size-1);
    vector_init(&res_OPTM, 2*size-1);
    memset(res_BASE.data, -1, sizeof(int)*res_BASE.len); // init vectors to -1
    memset(res_BASE.data, -1, sizeof(int)*res_BASE.len); // must reset vals to 0 when summing
    
    timing_start();
    for(int i=0; i<REPEATS; i++){
      sumdiag_BASE(mat,res_BASE);
    }
    double wall_time_BASE = timing_stop();

    // THREAD LOOP
    for(int tidx = 0; tidx < nthread_counts; tidx++){
      int thread_count = thread_counts[tidx];

      timing_start();
      for(int i=0; i<REPEATS; i++){
        sumdiag_OPTM(mat,res_OPTM, thread_count);
      }
      double wall_time_OPTM = timing_stop();

      double speedup_OPTM = (wall_time_BASE / wall_time_OPTM);
      double points = log(speedup_OPTM) / log(2.0) * size / 4500;
      if(points < 0){
        points = 0;
      }

      for(int i=0; i<res_BASE.len; i++){
        int base_i = VGET(res_BASE,i);
        int opt_i  = VGET(res_OPTM,i);
        if(base_i != opt_i){
          printf("ERROR: BASE and OPT versions produced different results\n");
          printf("ERROR: res[%d]: %d != %d\n",i,base_i,opt_i);
          printf("ERROR: Skipping checks on remaining elements\n");
          printf("ERROR: Try running the 'sumdiag_print <size>' program to see all differences\n");
          speedup_OPTM = -1.0;
          points = 0;
          break;
        }
      }

      total_points += points;

      print_result(size, wall_time_BASE,
                   thread_count, wall_time_OPTM,
                   speedup_OPTM, points);
    } 
    // END THREAD COUNT LOOP

    matrix_free_data(&mat);       // clean up data before going to next size
    vector_free_data(&res_BASE);
    vector_free_data(&res_OPTM);
  }

  double max_score = 30.0;
  actual_score = total_points;
  printf("RAW POINTS: %.2f\n",actual_score);

  if(actual_score > max_score){
    actual_score = max_score;
    final_check();
  }

  printf("TOTAL POINTS: %.0f / %.0f\n",actual_score,max_score);

  check_hostname();

  return 0;
}
  
#define MAXHOSTNAMELEN 1024
#define FULL_EXPECT_HOST "grace9.umd.edu" // full expected host name
char *allowed_hosts[] = {
  "grace3.umd.edu",
  "grace5.umd.edu",
  "grace7.umd.edu",
  "grace9.umd.edu",
  NULL
};

void check_hostname(){
  char actual_host[MAXHOSTNAMELEN];
  if (gethostname(actual_host, MAXHOSTNAMELEN) != 0) {
    printf("WARNING: Couldn't get machine hostname\n");
    return;
  }
  for(int i=0; allowed_hosts[i]!=NULL; i++){
    if(strcmp(allowed_hosts[i], actual_host) == 0){
      host_ok = 1;
      return;
    }
  }
  printf("WARNING: expected host like '%s' but got host '%s'\n",FULL_EXPECT_HOST,actual_host);
  printf("WARNING: ensure you are on an ODD grace node\n");
  printf("WARNING: timing results / scoring will not reflect actual scoring\n");
  printf("WARNING: run on one of the following hosts for accurate results\n");
  for(int i=0; allowed_hosts[i]!=NULL; i++){
    printf("WARNING:   %s\n",allowed_hosts[i]);
  }
  printf("WARNING: while on grace, try `ssh grace5` to log into a specifc node\n");
  return;
}
