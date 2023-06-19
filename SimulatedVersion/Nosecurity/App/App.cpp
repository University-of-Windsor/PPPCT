
using namespace std;
#include <omp.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "time.h"
#include "limits.h"
#include "float.h"
#include "math.h"
#include "string.h"
#include <iostream>
#include "FileIO.h"
#include <unistd.h>
#include <pwd.h>
#include <assert.h>
#include <fstream>
#include <getopt.h>

#define MAX_PATH FILENAME_MAX
#define MAX_ITER 3000
#define THRESHOLD 1e-11

int number_of_points_global;
int number_of_threads_global;
int number_of_iterations_global;
int K_global;
float** data_points_global;
double*** iter_centroids_global;
int* data_point_cluster_global;
int** iter_cluster_count_global;
int rows_global=0;
int cols_global=0;

// Defined global delta
double delta_global = THRESHOLD + 1;

//#include "App.h"
//#include "utils.h"

void kmeans_omp(int num_threads, int rows,int cols, int K, int* data_points_cluster_id,double*** iter_centroids, int number_of_iterations,float ** data_points);
void Fit_Transform2(int * tid, int Number_of_clusters, int Number_of_cols, int Number_of_rows,double &delta,int& number_of_iterations_global,int* data_point_cluster_global, double *** iterate_centroids_global,float ** data_points);
void find_distance(int start,int end,int Number_of_clusters,int Number_of_cols,int Number_of_rows,int iter_counter, int * myarray, int * points_inside_cluster_count,float ** cluster_points_sum,double ***iterate_centroids_global, float ** dataset3);
void find_centroids(int Number_of_clusters,int Number_of_cols,int iter_counter, int * points_inside_cluster_count,float ** cluster_points_sum, int **iter_cluster_count_global,double ***iterate_centroids_global);
void find_delta(int Number_of_clusters,int Number_of_cols,int iter_counter,double ***iterate_centroids_global,double *delta);

/* Application entry */
int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);


     int rows = 1000;
     int cols = 1200;
     rows_global=cols;
     cols_global=rows;			// Number of data points (input)
    int K = 1;                          //Number of clusters to be formed (input)
    int num_threads=4;
    char input[5];
    int* data_points = &rows;		//Data points (input)
    int* cluster_points = &rows;	//clustered data points (to be computed)
    double*** iter_centroids;              //centroids of each iteration (to be computed)
    int number_of_iterations = 300;      //no of iterations performed by algo (to be computed)
    //---------------------------------------------------------------------
    int x = 1;
    double start_time = 0, end_time = 0;
    double computation_time = 0;
    float** data_points2;
    int** data_labels;
    char * file=new char [100];
    char * dataset=new char[100];
    char * numberOfClusters=new char [2];
    char * numberOfCells=new char [10];
    char * numberOfGenes=new char [10];

    if(argv[1]!=""){
    	dataset=argv[1];
    }
    if(argv[2]!=""){
    	numberOfClusters=argv[2];
    }
    if(argv[3]!=""){
    	numberOfCells=argv[3];
    }
    if(argv[4]!=""){
    	numberOfGenes=argv[4];
    }


    /*
  * start pre-processing with NMF
  */
    K = atoi(numberOfClusters);
    rows_global=atoi(numberOfCells);
    cols_global = atoi(numberOfGenes);

    printf("\nEnter number of threads to be used: ");
    scanf("%d", &num_threads);
    start_time = omp_get_wtime();
    FileIO processFile=FileIO();
    std::string python_command {"python3.10 preprocessing.py "};
    python_command+=dataset;
    python_command+=" ";
    python_command+=numberOfClusters;
    system(const_cast<char*>(python_command.c_str()));

    cout << "python_command: " << python_command;
    data_points2=processFile.readCsvFile("W.csv",rows_global,K+1);

    end_time = omp_get_wtime();
    double read_time=end_time-start_time;
    start_time = omp_get_wtime();
  
  /*
      * Start parallel processing
      */



    cols_global=K+1;
    kmeans_omp(num_threads, rows_global,cols_global, K, cluster_points, iter_centroids, number_of_iterations,data_points2);
    end_time = omp_get_wtime();

    ////////THIS PART IS TO GENERATE UNSEALED LABELS THAT IS SPECIFIC TO iDASH EVALUATORS
    char  labels[50]="unsealed_labels.txt";
    processFile.clusters_out(labels,rows_global, data_point_cluster_global);
    ////////////////////////////////////////////////////////////////////////////////////


    computation_time = end_time - start_time;
    char time[100]="computation time";


    processFile.computation_time_out(time,computation_time);

    cout << "\nTime Taken: " << computation_time<< "\n";
cout << "\nTime Pre-processing : " << read_time  << "\n";
cout << "\nTime Taken: " << computation_time+read_time  << "\n";



    return 0;
}

