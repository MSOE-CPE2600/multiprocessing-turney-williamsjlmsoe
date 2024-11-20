//  John Williams
//  CPE 2600 121
//  mandelMovie.c
//  This file generates images in parallel with semaphores
//  John Williams
//  CPE 2600 121
//  mandelMovie.c
//  This file generates images of the Mandelbrot set with a zoom-in effect in parallel using semaphores

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include "jpegrw.h"
#include <sys/time.h>

void generate_frame(int frame_number, double xscale);

int main(int argc, char *argv[]) {
    int num_processes = 4;  // Default number of processes
    int total_frames = 50; // Total frames to generate
    double xscale = 4.0;   // Starting scale
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "p:h")) != -1) {
        switch (opt) {
            case 'p':
                num_processes = atoi(optarg);
                if (num_processes <= 0) {
                    fprintf(stderr, "Invalid number of processes. Must be greater than 0.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                printf("Usage: %s -p <num_processes>\n", argv[0]);
                printf("  -p <num_processes> : Number of child processes (default: 4).\n");
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s -p <num_processes>\n", argv[0]);
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
                generate_frame(frame, scale);
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
    printf("Runtime with %d processes: %.6f seconds\n", num_processes, runtime);

    return 0;
}

void generate_frame(int frame_number, double xscale) {
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

    // Generate Mandelbrot set
    for (unsigned int y = 0; y < height; y++) {
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

    // Save the frame
    if (storeJpegImageFile(image, filename) != 0) {
        fprintf(stderr, "Error: Could not save frame %d to file %s\n", frame_number, filename);
    } else {
        printf("Saved frame %d to %s\n", frame_number, filename);
    }

    freeRawImage(image);
}
