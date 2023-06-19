#include "csv.h"


float** readCsvFile(char* file_name, const int num_rows, const int num_col);
float** readCsvFileTranspose(char* file_name, const int num_rows, const int num_col);

void centroids_out(char *filename,int K, int number_of_iterations, double*** iter_centroids, int cols);

void clusters_out(char *filename,int N, int* cluster_points);

void computation_time_out(char *filename,double computation_time);



class FileIO {
    public:
		//int** readCsvFile(char* file_name, const int num_rows, const int num_col);
		//int** readCsvFileTranspose(char* file_name, const int num_rows, const int num_col);
		//void centroids_out(char *filename,int K, int number_of_iterations, float*** iter_centroids, int cols);
		//void centroids_out(char *filename,int K, int number_of_iterations, float*** iter_centroids, int cols);
		//void clusters_out(char *filename,int N, int* cluster_points);
		//void computation_time_out(char *filename,double computation_time);
		FileIO(){}

		float **readCsvFileTranspose(char* file_name, const int num_rows, const int num_col)
		{
		    float** data_points, ** new_points;
		    data_points = readCsvFile(file_name, num_rows, num_col);
		    int i, j;
		    new_points = new float* [num_col];
		    for (i = 0; i < num_col; i++) {
		        new_points[i] = new float[num_rows];
		        for (j = 0; j < num_rows; j++) {
		           new_points[i][j] = data_points[j][i];
		        }
		    }
		    return new_points;
		}
		float **readCsvFile(char* file_name, const int num_rows, const int num_col)
		{
		    float** data_points;
		    data_points = new float* [num_rows];
		    int i = 0;
		    io::CSVReader<3> in(file_name);
		    while (char* line = in.next_line()) {//in.read_row(vendor, size, speed)){
		        char* values = strtok(line, ",");
		        int j = 0;
		        data_points[i] = new float[num_col];
		        while (values != NULL) {
		            data_points[i][j] = atof(values);
		            values = strtok(NULL, ",");
		            j++;
		        }
		        i++;
		    }
		    return data_points;
		}
		void centroids_out(char *filename,int K,int number_of_iterations, double*** iter_centroids,int cols)
		{
		    //char * centroid_filename = "centroid_threads";
		    strcat(filename, ".txt");
		    FILE* fout = fopen(filename, "w");
		    int i = 0;
		    for (i = 0; i < number_of_iterations + 1; i++)
		    {
		        int j = 0;
		        for (j = 0; j < K; j++)
		        {
		            for (int k=0;k<cols;k++){
		                fprintf(fout,"%f, ",iter_centroids[i][j][k]);
		            }
		        }
		        fprintf(fout, "\n");
		    }
		    fclose(fout);
		}
		void clusters_out(char *filename,int N, int* cluster_points)
		{
		    //char *cluster_filename = "clusters_threads";
		    strcat(filename, ".txt");
		    FILE* fout = fopen(filename, "w");
		    int i = 0;
		    for (i = 0; i < N; i++)
		    {
		        fprintf(fout, "%d\n",cluster_points[i]);
		    }
		    fclose(fout);
		}
		void computation_time_out(char *filename,double computation_time)
		{
		    //char * time_file_omp = "Computation_Time";
		    strcat(filename, ".txt");

		    FILE* fout = fopen(filename, "w");
		    fprintf(fout, "%f\n", computation_time);
		    fclose(fout);
		}



};
