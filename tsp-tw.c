#include <stdio.h>
#include <math.h>
#include <string.h>

#define ALFA 0.5
#define MAX_STATES 1000000
#define MAX_TIME 5000
#define MAX_CUSTOMERS 50 // customers + origin
#define MAX_DELTA  10
#define MAX_ASCENT_LOOP 400

int N = 0; // customers ONLY

double lamda[MAX_CUSTOMERS];

int x[MAX_CUSTOMERS];
int y[MAX_CUSTOMERS];

int earlyTime[MAX_CUSTOMERS];
int lateTime[MAX_CUSTOMERS];

double c[MAX_CUSTOMERS][MAX_CUSTOMERS];
double _c[MAX_CUSTOMERS][MAX_CUSTOMERS]; // lagrangian ascend weights
int t[MAX_CUSTOMERS][MAX_CUSTOMERS];

int head[MAX_CUSTOMERS*MAX_CUSTOMERS];
int stateTime[MAX_STATES];
double F[MAX_STATES];
double G[MAX_STATES];
double realF[MAX_STATES];
double realG[MAX_STATES];
int Pi[MAX_STATES];
int Gamma[MAX_STATES];
int currentCustomer[MAX_STATES];

double f[MAX_CUSTOMERS][MAX_TIME];
double g[MAX_CUSTOMERS][MAX_TIME];
double realf[MAX_CUSTOMERS][MAX_TIME];
double realg[MAX_CUSTOMERS][MAX_TIME];
int fPi[MAX_CUSTOMERS][MAX_TIME];
int gGamma[MAX_CUSTOMERS][MAX_TIME];
int fCurrentCustomer[MAX_CUSTOMERS][MAX_TIME];

void readInput(char fileName[]) {
  FILE* file = fopen (fileName, "r");
  int service[MAX_CUSTOMERS];
  int tmp = 0;

  fscanf (file, "%d", &N);
  N--; // only customers number in N!!!
  printf ("customers number = %d\n", N);
  for (int i = 0; i < N+1; i++)
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
  for (int j = 0; j < N+1; j++) {
    for (int u = 0; u < N+1; u++) {
      dist = sqrt(pow(x[j] - x[u], 2) + pow(y[j] - y[u], 2));
      c[j][u] = dist;
      _c[j][u] = dist;
      t[j][u] = dist + service[u];
    }
  }
}

int getHead(int k, int i) {
  if (k == N+1) {
    return head[(N*N) + i];
  } else if (k < N+1 && i == N+1) {
    int index = ((k-1)*N) + (i-2);
    while (head[index] != 0) {
      index++;
    }
    return head[index - 1] + 1;
  } else {
    return head[((k-1)*N) + (i-1)];
  }
}

void setHead(int k, int i, int pointer) {
  if (k == N+1) {
    head[(N*N) + i] = pointer;
  } else {
    head[((k-1)*N) + (i-1)] = pointer;
  }
}

double delayCost(int customer, int arrivalTime) {
  if(lateTime[customer] >= arrivalTime) {
    return 0.0;
  } else {
    return (arrivalTime - lateTime[customer]) * ALFA;
  }
}

void checkDominance() {
  for (int i = 0; i < N+1; i++) {
    double fMin = INFINITY;
    for (int j = 0; j < MAX_TIME; j++) {
      if (f[i][j] != INFINITY) {
        if (f[i][j] < fMin) {
          fMin = f[i][j];
        } else {
          f[i][j] = INFINITY;
        }
      }
    }
  }
}

void resetFG() {
  for (int i = 0; i < N+1; i++) {
    for (int j = 0; j < MAX_TIME; j++) {
      f[i][j] = INFINITY;
      g[i][j] = INFINITY;
      realf[i][j] = INFINITY;
      realg[i][j] = INFINITY;
      fPi[i][j] = -1;
      gGamma[i][j] = -1;
    }
  }
}

void resetHead() {
  for (int i = 0; i < MAX_CUSTOMERS*MAX_CUSTOMERS; i++) {
    head[i] = 0;
  }
}

