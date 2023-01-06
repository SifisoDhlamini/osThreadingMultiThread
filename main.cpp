#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX 500
#define NUM_THREADS 4

int raw, col;
float val;
double matrix1[MAX][MAX], matrix2[MAX][MAX], matrix3[MAX][MAX];
char infilename[20], outfilename[20];
FILE *infile, *outfile;

typedef struct
{
    int start_row;
    int end_row;
} thread_data;

// Declare the mutex object at the global level
pthread_mutex_t mutex;

void *matrix_multiply(void *thread_arg)
{
    thread_data *data = (thread_data *)thread_arg;
    int start_row = data->start_row;
    int end_row = data->end_row;
    int i, j, k;
    double val;
    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < col; j++)
        {
            val = 0;
            for (k = 0; k < raw; k++)
            {
                val += matrix1[i][k] * matrix2[k][j];
            }
            // Lock the mutex before accessing matrix3
            pthread_mutex_lock(&mutex);
            matrix3[i][j] = val;
            // Unlock the mutex after accessing matrix3
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
}

void *read_input(void *infile_ptr)
{
    // Lock the mutex before accessing infile
    pthread_mutex_lock(&mutex);
    FILE *infile = (FILE *)infile_ptr;
    int i, j;
    // Read the dimensions of the matrix
    if (fscanf(infile, "%d", &raw) < 1 || fscanf(infile, "%d", &col) < 1)
    {
        perror("Error reading matrix dimensions from input file");
        // Unlock the mutex and return early
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    // Read the values in the matrix
    for (i = 0; i < raw; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (fscanf(infile, "%f", &val) < 1)
            {
                perror("Error reading matrix value from input file");
                // Unlock the mutex and return early
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            matrix1[i][j] = matrix2[i][j] = val;
        }
    }
    // Unlock the mutex after accessing infile
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *write_output(void *outfile_ptr)
{
    // Lock the mutex before accessing outfile
    pthread_mutex_lock(&mutex);
    FILE *outfile = (FILE *)outfile_ptr;
    int i, j;
    // Write the dimensions of the matrix
    if (fprintf(outfile, "%d\n%d\n", raw, col) < 0)
    {
        perror("Error writing matrix dimensions to output file");
        // Unlock the mutex and return early
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    // Write the values in the matrix
    for (i = 0; i < raw; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (fprintf(outfile, "%f\n", matrix3[i][j]) < 0)
            {
                perror("Error writing matrix value to output file");
                // Unlock the mutex and return early
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
    }
    // Unlock the mutex after accessing outfile
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    int i, j, k, x;
    struct timeval tp_s, tp_e;
    struct timezone tzp_s, tzp_e;

    // Initialize the mutex object
    pthread_mutex_init(&mutex, NULL);

    gettimeofday(&tp_s, &tzp_s);
    for (x = 0; x < 1000; x++)
    {
        sprintf(infilename, "in%d.txt", x + 1);
        sprintf(outfilename, "out%d.txt", x + 1);
        if ((infile = fopen(infilename, "r")) == NULL)
        {
            perror("in open error~\n");
        }
        if ((outfile = fopen(outfilename, "w")) == NULL)
        {
            perror("out open error~\n");
        }
        printf("%d\n", x);

        pthread_t input_thread, output_thread;
        pthread_t threads[NUM_THREADS];
        thread_data thread_data_array[NUM_THREADS];
        int rows_per_thread = raw / NUM_THREADS;
        for (i = 0; i < NUM_THREADS; i++)
        {
            thread_data_array[i].start_row = i * rows_per_thread;
            thread_data_array[i].end_row = (i + 1) * rows_per_thread;
            pthread_create(&threads[i], NULL, matrix_multiply, (void *)&thread_data_array[i]);
        }
        // Create the input and output threads
        pthread_create(&input_thread, NULL, read_input, (void *)infile);
        pthread_create(&output_thread, NULL, write_output, (void *)outfile);

        for (i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }
        // Wait for the input and output threads to finish
        pthread_join(input_thread, NULL);
        pthread_join(output_thread, NULL);

        fclose(infile);
        fclose(outfile);
    }
    gettimeofday(&tp_e, &tzp_e);
    if ((outfile = fopen("time.txt", "w")) == NULL)
    {
        perror("out open error~\n");
    }
    fprintf(outfile, "Total execution time =%ld\n", tp_e.tv_sec - tp_s.tv_sec);
    fclose(outfile);
}

// Time complexity is O(n^3) and space complexity is O(n^2) where n is the number of rows and columns of the matrix.