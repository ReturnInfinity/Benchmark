#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    long int i;
    long int darts_in_circle = 0;
    long int total_darts = 50000000; // Increase this for better precision
    double x, y, distance_squared;
    double pi_estimate;

    // Seed the random number generator
    srand(time(NULL));

    for (i = 0; i < total_darts; i++) {
        // Generate random coordinates between 0 and 1
        x = (double)rand() / RAND_MAX;
        y = (double)rand() / RAND_MAX;

        // Check if the point is inside the unit circle (x^2 + y^2 <= 1)
        distance_squared = x * x + y * y;

        if (distance_squared <= 1) {
            darts_in_circle++;
        }
    }

    // Calculate pi
    pi_estimate = 4.0 * darts_in_circle / total_darts;

    printf("After %ld darts, the estimate for pi is: %f\n", total_darts, pi_estimate);

    return 0;
}
