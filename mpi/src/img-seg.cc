#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

int mpirank, p;
int pN, d, C, N;

/* 2d array allocated in contigous way */
double** alloc_2d_double(int rows, int cols) {
  int i;
  double* data = (double*)malloc(rows * cols * sizeof(double));
  double** array = (double**)malloc(rows * sizeof(double*));
  for (i = 0; i < rows; i++)
    array[i] = &(data[cols * i]);
  return array;
}

void printData(double** data, int row, int col) {
  int k, j;
  for (k = 0; k < row; k++) {
    for (j = 0; j < col; j++) {
      printf("%f ", data[k][j]);
    }
    printf("\n");
  }
}

void readImagePartial(cv::Mat& image, double** data) {
  uchar* p = image.ptr<uchar>(0);
  int start = mpirank * pN * 3;
  int end = (mpirank + 1) * pN * 3;

  int i = start;
  int j = 0;
  while (i < end) {
    data[j][0] = p[i];
    data[j][1] = p[i + 1];
    data[j][2] = p[i + 2];
    j++;
    i += 3;
  }
}

void getInitialClustersFromImage(cv::Mat& image, double** centers) {
  int i;
  for (i = 0; i < C; i++) {
    centers[i][0] = rand() % 255;
    centers[i][1] = rand() % 255;
    centers[i][2] = rand() % 255;
  }
}

void segmentImage(cv::Mat& image, double** centers) {
  uchar* p = image.ptr<uchar>(0);
  int j, k, l;
  int min_i;
  double c_res, min_res;
  for (j = 0; j < N * 3; j += 3) {
    min_res = INFINITY;
    min_i = -1;

    for (k = 0; k < C; k++) {
      c_res = 0.0;

      // norm
      for (l = 0; l < d; l++) {
        c_res += pow(abs(p[j + l] - centers[k][l]), 2);
      }
      c_res = sqrt(c_res);
      if (c_res < min_res) {
        min_i = k;
        min_res = c_res;
      }
    }
    for (l = 0; l < d; l++) {
      p[j + l] = (unsigned char)centers[min_i][l];
    }
  }
  imwrite("images/test.jpg", image);
}

int main(int argc, char* argv[]) {
  srand(time(NULL));

  // GLOBAL  int mpirank, p;
  // GLOBAL int pN,d,C,N;
  int i, j, k, l;
  char f_name[100];
  int N_ITER;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  /* get name of host running MPI process */
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  // printf("Rank %d/%d running on %s.\n", mpirank, p, processor_name);

  if (argc != 4 && mpirank == 0) {
    fprintf(stderr, "Usage: ./k_means dim C data n_iter\n");
    fprintf(stderr, "C: number of clusters\n");
    fprintf(stderr, "data: image file path\n");
    fprintf(stderr, "n_iter: number of iterations\n");
    MPI_Abort(MPI_COMM_WORLD, 0);
  }

  // READ arguments
  sscanf(argv[1], "%d", &C);
  sscanf(argv[3], "%d", &N_ITER);

  strcpy(f_name, argv[2]);
  cv::Mat image = cv::imread(f_name, cv::IMREAD_COLOR);
  int channels = image.channels();

  int nRows = image.rows;
  int nCols = image.cols * channels;

  if (image.isContinuous()) {
    nCols *= nRows;
    nRows = 1;
  }
  d = 3;
  N = nCols / 3;

  pN = N / p;
  if (mpirank == 0)
    printf("%dd data,Total: %d points,%d points per process from file %s\n", d,
           N, pN, f_name);

  double** centers;
  double** class_data_sum;
  int local_class_residual_count[C];
  int global_class_residual_count[C];
  centers = alloc_2d_double(C, d);
  class_data_sum = alloc_2d_double(C, d);

  if (mpirank == 0) {
    getInitialClustersFromImage(image, centers);
  }

  MPI_Bcast(&(centers[0][0]), C * d, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  if (mpirank == 0) {
    cout << "initial clusters: " << endl;
    printData(centers, C, 3);
  }

  double** data;
  data = alloc_2d_double(pN, d);
  readImagePartial(image, data);

  /* timing */
  double start, end;
  MPI_Barrier(MPI_COMM_WORLD);
  start = MPI_Wtime();

  for (i = 0; i < N_ITER; i++) {
    // For each turn zero the sums

    for (k = 0; k < C; k++) {
      for (j = 0; j < d; j++) {
        class_data_sum[k][j] = 0;
      }
      local_class_residual_count[k] = 0;
    }

// For each data point get the clusters cluster center and add the residual
#pragma omp parallel
    {
      double** priv_class_data_sum;
      priv_class_data_sum = alloc_2d_double(C, d);
      for (int i = 0; i < C; i++)
        for (int j = 0; j < d; j++)
          priv_class_data_sum[i][j] = 0;

      int priv_local_class_residual_count[C];
      for (int i = 0; i < C; i++)
        priv_local_class_residual_count[i] = 0;

#pragma omp for
      for (int j = 0; j < pN; j++) {
        double min_res = INFINITY;
        int min_i = -1;

        for (int k = 0; k < C; k++) {
          double c_res = 0.0;

          // norm
          for (int l = 0; l < d; l++) {
            c_res += pow(abs(data[j][l] - centers[k][l]), 2);
          }
          c_res = sqrt(c_res);
          if (c_res < min_res) {
            min_i = k;
            min_res = c_res;
          }
        }
        for (int l = 0; l < d; l++) {
          priv_class_data_sum[min_i][l] += data[j][l];
        }
        priv_local_class_residual_count[min_i] += 1;
      }
      for (int i = 0; i < C; i++)
        for (int j = 0; j < d; j++)
#pragma omp atomic
          class_data_sum[i][j] += priv_class_data_sum[i][j];

      for (int i = 0; i < C; i++)
#pragma omp atomic
        local_class_residual_count[i] += priv_local_class_residual_count[i];
    }

    /* cout << "MPIRANK: " << mpirank << endl;
    cout<< "summs: "<<endl;
    printData(class_data_sum, C, 3);
    cout<< "counters: "<<endl;
    for (int x = 0; x < C; x++)
        cout << local_class_residual_count[x] << endl; */
    // Share the sums of points and the countrs
    MPI_Allreduce(&(class_data_sum[0][0]), &(centers[0][0]), C * d, MPI_DOUBLE,
                  MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(local_class_residual_count, global_class_residual_count, C,
                  MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (k = 0; k < C; k++) {
      for (l = 0; l < d; l++) {
        if (global_class_residual_count[k] > 0)
          centers[k][l] /= global_class_residual_count[k];
        else
          centers[k][l] = 0;
      }
    }
    // if (mpirank==1){
    //   printf("rank1: centers\n");
    //   printData(centers,C,d);
    // }
  }
  // if (mpirank==1){
  //     printf("rank1:\n");
  //     printData(centers,C,d);
  // }
  if (mpirank == 0) {
    printf("centers:\n");
    printData(centers, C, d);
  }

  /* timing */
  MPI_Barrier(MPI_COMM_WORLD);
  end = MPI_Wtime();

  if (0 == mpirank) {
    printf("Time elapsed is %f seconds.\n", end - start);
    segmentImage(image, centers);
  }

  free(data[0]);
  free(data);
  free(centers[0]);
  free(centers);

  MPI_Finalize();
  return 0;
}