void kmeans_omp(int num_threads, int rows, int cols, int K, int* data_points_cluster_id,double*** iter_centroids, int number_of_iterations,float ** data_points)
{

    // Initialize global variables
    number_of_points_global = rows;
    number_of_threads_global = num_threads;
    number_of_iterations_global = 0;
    K_global = K;

    data_points_cluster_id = (int*)malloc(rows * sizeof(int));   //Allocating space of 4 units each for N data points
    data_point_cluster_global = data_points_cluster_id;


    iter_centroids_global = new double ** [MAX_ITER+1];
    for(int i=0;i<MAX_ITER+1;i++){
        iter_centroids_global[i] = new double * [K];
        for (int j = 0; j < K; j++){
            iter_centroids_global[i][j] = new double [cols];
            for (int k = 0; k < cols; k++) iter_centroids_global[i][j][k] = 0;
        }
    }
	//int random_value=rand() % 100 +10;
	double * mean_values=new double[cols];
	for(int i=0;i<rows;i++){
                //int random_value=rand() * 10;
		for(int j=0;j<cols;j++){
			mean_values[j]+=(double)data_points[i][j];
		}
	}

    // Assigning first K points to be initial centroids
    int i = 0;
    for (i = 0; i < K; i++)
    {
    int random_value = rand() % rows;
        for (int j = 0; j < cols; j++) {
            //cout << "Random value: " << mean_values[j] << endl;         
            iter_centroids_global[0][i][j] = data_points[random_value][j];//random_value;//(i + 1) * mean_values[j]/rows;
        }
    }
    /*
//0.911242996	0.000741751	0.152542238	0.308471013

    iter_centroids_global[0][0][0]=0.911242996;
    iter_centroids_global[0][0][1]=0.000741751;
    iter_centroids_global[0][0][2]=0.152542238;
    iter_centroids_global[0][0][3]=0.308471013;

//0.406431756	1.174972094	0.211517937	0.626469327

    iter_centroids_global[0][1][0]=(double)(0.911242996+0.614158267)/2;
    iter_centroids_global[0][1][1]=(double)(0.000741751+0.186882471)/2;
    iter_centroids_global[0][1][2]=(double)(0.152542238+1.0366507)/2;
    iter_centroids_global[0][1][3]=(double)(0.308471013+0.518442855)/2;

//0.614158267	0.186882471	1.0366507	0.518442855




    iter_centroids_global[0][2][0]=0.280638652;
    iter_centroids_global[0][2][1]=0.344845907;
    iter_centroids_global[0][2][2]=0;
    iter_centroids_global[0][2][3]=0.177482547;
*/
    iter_cluster_count_global = (int**)malloc(MAX_ITER * sizeof(int*));
    for (i = 0; i < MAX_ITER; i++)
    {
        iter_cluster_count_global[i] = (int*)calloc(K, sizeof(int));
    }

    omp_set_nested(1);
    // Creating threads
    omp_set_num_threads(num_threads);

    //printf("\nProcessing...Please wait!\n");

#pragma omp parallel shared(cols,rows,data_point_cluster_global,iter_centroids_global,delta_global,data_points) //shared(number_of_iterations_global,iter_centroids_global)
    {
    	int ID=omp_get_thread_num();
        printf("Thread: %d created!\n", ID);
       Fit_Transform2(&ID,K,cols,rows,
    		   delta_global,number_of_iterations_global,data_point_cluster_global,iter_centroids_global,data_points);
    }

    // Record number_of_iterations
    number_of_iterations = number_of_iterations_global;

    // Record number of iterations and store iter_centroids_global data into iter_centroids
    int iter_centroids_size = number_of_iterations + 1;
    iter_centroids=new double **[iter_centroids_size];
    for (int i = 0; i < iter_centroids_size; i++)
    {
        iter_centroids[i]=new double *[K];
        for(int j=0;j<K;j++){
            iter_centroids[i][j]=new double[cols];
            for(int l=0;l<cols;l++)
                iter_centroids[i][j][l]=iter_centroids_global[i][j][l];
        }
    }
}




