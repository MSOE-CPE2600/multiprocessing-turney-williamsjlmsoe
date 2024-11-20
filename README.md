# System Programming Lab 11 Multiprocessing

a.  This program generates 50 frames of Mandelbrot fractal images using multiprocessing. The implementation 
    is designed to allow the user to specify the number of child processes (-p) for parallel execution. Each 
    child process generates a subset of the frames, ensuring efficient use of system resources.


b. If you right click and press copy and then paste into google you can see it

https://msoe365-my.sharepoint.com/:i:/r/personal/williamsjl_msoe_edu/Documents/Pictures/Screenshots/Screenshot%202024-11-19%20230319.png?csf=1&web=1&e=gYidei

c.  This implementation shows how using multiple processes can improve the runtime for generating Mandelbrot 
    fractal images. As the number of processes increases, the workload is divided among the processes, which 
    reduces the overall runtime. However, there is a limit to how much performance improves because creating 
    and managing more processes adds overhead.

    One thing I noticed is that while more processes can help, after a certain number, the benefit starts to 
    level off. This is likely due to the system reaching its maximum efficiency with the available CPU cores. 
    Using too many processes could actually hurt performance due to the extra work required to manage them.

    Overall, the program worked well, and the frames were generated correctly. Measuring the runtime helped 
    show how the number of processes affects performance, and the results support the idea that multiprocessing 
    can speed up intensive tasks like this one.