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

struct thread_data
{
    int thread_id;
    int start_row;
    int end_row;
};

void *matrix_multiply(void *thread_arg)
{
    struct thread_data *data = (struct thread_data *)thread_arg;
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
            matrix3[i][j] = val;
        }
    }
    pthread_exit(NULL);
}


int main()
{
    int i, j, k, x;
    struct timeval tp_s, tp_e;
    struct timezone tzp_s, tzp_e;

    // Set matrix values to 0
    for (i = 0; i < raw; i++)
    {
        for (j = 0; j < col; j++)
        {
            matrix1[1][1] = matrix2[1][1] = matrix3[1][1] = 0;
        }
    }
    gettimeofday(&tp_s, &tzp_s);
    for (x = 0; x < 1000; x++)
    {
        sprintf(infilename, "in%d.txt", x + 1);
        sprintf(outfilename, "out%d.txt", x + 1);
        if ((infile = fopen(infilename, "r")) == NULL)
        {
            perror("in open error~\n");
        }
        fscanf(infile, "%d", &raw);
        fscanf(infile, "%d", &col);
        printf("%d\n", x);
        for (i = 0; i < raw; i++)
        {
            for (j = 0; j < col; j++)
            {
                fscanf(infile, "%f", &val);
                matrix1[i][j] = matrix2[i][j] = val;
            }
        }

        pthread_t threads[NUM_THREADS];
        struct thread_data thread_data_array[NUM_THREADS];
        int rows_per_thread = raw / NUM_THREADS;
        for (i = 0; i < NUM_THREADS; i++)
        {
            thread_data_array[i].thread_id = i;
            thread_data_array[i].start_row = i * rows_per_thread;
            thread_data_array[i].end_row = (i + 1) * rows_per_thread;
            int rc = pthread_create(&threads[i], NULL, matrix_multiply, &thread_data_array[i]);
            if (rc)
            {
                printf("Error creating thread\n");
                exit(-1);
            }
        }

        // Wait for all threads to complete
        for (i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }

        if ((outfile = fopen(outfilename, "w")) == NULL)
        {
            perror("out open error~\n");
        }
        fprintf(outfile, "%d\n%d\n", raw, col);
        for (i = 0; i < raw; i++)
        {
            for (j = 0; j < col; j++, val++)
            {
                fprintf(outfile, "%f\n", matrix3[i][j]);
            }
        }
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
    return 0;
}
