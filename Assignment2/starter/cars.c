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

    // Init lane 
    for(i = 0; i < 4; ++i) {

        // Init mutex and cond_vars
        pthread_mutex_init(&(isection.lane[i].lock), NULL);
        pthread_cond_init(&(isection.lane[i].producer_cv), NULL);
        pthread_cond_init(&(isection.lane[i].consumer_cv), NULL);

        // Init list heads to NULL
        isection.lane[i].in_cars = NULL;
        isection.lane[i].out_cars = NULL;

        // Other variables
        isection.lane[i].inc = 0;
        isection.lane[i].passed = 0;
        
        // Init buffer
        isection.lane[i].buffer = malloc(LANE_LENGTH * sizeof(struct car*));
        isection.lane[i].head = 0;
        isection.lane[i].tail = 0;
        isection.lane[i].capacity = LANE_LENGTH;
        isection.lane[i].in_buf = 0;

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
        while(l.in_buf == LANE_LENGTH) {
            pthread_cond_wait(&(l -> consumer_cv), &(l -> lock));
        }

        // Get the first car from list in_cars
        buffer[tail] = l -> in_cars;
        l -> in_cars = l -> in_cars -> next;
        tail = (tail + 1) % LANE_LENGTH;
        
        // Update lane variables
        ++(l -> inc);
        ++(l -> in_buf);

        // Signal producer_cv
        pthread_cond_signal(&(l -> producer_cv));

        pthread_mutex_lock(&l.lock);

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
    
    int *passed = (int *)malloc(4 * sizeof(int));
    int *path = (int *)malloc(4 * sizeof(int));
    memset(passed, 0, 4 * sizeof(int));
    memset(path, 0, 4 * sizeof(int));
    
    // Calculate which quadrant to pass, and set the corresponding flag in passed to 1
    if(in_dir == out_dir) {
        // Cars make U-turns, takes up all quadrants
        memset(passed, 1, 4 * sizeof(int));
    }
    else{
        // Otherwise
        for(i = (in_dir + 1) % 4 + 1; i != out_dir + 2; ++i) {
            if(i == 5) i = 1;
            passed[i - 1] = 1;
        }
    }
    
    // Generate list of passed quadrants. Pad 0s at the end.
    for(i = 1; i <= 4; ++i) {
        if(passed[i - 1]) {
            path[j++] = i;
        }
    }
    
    // Release Memory and return
    free(passed);
    return path;
    
}