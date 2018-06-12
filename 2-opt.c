#include <stdio.h>
#include <math.h>

#define MAX_SIZE 50
#define ALFA 0.5
#define STATES 10000
#define MAX_TIMES 1000
#define MAX_ITER 20

int N = 0; /* customers + deposit */

int earlyTime[MAX_SIZE];
int lateTime[MAX_SIZE];

int c[MAX_SIZE][MAX_SIZE];
int t[MAX_SIZE][MAX_SIZE];

void readInput(char fileName[]) {
  FILE* file = fopen (fileName, "r");
  int x[MAX_SIZE];
  int y[MAX_SIZE];
  int service[MAX_SIZE];
  int tmp = 0;

  fscanf (file, "%d", &N);  
  printf ("tot = %d\n", N);
  for (int i = 0; i < N; i++)
  { 
    fscanf (file, "%d", &tmp);
    fscanf (file, "%d", &x[i]);
    fscanf (file, "%d", &y[i]);
    fscanf (file, "%d", &tmp);
    fscanf (file, "%d", &earlyTime[i]);
    fscanf (file, "%d", &lateTime[i]);
    fscanf (file, "%d", &service[i]);
    //printf("x = %d, y = %d, a = %d, b = %d, service = %d\n", x[i], y[i], earlyTime[i], lateTime[i], service[i]);

  }
  fclose (file);    

  printf("\n\n** DISTANCES\n\n");
  int dist = 0;
  for (int j = 0; j < N; j++) {
    for (int u = 0; u < N; u++) {
      dist = sqrt(pow(x[j] - x[u], 2) + pow(y[j] - y[u], 2));
      c[j][u] = dist;
      t[j][u] = dist + service[u];
      printf("%d - %d -> %d\n", j, u, c[j][u]);
    }
  } 
}

int max(int a, int b) {
  return a >= b ? a : b;
}

double calculateDelayCost(int arrivalTime, int client) {
  int delay = max(0, arrivalTime - lateTime[client]);
  return delay * ALFA;
}

double calculatePathCost(int path[]) {
  int time = 0;
  double f = 0.0;
  for (int i = 0; i < N; i++) {
    time = max(earlyTime[i+1], time + t[path[i]][path[i+1]]);
    f = f + c[path[i]][path[i+1]] + calculateDelayCost(time, path[i+1]);
  } 
  return f;
}

void arrayCopy(int array1[], int array2[], int dim) {
  for (int i = 0; i < dim; i++) {
    array2[i] = array1[i];
  }
}

void ThreeOptSwitch(int tour[], int i1, int i2, int i3, int switchIndex, int newPath[]) {
  int j = 0; // new path index
  for (j; j <= i1; j++) {
    newPath[j] = tour[j];
  }

  switch (switchIndex) {
    case 0: // A, B<-, C->, D
      for (int y = i2; y > i1; y--) {
        newPath[j] = tour[y];
        j++;
      }
      for (j; j <= i3; j++) {
        newPath[j] = tour[j];
      }
      break;

    case 1: // A, B->, C<-; D
      for (j; j <= i2; j++) {
        newPath[j] = tour[j];
      }
      for (int y = i3; y > i2; y--) {
        newPath[j] = tour[y];
        j++;
      }
      break;

    case 2: // A, C<-, B<-, D
      for (int y = i3; y > i1; y--) {
        newPath[j] = tour[y];
        j++;
      }
      break;
    
    case 3: // A, C->, B->, D
      for (int y = i2 + 1; y <= i3; y++) {
        newPath[j] = tour[y];
        j++;
      }
      for (int y = i1 + 1; y <= i2; y++) {
        newPath[j] = tour[y];
        j++;
      }
      break;

    case 4: // A, B<-, C<-, D
      for (int y = i2; y > i1; y--) {
        newPath[j] = tour[y];
        j++;
      }
      for (int y = i3; y > i2; y--) {
        newPath[j] = tour[y];
        j++;
      }
      break;

    case 5: // A, C->, B<-, D
      for (int y = i2 + 1; y <= i3; y++) {
        newPath[j] = tour[y];
        j++;
      }
       for (int y = i2; y > i1; y--) {
        newPath[j] = tour[y];
        j++;
      }
      break;

    case 6: // A, C<-, B->, D
      for (int y = i3; y > i2; y--) {
        newPath[j] = tour[y];
        j++;
      }
      for (int y = i1 + 1; y <= i2; y++) {
        newPath[j] = tour[y];
        j++;
      }
      break;
  }

  for (j; j <= N; j++) {
    newPath[j] = tour[j];
  }

}

int main(int argc, char *argv[]) {
  printf("%s\n\n", argv[1]);
  readInput(argv[1]);
  int path[N+1];
  for (int i = 0; i < N; i++) { // inital path, 0-1-2-3-...-N-0.
    path[i] = i;
  }
  path[N] = 0;
  double optimalCost = calculatePathCost(path);
  int optimalTour[N+1];
  arrayCopy(path, optimalTour, N+1);
  int improvement = 1;
  int newTour1[N+1];
  int newTour2[N+1];
  while (improvement == 1) {
    improvement = 0;
    for (int i = 1; i < N - 1; i++) {
      for (int j = i+1; j < N; j++) {
        int dec = 0;
        for (int h = 0; h < N+1; h++) {
          if (h < i || h > j) {
            newTour1[h] = optimalTour[h];
          } else {
            newTour1[h] = optimalTour[j - dec];
            dec++;
          }
        }

        if (calculatePathCost(newTour1) < optimalCost) {
          improvement = 1;
          arrayCopy(newTour1, optimalTour, N+1);
          optimalCost = calculatePathCost(newTour1);
        }

        for (int g = 0; g < N+1; g++) {
          newTour2[g] = newTour1[N - g];
        }

        if (calculatePathCost(newTour2) < optimalCost) {
          improvement = 1;
          arrayCopy(newTour2, optimalTour, N+1);
          optimalCost = calculatePathCost(newTour2);
        }
      }
    }
  }

  printf("optimal tour = ");
  for (int i = 0; i < N+1; i++) {
    printf("%d, ", optimalTour[i]);
  }

  printf("\nCost = %lf\n", optimalCost);


}