void Fit_Transform2(int * tid, int Number_of_clusters, int Number_of_cols, int Number_of_rows,
		double &delta,int& number_of_iterations_global,int* data_point_cluster_global, double *** iterate_centroids_global,float ** data_points)
{
    int* id = (int*)tid;

    // Assigning data points range to each thread
    int data_length_per_thread = number_of_points_global / number_of_threads_global;
    int start = (*id) * data_length_per_thread;
    int end = start + data_length_per_thread;
    if (end + data_length_per_thread > number_of_points_global)
    {
        //To assign last undistributed points to this thread for computation, change end index to number_of_points_global
        end = number_of_points_global;
        data_length_per_thread = number_of_points_global - start;
    }


    double min_dist=99999;
    double current_dist=0;

    //float cluster_points_sum[Number_of_clusters][Number_of_rows];
    float ** cluster_points_sum =new float *[Number_of_clusters];
    for(int i=0;i<Number_of_clusters;i++){
    	cluster_points_sum[i]=new float[Number_of_rows];
    }
    // Cluster id associated with each point
    int *myarray=new int[data_length_per_thread];
    int point_to_cluster_id[data_length_per_thread]={};
    int points_inside_cluster_count[Number_of_clusters];
    int iter_counter = 0;
    int i = 0, j = 0;
   // Initialize cluster_points_sum or centroid to 0.0
    for (i = 0; i < Number_of_clusters; i++)
       for(j=0; j< Number_of_cols ;j++)
            cluster_points_sum[i][j] = 0;
   // Initialize number of points for each cluster to 0
   for (i = 0; i < Number_of_clusters; i++)
       points_inside_cluster_count[i] = 0;

   while ((delta > THRESHOLD) && (iter_counter < MAX_ITER))
   {
	  //cout << "delta: "<<  delta <<endl;
	   find_distance(start,end,Number_of_clusters,Number_of_cols,Number_of_rows,iter_counter,myarray,points_inside_cluster_count,cluster_points_sum,iterate_centroids_global,data_points);


	#pragma omp critical
        {
        	find_centroids(Number_of_clusters,Number_of_cols,iter_counter,points_inside_cluster_count,cluster_points_sum, iter_cluster_count_global,iterate_centroids_global);
        }

	#pragma omp barrier
        if ((*id)== 0)
        {
        	find_delta(Number_of_clusters,Number_of_cols,iter_counter,iterate_centroids_global,&delta);
            number_of_iterations_global++;
        }

        // Wait for all thread to arrive and update the iter_counter by +1
	#pragma omp barrier
        iter_counter++;
//cout << "delta: " << delta << "    Iter: "<< iter_counter<< endl;
        
   }   //End of while loop

        for (int i = start; i < end; i++)
        {
            // Assign points to clusters
            data_point_cluster_global[i] = myarray[i - start];
            assert(myarray[i - start] >= 0 && myarray[i - start] < Number_of_clusters);
        }
        cout << "[0]: "<<points_inside_cluster_count[0]<<endl;
	cout << "[1]: "<<points_inside_cluster_count[1]<<endl;
	cout << "[2]: "<<points_inside_cluster_count[2]<<endl;
}



void find_delta(int Number_of_clusters,int Number_of_cols,int iter_counter,double ***iterate_centroids_global,double *delta)
{

    double temp_delta=0;
//#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < Number_of_clusters; i++)
    {
        for (int j = 0; j < Number_of_cols; j++) {
        	temp_delta += (iterate_centroids_global[iter_counter + 1][i][j] - iterate_centroids_global[iter_counter][i][j]) * 
        		(iterate_centroids_global[iter_counter + 1][i][j] - iterate_centroids_global[iter_counter][i][j]);
		//temp_delta2 += max(iterate_centroids_global[iter_counter + 1][i][j] , iterate_centroids_global[iter_counter][i][j]);
	
       }
    }
    *delta=temp_delta;
    cout << (*delta) << endl;
}
void find_centroids(int Number_of_clusters,int Number_of_cols,int iter_counter, int * points_inside_cluster_count,float ** cluster_points_sum, int **iter_cluster_count_global,double ***iterate_centroids_global)
{
    //#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < Number_of_clusters; i++)
    {
        if (points_inside_cluster_count[i] == 0){
            continue;
        }
        for (int j = 0; j < Number_of_cols; j++) {
        	iterate_centroids_global[iter_counter + 1][i][j] = (iterate_centroids_global[iter_counter + 1][i][j]
                * iter_cluster_count_global[iter_counter][i] + cluster_points_sum[i][j])
                / (float)(iter_cluster_count_global[iter_counter][i] + points_inside_cluster_count[i]);
                //cout << "i: " << i << " j: " << j << "     " << iterate_centroids_global[iter_counter + 1][i][j] << endl;
                //cout << "i: " << i << " j: " << j << "     " << cluster_points_sum[i][j]<< endl;
        }
     }
}

void find_distance(int start,int end,int Number_of_clusters, int number_of_cols,int Number_of_rows,int iter_counter, int * myarray, int * points_inside_cluster_count,float ** cluster_points_sum,double ***iterate_centroids_global, float ** dataset3)
{

    int output;
    //#pragma omp parallel for schedule(dynamic)
    for (int i = start; i < end; i++)
     {
      double min_dist = DBL_MAX;
      for (int j = 0; j < Number_of_clusters; j++)
      {
    	//cout << "\n dataset [i][j]: " << dataset3[i][j];
          double global_dist=0;
          for (int k = 0; k < number_of_cols; k++) {
              double value=(iterate_centroids_global[iter_counter][j][k] - (double)dataset3[i][k]);
              global_dist+=value * value;
              //cout << "\n" <<global_dist;
          }
          double dist=sqrt(global_dist);
          if (dist < min_dist)
          {
              min_dist = dist;
              myarray[i-start]=j;
          }
      }
      points_inside_cluster_count[myarray[i - start]] += 1;
      for (int j = 0; j < number_of_cols; j++)
     	 cluster_points_sum[myarray[i - start]][j] += (float)dataset3[i][j];
     }
}

