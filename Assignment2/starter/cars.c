#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "traffic.h"

extern struct intersection isection;

/**
 * Populate the car lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <in_direction> <out_direction>
 *
 * Each car is added to the list that corresponds with 
 * its in_direction
 * 
 * Note: this also updates 'inc' on each of the lanes
 */
void parse_schedule(char *file_name) {
    int id;
    struct car *cur_car;
    struct lane *cur_lane;
    enum direction in_dir, out_dir;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*)&in_dir, (int*)&out_dir) == 3) {

        /* construct car */
        cur_car = malloc(sizeof(struct car));
        cur_car->id = id;
        cur_car->in_dir = in_dir;
        cur_car->out_dir = out_dir;

        /* append new car to head of corresponding list */
        cur_lane = &isection.lanes[in_dir];
        cur_car->next = cur_lane->in_cars;
        cur_lane->in_cars = cur_car;
        cur_lane->inc++;
    }

    fclose(f);
}

/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the intersection
 * before any cars start coming
 * 
 */
void init_intersection() {

    int i;

    // Init isection mutex
    for(i = 0; i < 4; ++i) {
        pthread_mutex_init(isection.quad + i, NULL);
    }

    // Init each lane 
    for(i = 0; i < 4; ++i) {

        // Init mutex and cond_vars
        pthread_mutex_init(&(isection.lanes[i].lock), NULL);
        pthread_cond_init(&(isection.lanes[i].producer_cv), NULL);
        pthread_cond_init(&(isection.lanes[i].consumer_cv), NULL);

        // Init list heads to NULL
        isection.lanes[i].in_cars = NULL;
        isection.lanes[i].out_cars = NULL;

        // Init buffer. Buffers are released when corresponding car_cross returns
        isection.lanes[i].buffer = malloc(LANE_LENGTH * sizeof(struct car*));
        
        // Init other variables
        isection.lanes[i].passed = 0;
        isection.lanes[i].head = 0;
        isection.lanes[i].tail = 0;
        isection.lanes[i].capacity = LANE_LENGTH;
        isection.lanes[i].in_buf = 0;

    }

}

/**
 * TODO: Fill in this function
 *
 * Populates the corresponding lane with cars as room becomes
 * available. Ensure to notify the cross thread as new cars are
 * added to the lane.
 * 
 */
void *car_arrive(void *arg) {
    struct lane *l = arg;

    // While there are cars pending to pass through the line
    while(l -> in_cars != NULL) {

        pthread_mutex_lock(&(l -> lock));

        // If buffer full, wait for space
        while(l -> in_buf == LANE_LENGTH) {
            pthread_cond_wait(&(l -> consumer_cv), &(l -> lock));
        }

        // Get the first car from list in_cars
        l -> buffer[l -> head] = l -> in_cars;
        l -> in_cars = l -> in_cars -> next;
        l -> head = (l -> head + 1) % LANE_LENGTH;

        // Update lane variables
        ++(l -> in_buf);
        ++(l -> passed);

        // Signal producer_cv
        pthread_cond_signal(&(l -> producer_cv));

        pthread_mutex_unlock(&(l -> lock));

    }

    /* avoid compiler warning */
    l = l;

    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Moves cars from a single lane across the intersection. Cars
 * crossing the intersection must abide the rules of the road
 * and cross along the correct path. Ensure to notify the
 * arrival thread as room becomes available in the lane.
 *
 * Note: After crossing the intersection the car should be added
 * to the out_cars list of the lane that corresponds to the car's
 * out_dir. Do not free the cars!
 *
 * 
 * Note: For testing purposes, each car which gets to cross the 
 * intersection should print the following three numbers on a 
 * new line, separated by spaces:
 *  - the car's 'in' direction, 'out' direction, and id.
 * 
 * You may add other print statements, but in the end, please 
 * make sure to clear any prints other than the one specified above, 
 * before submitting your final code. 
 */
void *car_cross(void *arg) {
    struct lane *l = arg;
    int i;
    int *locks;

    // While there are cars pending or in buffer
    while(l -> in_cars != NULL || l -> in_buf != 0) {
        
        pthread_mutex_lock(&(l -> lock));

        // If buffer empty, wait for buffer to fill up
        while(l -> in_buf == 0) {
            pthread_cond_wait(&(l -> producer_cv), &(l -> lock));
        }

        // Get earliest car in buffer, and acquire locks
        locks = compute_path(l -> buffer[l -> tail] -> in_dir, l -> buffer[l -> tail] -> out_dir);
        for(i = 0; locks[i] != 0 && i < 4; ++i) {
            pthread_mutex_lock(&isection.quad[locks[i] - 1]);
        }

        // Lock acquired, remove car from buffer to head of out_cars
        l -> buffer[l -> tail] -> next = l -> out_cars;
        l -> out_cars = l -> buffer[l -> tail];
        l -> buffer[l -> tail] = NULL;
        l -> tail = (l -> tail + 1) % LANE_LENGTH;
        --(l -> in_buf);
        fprintf(stderr, "%d %d %d\n", l -> out_cars -> in_dir, l -> out_cars -> out_dir, l -> out_cars -> id);

        // Relinquish Locks and free memory allocated in compute_path()
        for(i = 0; locks[i] != 0 && i < 4; ++i) {
            pthread_mutex_unlock(&isection.quad[locks[i] - 1]);
        }
        free(locks);
        
        // Signal consumer_cv
        pthread_cond_signal(&(l -> consumer_cv));

        pthread_mutex_unlock(&(l -> lock));

    }

    // Free lane buffer
    free(l -> buffer);

    /* avoid compiler warning */
    l = l;

    return NULL;
}

/**
 * TODO: Fill in this function
 *
 * Given a car's in_dir and out_dir return a sorted 
 * list of the quadrants the car will pass through.
 * 
 */
 int *compute_path(enum direction in_dir, enum direction out_dir) {
    
    // Need to free memory for list path in outer function
    
    // Local Variables
    int i, j = 0;
    
    int *passed_quadrant = (int *)malloc(4 * sizeof(int));
    int *path = (int *)malloc(4 * sizeof(int));
    memset(passed_quadrant, 0, 4 * sizeof(int));
    memset(path, 0, 4 * sizeof(int));
    
    // Calculate which quadrant to pass, and set the corresponding flag in passed to 1
    if(in_dir == out_dir) {
        // Cars make U-turns, takes all four quadrants
        memset(passed_quadrant, 1, 4 * sizeof(int));
    }
    else{
        // Otherwise
        for(i = (in_dir + 1) % 4 + 1; i != out_dir + 2; ++i) {
            if(i == 5) i = 1;
            passed_quadrant[i - 1] = 1;
        }
    }
    
    // Generate an array of length 4. Pad 0 at the end if necessary.
    for(i = 0; i < 4; ++i) {
        if(passed_quadrant[i]) {
            path[j++] = i + 1;
        }
    }
    
    // Release Memory and return
    free(passed_quadrant);
    return path;
    
}