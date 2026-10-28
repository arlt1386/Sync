/*
 * 
 *
 * CS 441/541: Finicky Voter 
 *
 */
#include "finicky-voter.h"

int main(int argc, char * argv[]) {
  srandom(time(NULL));
  
  char buffer[264];
  char *token;
  int j = 0;
  
  //determine if it is file redirection or manual
  if (isatty(fileno(stdin)) == 0){
    fgets(buffer, sizeof(buffer), stdin);
    token = strtok(buffer, " ");
    
    while(token != NULL){
      if(atoi(token) < 1){
	printf("Usage: %s [booths] [democrats] [republicans] [independents]\n", argv[0]);
        printf("All parameters are optional with defaults: 10 5 5 5\n");
        return -1;
      }
      if(j == 0){
	num_booths = atoi(token);
      }else if(j == 1){
	num_dem = atoi(token);
      } else if(j == 2){
	num_rep = atoi(token);
      }else if(j == 3){
	num_ind = atoi(token);
      }else{
	printf("Usage: %s [booths] [democrats] [republicans] [independents]\n", argv[0]);
	printf("All parameters are optional with defaults: 10 5 5 5\n");
	return -1;
      }
      token = strtok(NULL, " ");
      j++;
    }
  }else{
    //parse args
    if (argc > 1){
      num_booths = atoi(argv[1]);
    }
    if (argc > 2){
      num_dem = atoi(argv[2]);
    }
    if (argc > 3){
      num_rep = atoi(argv[3]);
    }
    if (argc > 4) {
      num_ind = atoi(argv[4]);
    }
    
    if(argc > 5) {
      printf("Usage: %s [booths] [democrats] [republicans] [independents]\n", argv[0]);
      printf("All parameters are optional with defaults: 10 5 5 5\n");
      return -1;
      
    }
  }   
    total_voters = num_dem + num_rep + num_ind;

    //create all semaphores
    semaphore_create(&printf_sem, 1);
    semaphore_create(&lock, 1);
    semaphore_create(&dem_mutex, 1);
    semaphore_create(&rep_mutex, 1);
    semaphore_create(&turnstile,1);
    semaphore_create(&booths, num_booths);
    semaphore_create(&booth_mutex, 1);
    semaphore_create(&ready, 1);
    semaphore_create(&barrier, 0);

    //print outs
     printf("Number of Voting Booths   :%d\n", num_booths);
     printf("Number of Democrat        :%d\n", num_dem);
     printf("Number of Republican      :%d\n", num_rep);
     printf("Number of Independent      :%d\n", num_ind);
     printf("--------------------------------------------------------------------------------\n");

     //dynamically allocating
     threads = malloc (sizeof(voter) * total_voters);
     booths_free = (char*)calloc(num_booths, sizeof(char));

     if(threads == NULL) {
            printf("allocation failed\n");
            return 1;
     } 
     if(booths_free == NULL){
       printf("allocation failed\n");
       return 1;
     }
     
     int thread_index = 0; /*intializing voter information */
       //Democrats
     for(int i = 0; i < num_dem; i++) {
              threads[thread_index].identify = i;
              threads[thread_index].party = 'D';
              thread_index++;
      }

      //Republicans
      for(int i = 0; i < num_rep; i++) {
              threads[thread_index].identify = i;
              threads[thread_index].party = 'R';
              thread_index++;
      }

      //Independents
      for(int i = 0; i < num_ind; i++) {
              threads[thread_index].identify = i;
              threads[thread_index].party = 'I';
              thread_index++;
      }

      //Print initial waiting states
     for(int i = 0; i < total_voters; i++) {
	       print_state(i, "Waiting for polling station to open..", -1);
      }

      printf("--------------------------------------------------------------------------------\n");

    //create threads
      int t, rc = 0;
      for(t = 0; t < total_voters; t ++){
	      rc = pthread_create(&threads[t].thread, NULL, vote, (void *)(intptr_t)t);
      }      

      //check for thread error
        if (0 != rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	//wait for all threads to finish
	for(int i = 0; i < total_voters; i++){
	  pthread_join(threads[i].thread, NULL);
	}

	//destroy all semaphores
	semaphore_destroy(&printf_sem);
	semaphore_destroy(&lock);
	semaphore_destroy(&turnstile);
	semaphore_destroy(&booth_mutex);
	semaphore_destroy(&rep_mutex);
	semaphore_destroy(&dem_mutex);
	semaphore_destroy(&booths);
	semaphore_destroy(&ready);
	semaphore_destroy(&barrier);

	//free allocated memory
	free(booths_free);
	free(threads);
	return 0;
}

void * vote(void * threadid){
	
        int tid = (intptr_t) threadid;
	int booth_id = -1;

	//wait for all
	semaphore_wait(&ready);
	ready_count++;
	if(ready_count == total_voters){
	  usleep(2000000);
	  for(int i = 0; i < total_voters; i++){
	    semaphore_post(&barrier);
	  }
	}
	semaphore_post(&ready);
	semaphore_wait(&barrier);

	//enter polling station
	print_state(tid, "Entering the polling station", -1);
	usleep(random() % 50000);
	print_state(tid, "Waiting for a voting booth", -1);
	
	if(threads[tid].party == 'D'){
	  //wait for democrats turn
          semaphore_wait(&turnstile);

	  //count each democrat trying to enter
    	  semaphore_wait(&dem_mutex);
	  dem_count++;
	  if(dem_count == 1){
	    semaphore_wait(&lock);
	  }
	  semaphore_post(&dem_mutex);

	  //change turnstile to prevent starvation
	  semaphore_post(&turnstile);

	  //vote!
	  semaphore_wait(&booths);
	  semaphore_wait(&booth_mutex);
	  booth_id = assign_booth(tid);
	  print_state(tid, "Voting", booth_id);
	  semaphore_post(&booth_mutex);
	  usleep(random() % 100000);

	  //leaving voting room
	  semaphore_wait(&booth_mutex);
	  booths_free[booth_id] = 0;
          print_state(tid, "Leaving the polling station", -1);
	  semaphore_post(&booth_mutex);


	  semaphore_wait(&dem_mutex);
          dem_count --;
          if(dem_count == 0){
            semaphore_post(&lock);
          }
          semaphore_post(&dem_mutex);
	  semaphore_post(&booths);
	  
	  
	}else if(threads[tid].party == 'R'){
	  //wait for republican turn
          semaphore_wait(&turnstile);

          //count each republican trying to enter
          semaphore_wait(&rep_mutex);
          rep_count++;
          if(rep_count == 1){
            semaphore_wait(&lock);
          }
          semaphore_post(&rep_mutex);

          //change turnstile to prevent starvation
          semaphore_post(&turnstile);

          //vote!
	  semaphore_wait(&booths);
          semaphore_wait(&booth_mutex);
	  booth_id = assign_booth(tid);
          print_state(tid, "Voting", booth_id);
	  semaphore_post(&booth_mutex);
	  usleep(random() % 100000);
	  semaphore_wait(&booth_mutex);
	  booths_free[booth_id] = 0;
	  print_state(tid, "Leaving the polling station", -1);
	  semaphore_post(&booth_mutex);
	   
	  semaphore_wait(&rep_mutex);
          rep_count --;
          if(rep_count == 0){
            semaphore_post(&lock);
          }
          semaphore_post(&rep_mutex);
	  semaphore_post(&booths);


	  
	}else{ // independent
	  //wait for open booth
	  semaphore_wait(&booths);

	  //one voter at a time
	  semaphore_wait(&booth_mutex);
	  booth_id = assign_booth(tid);
	  print_state(tid, "Voting", booth_id);
	  semaphore_post(&booth_mutex);
	  usleep(random() % 100000);

	  //leave polling station
	  semaphore_wait(&booth_mutex);
	  booths_free[booth_id] = 0;
	  print_state(tid, "Leaving the polling station", -1);
	  semaphore_post(&booth_mutex);
	  semaphore_post(&booths);
	}

	return 0;
  }

int assign_booth(int tid){
  for(int i = 0; i < num_booths; i++){
    if(booths_free[i] == 0){
      booths_free[i] = threads[tid].party;
      return i;
    }
  }
  
  return -1;
}

void print_state(int tid, const char* state, int booth_num) {
  //only one thread can print at a time
  semaphore_wait(&printf_sem);
  
    //determine full party name
    char* party_name;
    if (threads[tid].party == 'D'){
        party_name = "Democrat";
    }
    else if (threads[tid].party == 'R'){
        party_name = "Republican";
    }
    else{
        party_name = "Independent";
    }
    if(strcmp(state, "Voting") == 0){
      printf("%-12s %-2d in    %-2d|-> [", party_name, threads[tid].identify, booth_num);
    }else{
      printf("%-12s %-10d |-> [", party_name, threads[tid].identify);
    }

    //print out each booth
    for(int i = 0; i < num_booths; i++){
      if(booths_free[i] == 'D'){
	printf("D");
      }else if (booths_free[i] == 'R'){
	printf("R");
      } else if (booths_free[i] == 'I'){
	printf("I");
      }else{
	printf(".");
      }
      if(i < num_booths - 1){
	printf("][");
      }
	
    }
    printf("] <-| %s\n", state);
    semaphore_post(&printf_sem);
}
