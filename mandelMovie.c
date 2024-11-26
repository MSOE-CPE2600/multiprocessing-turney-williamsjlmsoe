//  John Williams
//  CPE 2600 121
//  mandelMovie.c
//  This file generates images in parallel with semaphores
//  John Williams
//  CPE 2600 121
//  mandelMovie.c
//  This file generates images of the Mandelbrot set with a zoom-in effect in parallel using semaphores

// John Williams
// CPE 2600 121
// mandelMovie.c
// This file generates images of the Mandelbrot set with a zoom-in effect using both multiprocessing and multithreading

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <sys/wait.h>
#include "jpegrw.h"
#include <sys/time.h>

#define MAX_THREADS 20 // Maximum threads allowed

typedef struct {
    int thread_id;
    int total_threads;
    int frame_number;
    double xscale;
    unsigned int width;
    unsigned int height;
    double adjusted_xmin;
    double adjusted_xmax;
    double adjusted_ymin;
    double adjusted_ymax;
    imgRawImage* image;
} ThreadArgs;

void* generate_region(void* args);
void generate_frame(int frame_number, double xscale, int num_threads);

int main(int argc, char* argv[]) {
    int num_processes = 4;  // Default number of processes
    int num_threads = 4;   // Default number of threads per process
    int total_frames = 50; // Total frames to generate
    double xscale = 4.0;   // Starting scale
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "p:t:h")) != -1) {
        switch (opt) {
            case 'p':
                num_processes = atoi(optarg);
                if (num_processes <= 0) {
                    fprintf(stderr, "Invalid number of processes. Must be greater than 0.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                num_threads = atoi(optarg);
                if (num_threads <= 0 || num_threads > MAX_THREADS) {
                    fprintf(stderr, "Invalid number of threads. Must be between 1 and %d.\n", MAX_THREADS);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                printf("Usage: %s -p <num_processes> -t <num_threads>\n", argv[0]);
                printf("  -p <num_processes> : Number of child processes (default: 4).\n");
                printf("  -t <num_threads>   : Number of threads per process (default: 4, max: %d).\n", MAX_THREADS);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s -p <num_processes> -t <num_threads>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Start timer for runtime measurement
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Determine the number of frames each process handles
    int frames_per_process = total_frames / num_processes;
    int remaining_frames = total_frames % num_processes;

    pid_t pids[num_processes];
    for (int i = 0; i < num_processes; i++) {
        if ((pids[i] = fork()) == 0) { // Child process
            int start_frame = i * frames_per_process;
            int end_frame = start_frame + frames_per_process;
            if (i == num_processes - 1) {
                end_frame += remaining_frames; // Last process handles the extra frames
            }

            for (int frame = start_frame; frame < end_frame; frame++) {
                double scale = xscale * pow(0.95, frame); // Gradually zoom in
                generate_frame(frame, scale, num_threads);
            }
            exit(EXIT_SUCCESS);
        } else if (pids[i] < 0) { // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_processes; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // End timer for runtime measurement
    gettimeofday(&end_time, NULL);
    double runtime = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_usec - start_time.tv_usec) / 1e6;

    printf("All frames generated successfully.\n");
    printf("Runtime with %d processes and %d threads per process: %.6f seconds\n", num_processes, num_threads, runtime);

    return 0;
}

void generate_frame(int frame_number, double xscale, int num_threads) {
    char filename[256];
    snprintf(filename, sizeof(filename), "mandel%d.jpg", frame_number);

    unsigned int width = 600;
    unsigned int height = 600;
    imgRawImage* image = initRawImage(width, height);

    // Adjust the zoom level
    double x_center = -1.0, y_center = 0.0;
    double adjusted_xmin = x_center - xscale;
    double adjusted_xmax = x_center + xscale;
    double adjusted_ymin = y_center - xscale;
    double adjusted_ymax = y_center + xscale;

    // Create threads to process regions of the frame
    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_args[i] = (ThreadArgs){
            .thread_id = i,
            .total_threads = num_threads,
            .frame_number = frame_number,
            .xscale = xscale,
            .width = width,
            .height = height,
            .adjusted_xmin = adjusted_xmin,
            .adjusted_xmax = adjusted_xmax,
            .adjusted_ymin = adjusted_ymin,
            .adjusted_ymax = adjusted_ymax,
            .image = image,
        };

        if (pthread_create(&threads[i], NULL, generate_region, &thread_args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Save the frame
    if (storeJpegImageFile(image, filename) != 0) {
        fprintf(stderr, "Error: Could not save frame %d to file %s\n", frame_number, filename);
    } else {
        printf("Saved frame %d to %s\n", frame_number, filename);
    }

    freeRawImage(image);
}

void* generate_region(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    int thread_id = thread_args->thread_id;
    int total_threads = thread_args->total_threads;
    unsigned int width = thread_args->width;
    unsigned int height = thread_args->height;
    double adjusted_xmin = thread_args->adjusted_xmin;
    double adjusted_xmax = thread_args->adjusted_xmax;
    double adjusted_ymin = thread_args->adjusted_ymin;
    double adjusted_ymax = thread_args->adjusted_ymax;
    imgRawImage* image = thread_args->image;

    unsigned int y_start = (height / total_threads) * thread_id;
    unsigned int y_end = (thread_id == total_threads - 1) ? height : y_start + (height / total_threads);

    for (unsigned int y = y_start; y < y_end; y++) {
        for (unsigned int x = 0; x < width; x++) {
            double real = adjusted_xmin + (x / (double)width) * (adjusted_xmax - adjusted_xmin);
            double imag = adjusted_ymin + (y / (double)height) * (adjusted_ymax - adjusted_ymin);

            double zr = 0.0, zi = 0.0;
            int max_iterations = 500;
            int iterations = 0;

            double zr_squared = zr * zr;
            double zi_squared = zi * zi;

            while (zr_squared + zi_squared < 4.0 && iterations < max_iterations) {
                zi = 2.0 * zr * zi + imag;
                zr = zr_squared - zi_squared + real;
                zr_squared = zr * zr;
                zi_squared = zi * zi;
                iterations++;
            }

            // Color based on iterations
            unsigned char red = (iterations % 256);
            unsigned char green = (iterations * 2 % 256);
            unsigned char blue = (iterations * 4 % 256);

            setPixelRGB(image, x, y, red, green, blue);
        }
    }

    return NULL;
}