int intMax(int a, int b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

double doubleMax(double a, double b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

void printPath(int state, int g) {
  if (state < N) {
    printf("0 - %d", currentCustomer[state]);
  } else {
    int previous = g == 0 ? Pi[state] : Gamma[state];
    if (currentCustomer[Pi[previous]] == currentCustomer[state]) {
      printPath(previous, 1);
    } else {
      printPath(previous, 0);
    }
    printf(" - %d", currentCustomer[state]);
  }
}

void getPathFromState(int state, int path[], int pathLength, int g) {
  path[pathLength-1] = currentCustomer[state];
  if (currentCustomer[state] != 0) {
    int previous = g == 0 ? Pi[state] : Gamma[state];
    if (currentCustomer[Pi[previous]] == currentCustomer[state]) {
      getPathFromState(previous, path, pathLength-1, 1);
    } else {
      getPathFromState(previous, path, pathLength-1, 0);
    }
  }
}

void printStates(int k) {
  if (k < N + 1) {
    for (int i = getHead(k, 1); i < getHead(k+1, k == N ? 0 : 1); i++) {
      printf("%d\n", i);
      printf("F = %lf, T = %d, Pi = %d, G = %lf, Gamma = %d\n", realF[i], stateTime[i], Pi[i], G[i], Gamma[i]);
      printPath(i, 0);
      printf("\n");
    }
  } else {
    for (int i = getHead(k, 0); i < getHead(k, 1); i++) {
      printf("%d\n", i);
      printf("F = %lf, T = %d, realF = %lf, Pi = %d, G = %lf, Gamma = %d\n", F[i], stateTime[i], realF[i], Pi[i], G[i], Gamma[i]);
      printPath(i, 0);
      printf("\n");
    }
  }
}

void dynamicProgrammingTSP() {
  resetHead();
  int z = 0;
  for (int j = 1; j <= N; j++) {
    int time = intMax(earlyTime[j], t[0][j]);
    stateTime[z] = time;
    F[z] = _c[0][j] + delayCost(j, time);
    G[z] = INFINITY;
    realF[z] = c[0][j] + delayCost(j, time);
    realG[z] = INFINITY;
    Pi[z] = -1;
    currentCustomer[z] = j;
    Gamma[z] = -1;
    setHead(1, j, z);
    z++;
  }

  for(int k = 2; k <= N; k++) {
    resetFG();
    for (int i = 1; i <= N; i++) { /* generate new MAX_STATES for every final destination */
      for (int j = 1; j <= N; j++) { /* for every final destination in previous MAX_STATES */
        if (j != i) {
          for (int h = getHead(k-1, j); h < getHead(k-1, j+1); h++) { /* h is the previous state considered now */
            int time = intMax(earlyTime[i], stateTime[h] + t[j][i]);
            double cost;
            double realCost;
            if (i != currentCustomer[Pi[h]]) {
              cost = F[h] + _c[j][i] + delayCost(i, time);
              realCost = realF[h] + c[j][i] + delayCost(i, time);
            } else {
              cost = G[h] + _c[j][i] + delayCost(i, time);
              realCost = realG[h] + c[j][i] + delayCost(i, time);
            }

            if (cost <= f[i][time] && cost != INFINITY) {
              g[i][time] = f[i][time];
              realg[i][time] = realf[i][time];
              gGamma[i][time] = fPi[i][time];
              f[i][time] = cost;
              realf[i][time] = realCost;
              fPi[i][time] = h;
              fCurrentCustomer[i][time] = i;
            }
          }
        }
      }
    }
    checkDominance();
    /* deploy MAX_STATES */
    for (int p = 1; p <= N; p++) { // p = last customer.
      setHead(k, p, z);
      for (int l = 0; l < MAX_TIME; l++) { // l = time
        if (f[p][l] != INFINITY) {
          stateTime[z] = l;
          F[z] = f[p][l];
          G[z] = g[p][l];
          realF[z] = realf[p][l];
          realG[z] = realg[p][l];
          Pi[z] = fPi[p][l];
          currentCustomer[z] = fCurrentCustomer[p][l];
          Gamma[z] = gGamma[p][l];
          z++;
        }
      }
    }
  }

  int k = N + 1;
  int i = 0;
  resetFG();
  for (int j = 1; j <= N; j++) { /* for every final destination in previous MAX_STATES */
    for (int h = getHead(k-1, j); h < getHead(k-1, j+1); h++) { /* h is the previous state considered now */
      int time = intMax(earlyTime[i], stateTime[h] + t[j][i]);
      double cost;
      double realCost;
      if (i != currentCustomer[Pi[h]]) {
        cost = F[h] + _c[j][i] + delayCost(i, time);
        realCost = realF[h] + c[j][i] + delayCost(i, time);
      } else {
        cost = G[h] + _c[j][i] + delayCost(i, time);
        realCost = realG[h] + c[j][i] + delayCost(i, time);
      }

      if (cost <= f[i][time] && cost != INFINITY) {
        g[i][time] = f[i][time];
        realg[i][time] = realf[i][time];
        gGamma[i][time] = fPi[i][time];
        f[i][time] = cost;
        realf[i][time] = realCost;
        fPi[i][time] = h;
        fCurrentCustomer[i][time] = i;
      }
    }
  }
  checkDominance();
  /* deploy MAX_STATES */
  setHead(k, i, z);
  for (int l = 0; l < MAX_TIME; l++) { // l = time
    if (f[i][l] != INFINITY) {
      stateTime[z] = l;
      F[z] = f[i][l];
      G[z] = g[i][l];
      realF[z] = realf[i][l];
      realG[z] = realg[i][l];
      Pi[z] = fPi[i][l];
      currentCustomer[z] = fCurrentCustomer[i][l];
      Gamma[z] = gGamma[i][l];
      z++;
    }
  }
  setHead(N+1, 1, z);
}

int main(int argc, char *argv[]) {
  readInput(argv[1]);
  int bestPath[N+1];
  int occurrence[N+1];
  double alpha = 2;
  int lbState = 0;
  double lbValue = INFINITY;
  int w = 0;
  int delta = 0;
  double LB = -INFINITY;
  int isPathComplete = 0;

  while (w <= MAX_ASCENT_LOOP && !isPathComplete) {
    for (int i = 0; i < N+1; i++) { // set lamdas to 0.
      lamda[i] = 0;
      occurrence[i] = 0;
    }
    dynamicProgrammingTSP();
    lbState = 0;
    lbValue = INFINITY;
    for (int h = getHead(N+1, 0); h < getHead(N+1, 1); h++) { // select the best path
      if (F[h] < lbValue) {
        lbValue = F[h];
        lbState = h;
      }
    }
    if (lbValue > LB) {
      LB = lbValue;
      delta = 0;
    } else {
      delta++;
      if (delta >= MAX_DELTA) {
        delta = 0;
        alpha = alpha * 0.75;
      }
    }
    if (w % 10 == 0) {
      printf("buond w = %d, bound = %lf\n", w, LB);
    }
    getPathFromState(Pi[lbState], bestPath, N+1, 0); // get the best path in an array

    if (w == 1 || w == MAX_ASCENT_LOOP) {
      char filename[30] = "results/dp";
      if (w == 1){
        strcat(filename, "Init");
        FILE *f = fopen("results/points", "ab+");
        for (int j = 0; j < N+1; j++) {
          fprintf(f, "%d %d %d\n", j, x[j], y[j]);
        }
        fclose(f);
      } else {
        strcat(filename, "End");
      }
      FILE *f = fopen(filename, "ab+");
      for (int j = 0; j < N; j++) {
        fprintf(f, "%d %d %d %d %d\n", bestPath[j], x[bestPath[j]], y[bestPath[j]], 
          x[bestPath[j+1]], y[bestPath[j+1]]);
      }
      fprintf(f, "%d %d %d %d %d\n", bestPath[N], x[bestPath[N]], y[bestPath[N]], 
          x[0], y[0]);
      fclose(f);
    }
    
    for (int i = 0; i < N+1; i++) {
      occurrence[bestPath[i]]++;
    }
    // now calculate lamdas;
    isPathComplete = 1;
    for (int i = 0; i < N+1; i++) {
      if (occurrence[i] != 1) {
        isPathComplete = 0;
        break;
      }
    }
    if (!isPathComplete) {
      int occSum = 0;
      for (int i = 0; i < N+1; i++) {
        occSum += pow(occurrence[i] - 1, 2);
      }
      for (int i = 0; i < N+1; i++) {
        lamda[i] = lamda[i] - alpha * ((0.2*lbValue)/occSum) * (occurrence[i] - 1);
      }
      //now refresh c'_ij
      for (int i = 0; i < N+1; i++) {
        for(int j = 0; j < N+1; j++) {
          _c[i][j] = c[i][j] -(0.5*lamda[i]) -(0.5*lamda[j]);
        }
      }
    } else {
      printf("*** Complete! state = ", lbState);
    }
    w++;
  }
  printf("\n");
  //printStates(N+1);
}