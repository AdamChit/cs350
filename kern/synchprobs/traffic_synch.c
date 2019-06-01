#include <types.h>
#include <lib.h>
#include <queue.h>
#include <synchprobs.h>
#include <synch.h>
#include <opt-A1.h>
#include <wchan.h>
#include <threadlist.h>

/* 
 * This simple default synchronization mechanism allows only vehicle at a time
 * into the intersection.   The intersectionSem is used as a a lock.
 * We use a semaphore rather than a lock so that this code will work even
 * before locks are implemented.
 */

/* 
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to 
 * declare other global variables if your solution requires them.
 */

/*
 * replace this with declarations of any synchronization and other variables you need here
 */
// static struct semaphore *intersectionSem;

static struct lock *intersection_lock;
static struct cv *cv_south_go;
static struct cv *cv_north_go;
static struct cv *cv_east_go;
static struct cv *cv_west_go;
Direction first_dir;
Direction north_dir;
Direction south_dir;
Direction east_dir;
Direction west_dir;
Direction *current_light;
bool is_waiting;
static struct queue *next_lights;
int num_vehicles_waiting_north;
int num_vehicles_waiting_south;
int num_vehicles_waiting_east;
int num_vehicles_waiting_west;
int num_vehicles_waiting_b;
/* 
 * The simulation driver will call this function once before starting
 * the simulation
 *
 * You can use it to initialize synchronization and other variables.
 * 
 */
void
intersection_sync_init(void)
{
  /* replace this default implementation with your own implementation */

  // intersectionSem = sem_create("intersectionSem",1);
  intersection_lock = lock_create("intersectionlock"); 
  
  next_lights = q_create(4);

  // // Direction x = north;
  // n = south;
  // Direction *x_p = &n;
  // // Direction x1 = south;
  // Direction *x1_p = &n;
  // // Direction x2 = east;
  // Direction *x2_p = &n;

  north_dir = north;
  south_dir = south;
  east_dir = east;
  west_dir = west;
  num_vehicles_waiting_north = 0;
  num_vehicles_waiting_south = 0;
  num_vehicles_waiting_east = 0;
  num_vehicles_waiting_west = 0;
  num_vehicles_waiting_b = 0;
  // Direction *x3_p = &n;
  
  // q_addtail(next_lights, x_p);
  // q_addtail(next_lights, x1_p);
  // q_addtail(next_lights, x2_p);
  // q_addtail(next_lights, x3_p);
  // current_light = q_peek(next_lights);
  // n = north;
  // current_light = &n;
  //*num_vehicles_waiting = 0;
  is_waiting = false;
  cv_south_go = cv_create("cv_south");
  cv_north_go = cv_create("cv_north");
  cv_east_go = cv_create("cv_east");
  cv_west_go = cv_create("cv_west");
  // if (intersectionSem == NULL) {
  //   panic("could not create intersection semaphore");
  // }

  if (intersection_lock == NULL) {
    panic("could not create intersection lock");
  }

  if (cv_south_go == NULL || cv_north_go == NULL || cv_east_go == NULL || cv_west_go == NULL) {
    panic("could not create intersection cv lock");
  }

  return;
}

/* 
 * The simulation driver will call this function once after
 * the simulation has finished
 *
 * You can use it to clean up any synchronization and other variables.
 *
 */
void
intersection_sync_cleanup(void)
{
  /* replace this default implementation with your own implementation */
  //KASSERT(intersectionSem != NULL);
  KASSERT(intersection_lock != NULL);
  KASSERT(next_lights != NULL);
  KASSERT(cv_south_go != NULL);
  KASSERT(cv_north_go != NULL);
  KASSERT(cv_east_go != NULL);
  KASSERT(cv_west_go != NULL);
  //sem_destroy(intersectionSem);
  lock_destroy(intersection_lock);
  cv_destroy(cv_south_go);
  cv_destroy(cv_north_go);
  cv_destroy(cv_east_go);
  cv_destroy(cv_west_go);

}

bool
in_queue(Direction  light_to_push)
{
  bool flag = false;
  int i = 0;
  for (i=q_getstart(next_lights); i!=q_getend(next_lights); i=(i+1)%q_getsize(next_lights)) {
    Direction *ptr = q_getguy(next_lights, i);
     if (light_to_push == *ptr){
        flag = true;
     }
  }

  return flag;

}

