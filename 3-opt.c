#include <stdio.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 50
#define ALFA 0.5
#define MAX_DELAY 1000

int N = 0; /* customers + deposit */
int x[MAX_SIZE];
int y[MAX_SIZE];

int earlyTime[MAX_SIZE];
int lateTime[MAX_SIZE];

int c[MAX_SIZE][MAX_SIZE];
int t[MAX_SIZE][MAX_SIZE];

int tours[MAX_DELAY][MAX_SIZE];
double roadTours[MAX_DELAY] = {INFINITY};

void readInput(char fileName[]) {
  FILE* file = fopen (fileName, "r");
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
  }
  fclose (file);    

  int dist = 0;
  for (int j = 0; j < N; j++) {
    for (int u = 0; u < N; u++) {
      dist = sqrt(pow(x[j] - x[u], 2) + pow(y[j] - y[u], 2));
      c[j][u] = dist;
      t[j][u] = dist + service[u];
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
    time = max(earlyTime[path[i+1]], time + t[path[i]][path[i+1]]);
    f = f + c[path[i]][path[i+1]] + calculateDelayCost(time, path[i+1]);
  } 
  return f;
}

double calculatePathRoadCost(int path[]) {
  int cost = 0.0;
  for (int i = 0; i < N; i++) {
    cost += c[path[i]][path[i+1]];
  }
  return cost;
}

void arrayCopy(int array1[], int array2[], int dim) {
  for (int i = 0; i < dim; i++) {
    array2[i] = array1[i];
  }
}

void revertArray(int array[], int dim) {
  int tmp = 0;
  for (int i = 0; i < dim / 2; i++) {
    tmp = array[dim - 1 - i];
    array[dim - 1 - i] = array[i];
    array[i] = tmp;
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

void checkDominance() {
  double minCost = INFINITY;
  for (int i = 0; i < MAX_DELAY; i++) {
    if (roadTours
  [i] >= minCost) {
      roadTours
    [i] = INFINITY;
    } else {
      minCost = roadTours
    [i];
    }
  }
}

int main(int argc, char *argv[]) {
  printf("%s\n\n", argv[1]);
  readInput(argv[1]);
  int CurrentOptimalTour[N+1];
  for (int i = 0; i < N; i++) { // inital path, 0-1-2-3-...-N-0.
    CurrentOptimalTour[i] = i;
  }
  CurrentOptimalTour[N] = 0;
  double optimalCost = calculatePathCost(CurrentOptimalTour);
  double currentOptimalCost = optimalCost;
  int optimalTour[N+1];
  int tmpTour[N+1];
  arrayCopy(CurrentOptimalTour, optimalTour, N+1);
  int improvement = 1;
  while (improvement == 1) {
    improvement = 0;
    for (int i = 0; i < N - 2; i++) { // i1
      for (int j = i+1; j < N - 1; j++) { // i2
        for (int t = j+1; t < N; t++) { // i3
          for (int h = 0; h < 7; h++) { // switchIndex
            ThreeOptSwitch(optimalTour, i, j, t, h, tmpTour);
            if (calculatePathCost(tmpTour) < currentOptimalCost) {
              improvement = 1;
              currentOptimalCost = calculatePathCost(tmpTour);
              arrayCopy(tmpTour, CurrentOptimalTour, N+1);
            }
            revertArray(tmpTour, N+1);
            if (calculatePathCost(tmpTour) < currentOptimalCost) {
              improvement = 1;
              currentOptimalCost = calculatePathCost(tmpTour);
              arrayCopy(tmpTour, CurrentOptimalTour, N+1);
            }
          }
          if (improvement == 1 && currentOptimalCost < optimalCost) {
            optimalCost = currentOptimalCost;
            arrayCopy(CurrentOptimalTour, optimalTour, N+1);
          }
        }
      }
    }
  }

  double thresholdCost = 1.1 * optimalCost;
  double tourCost = 0;
  double roadCost = 0;
  int tourDelay = 0;
  for (int i = 0; i < MAX_DELAY; i++) {
    roadTours[i] = INFINITY;
  }

  improvement = 1;
  while (improvement == 1) {
    improvement = 0;
    for (int i = 0; i < N - 2; i++) { // i1
      for (int j = i+1; j < N - 1; j++) { // i2
        for (int t = j+1; t < N; t++) { // i3
          for (int h = 0; h < 7; h++) { // switchIndex
            ThreeOptSwitch(optimalTour, i, j, t, h, tmpTour);
            tourCost = calculatePathCost(tmpTour);
            if (tourCost < thresholdCost) {
              roadCost = calculatePathRoadCost(tmpTour);
              tourDelay = (tourCost - roadCost) * (1/ALFA);
              if (roadTours[tourDelay] > roadCost) {
                improvement = 1;
                roadTours[tourDelay] = roadCost;
                arrayCopy(tmpTour, tours[tourDelay], N+1);
                if (currentOptimalCost < tourCost) {
                  currentOptimalCost = tourCost;
                  arrayCopy(tmpTour, CurrentOptimalTour, N+1);
                }
              } 
            }
            revertArray(tmpTour, N+1);
            tourCost = calculatePathCost(tmpTour);
            if (tourCost < thresholdCost) {
              roadCost = calculatePathRoadCost(tmpTour);
              tourDelay = (tourCost - roadCost) * (1/ALFA);
              if (roadTours[tourDelay] > roadCost) {
                improvement = 1;
                roadTours[tourDelay] = roadCost;
                arrayCopy(tmpTour, tours[tourDelay], N+1);
                if (currentOptimalCost < tourCost) {
                  currentOptimalCost = tourCost;
                  arrayCopy(tmpTour, CurrentOptimalTour, N+1);
                }
              } 
            }
          }
        }
      }
    }
  }

  if (improvement == 1) {
    optimalCost = currentOptimalCost;
    arrayCopy(CurrentOptimalTour, optimalTour, N+1);
  }
  checkDominance();

  for (int i = 0; i < MAX_DELAY; i++) {
    if (roadTours[i] < INFINITY) {
      printf("*******\nTour = ");
      char fileName[20] = "results/tsp";
      char buff[12];
      sprintf(buff, "%d", i);
      strcat(fileName, buff);
      FILE *f = fopen(fileName, "ab+");
      for (int j = 0; j < N; j++) {
        printf("%d, ", tours[i][j]);
        fprintf(f, "%d %d %d %d %d\n", tours[i][j], x[tours[i][j]], y[tours[i][j]], 
          x[tours[i][j+1]], y[tours[i][j+1]]);
      }
      fclose(f);

      printf("\nCost = %lf (road + %lf * delay)\n", roadTours[i] + (ALFA * i), ALFA);
      printf("Road = %lf\n", roadTours[i]);
      printf("Delay = %d\n", i);
    }
  }
}