/*
 * The simulation driver will call this function each time a vehicle
 * tries to enter the intersection, before it enters.
 * This function should cause the calling simulation thread 
 * to block until it is OK for the vehicle to enter the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle is arriving
 *    * destination: the Direction in which the vehicle is trying to go
 *
 * return value: none
 */

void
intersection_before_entry(Direction origin, Direction destination) 
{
  /* replace this default implementation with your own implementation */
  //(void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  // KASSERT(intersectionSem != NULL);
  // P(intersectionSem);
  KASSERT(intersection_lock != NULL);
  KASSERT(next_lights != NULL);
  KASSERT(cv_south_go != NULL);
  KASSERT(cv_north_go != NULL);
  KASSERT(cv_east_go != NULL);
  KASSERT(cv_west_go != NULL);


  lock_acquire(intersection_lock);
  // bool check = in_queue(origin);
  // if (!check){
  //   check = true;
  // }
  // if (q_empty(next_lights)){
  //   first_dir = origin;
  //   current_light = &first_dir;
  // }
  // add the cu
  // if(!in_queue(origin)){
        
  //       q_addtail(next_lights,pointer);
  //   }

  // current_light = q_peek(next_lights);
  // q_remhead(next_lights);

  // if (origin == north){
  //   // if the light is green
    
  //   if(!in_queue(origin)){
  //     q_addtail(next_lights,&north_dir);
  //   }

  //   current_light = q_peek(next_lights);
  //   // q_remhead(next_lights);
  //   if (origin == *current_light)  {
  //     if (num_vehicles_waiting_north != 0){
  //         num_vehicles_waiting_north++;
  //         cv_wait(cv_north_go,intersection_lock);
  //     }
  //   }
  //   else{
  //       num_vehicles_waiting_north++;
  //       cv_wait(cv_north_go,intersection_lock);
  //   }
  // }
  // else if (origin == south){
  //   // if the light is green
  //   if(!in_queue(origin)){
  //     q_addtail(next_lights,&south_dir);
  //   }
  //   current_light = q_peek(next_lights);
  //   // q_remhead(next_lights);
  //   if (origin == *current_light) {
  //     if (num_vehicles_waiting_south != 0){
  //     num_vehicles_waiting_south++;    
  //     cv_wait(cv_south_go,intersection_lock);
  //     }
  //   }
  //       else{
  //       num_vehicles_waiting_south++;
  //       cv_wait(cv_south_go,intersection_lock);
  //   }
  // }  
  // else if (origin == east){
  //   // if the light is green
  //   if(!in_queue(origin)){
  //     q_addtail(next_lights,&east_dir);
  //   }
    
  //   current_light = q_peek(next_lights);
  //   // q_remhead(next_lights);
  //   if (origin == *current_light) {
  //     if (num_vehicles_waiting_east != 0){
  //     num_vehicles_waiting_east++;   
  //     cv_wait(cv_east_go,intersection_lock);
  //     }
  //   }
  //   else{
  //     num_vehicles_waiting_east++;   
  //     cv_wait(cv_east_go,intersection_lock);      
  //   }
  // }  
  // else if (origin == west){
  //   // if the light is green
  //   if(!in_queue(origin)){
  //     q_addtail(next_lights,&west_dir);
  //   }
    
  //   current_light = q_peek(next_lights);
  //   // q_remhead(next_lights);
  //   if (origin == *current_light) {
  //     if (num_vehicles_waiting_west != 0){
  //     num_vehicles_waiting_west++;     
  //     cv_wait(cv_west_go,intersection_lock);
  //     }

  //   }else{
  //     num_vehicles_waiting_west++;     
  //     cv_wait(cv_west_go,intersection_lock);
  //   }
  // }

  if (origin == north)
    {
    // if the light is green
    if(!in_queue(origin)){
      q_addtail(next_lights,&north_dir);
    }
    current_light = q_peek(next_lights);
    // q_remhead(next_lights);
    num_vehicles_waiting_north++;
    if (origin != *current_light)  {
      cv_wait(cv_north_go,intersection_lock);
    }
   
  }
  else if (origin == south){
    // if the light is green
    if(!in_queue(origin)){
      q_addtail(next_lights,&south_dir);
    }
    current_light = q_peek(next_lights);
    // q_remhead(next_lights);
    num_vehicles_waiting_south++;    

    if (origin != *current_light) {
      
      cv_wait(cv_south_go,intersection_lock);
      
    }
       
  }  
  else if (origin == east){
    // if the light is green
    if(!in_queue(origin)){
      q_addtail(next_lights,&east_dir);
    }
    
    current_light = q_peek(next_lights);
    num_vehicles_waiting_east++;
    // q_remhead(next_lights);
    if (origin != *current_light) {
      cv_wait(cv_east_go,intersection_lock); 
    }

  }  
  else if (origin == west){
    // if the light is green
    if(!in_queue(origin)){
      q_addtail(next_lights,&west_dir);
    }
    num_vehicles_waiting_west++; 
    current_light = q_peek(next_lights);
    // q_remhead(next_lights);
    if (origin == *current_light) {
      cv_wait(cv_west_go,intersection_lock);
      

    }
  }
  lock_release(intersection_lock);
}

/*
 * The simulation driver will call this function each time a vehicle
 * leaves the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle arrived
 *    * destination: the Direction in which the vehicle is going
 *
 * return value: none
 */

void
intersection_after_exit(Direction origin, Direction destination) 
{
  KASSERT(intersection_lock != NULL);
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  // KASSERT(intersectionSem != NULL);
  // V(intersectionSem);
  // lock_release(intersection_lock);
  lock_acquire(intersection_lock);
  if (!q_empty(next_lights)){
    // if (num_vehicles_waiting_b > 0){
    //   num_vehicles_waiting_b--;
      
    //   if (origin == south){
    //     num_vehicles_waiting_south--;
    //   }
    //   else if (origin == north){
    //     num_vehicles_waiting_north--;
    //   }
    //   else if(origin == east){
    //     num_vehicles_waiting_east--;
    //   }
    //   else if(origin == west){
    //     num_vehicles_waiting_west--;
    //   }
      

    // }

    // if (origin == south){
    //   num_vehicles_waiting_south--;
    //   if(num_vehicles_waiting_south == 0){
    //     q_remhead(next_lights);
    //   }
    // }
    // else if (origin == north){
    //     num_vehicles_waiting_north--;
    //     if(num_vehicles_waiting_north == 0){
    //       q_remhead(next_lights);
    //   }
        
    // }
    // else if(origin == east){
    //   num_vehicles_waiting_east--;
    //   if(num_vehicles_waiting_east == 0){
    //     q_remhead(next_lights);
    //   }
    // }
    // else if(origin == west){
    //   num_vehicles_waiting_west--;
    //   if(num_vehicles_waiting_west == 0){
    //     q_remhead(next_lights);
    //   }    
    // }
      
    q_remhead(next_lights); 
   
    // if(num_vehicles_waiting_b == 0){
    //   q_remhead(next_lights);
    // }
    
    if (!q_empty(next_lights))
    {
      // q_remhead(next_lights);
      // current_light = q_peek(next_lights);
      current_light = q_peek(next_lights);
      if (*current_light == north){
        //*current_light = east;
        //num_vehicles_waiting -= cv_north_go->cv_wchan->wc_threads.tl_count;
        //struct threadlist l = (struct threadlist) cv_north_go->cv_wchan->wc_threads;
        // num_vehicles_waiting = 0;
        // is_waiting = true;
        //lock_release(intersection_lock);
        // num_vehicles_waiting_b = num_vehicles_waiting_north;
        cv_broadcast(cv_north_go,intersection_lock);
        
      }
      else if (*current_light == south){
        //num_vehicles_waiting -= cv_south_go->cv_wchan->wc_threads.tl_count;
        //*current_light = west;
        // num_vehicles_waiting = 0;
        //lock_release(intersection_lock);
        // num_vehicles_waiting_b = num_vehicles_waiting_south;
        cv_broadcast(cv_south_go,intersection_lock);
      }  
      else if (*current_light == east){
        // num_vehicles_waiting -= cv_east_go->cv_wchan->wc_threads.tl_count;
        // num_vehicles_waiting = 0;
        //*current_light = south;
        //lock_release(intersection_lock);
        // num_vehicles_waiting_b = num_vehicles_waiting_east;
        cv_broadcast(cv_east_go,intersection_lock);
      }  
      else if (*current_light == west){
        //*current_light = north;
        // num_vehicles_waiting -= cv_west_go->cv_wchan->wc_threads.tl_count;
        // num_vehicles_waiting = 0;
        //lock_release(intersection_lock);
        // num_vehicles_waiting_b = num_vehicles_waiting_west;
        cv_broadcast(cv_west_go,intersection_lock);
      }


    }
  }
  lock_release(intersection_lock);

}
