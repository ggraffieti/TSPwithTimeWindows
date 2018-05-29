#include <stdio.h>
#include <math.h>

#define ALFA 0.5
#define STATES 10000
#define N 3 /* customers */
#define TIMES 600

int earlyTime[] = {0, 0, 10, 20};
int lateTime[] = {TIMES, 20, 35, 40};

double c[N+1][N+1] = {
  {INFINITY, 10, 14.142135, 10}, /* deposit to every customer */
  {10, INFINITY, 10, 14.142135}, 
  {14.142135, 10, INFINITY, 10},
  {10, 14.142135, 10, INFINITY}
};

int t[N+1][N+1] = {
  {0, 10, 14.142135, 10}, /* deposit to every customer */
  {10, 0, 10, 14.142135}, 
  {14.142135, 10, 0, 10},
  {10, 14.142135, 10, 0}
};

int head[(N*N)+1];
int stateTime[STATES];
double F[STATES];
double G[STATES];
int Pi[STATES];
int Gamma[STATES];
int currentCustomer[STATES];

double f[N+1][TIMES];
double g[N+1][TIMES];
int fPi[N+1][TIMES];
int gGamma[N+1][TIMES];
int fCurrentCustomer[N+1][TIMES];

int getHead(int k, int i) {
  if (k == N+1 && i == 0) {
    return head[N*N];
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
  if (k == N+1 && i == 0) {
    head[N*N] = pointer;
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
    for (int j = 0; j < TIMES; j++) {
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
    for (int j = 0; j < TIMES; j++) {
      f[i][j] = INFINITY;
      g[i][j] = INFINITY;
      fPi[i][j] = -1;
      gGamma[i][j] = -1;
    }
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

void printStates(int k) {
  for (int i = getHead(k, 1); i < getHead(k+1,1); i++) {
    printf("%d\n", i);
    printf("F = %lf, T = %d, Pi = %d, G = %lf, Gamma = %d\n", F[i], stateTime[i], Pi[i], G[i], Gamma[i]);
    printPath(i, 0);
    printf("\n");
  }
}

int main() {
  int z = 0;
  for (int j = 1; j <= N; j++) {
    int time = intMax(earlyTime[j], t[0][j]);
    stateTime[z] = time;
    F[z] = c[0][j] + delayCost(j, time);
    G[z] = INFINITY;
    Pi[z] = -1;
    currentCustomer[z] = j;
    Gamma[z] = -1;
    setHead(1, j, z);
    z++;
  }

  for(int k = 2; k <= N; k++) {
    resetFG();
    for (int i = 1; i <= N; i++) { /* generate new states for every final destination */
      for (int j = 1; j <= N; j++) { /* for every final destination in previous states */
        if (j != i) {
          for (int h = getHead(k-1, j); h < getHead(k-1, j+1); h++) { /* h is the previous state considered now */
            int time = intMax(earlyTime[i], stateTime[h] + t[j][i]);
            double cost;
            if (i != currentCustomer[Pi[h]]) {
              cost = F[h] + c[j][i] + delayCost(i, time);
            } else {
              cost = G[h] + c[j][i] + delayCost(i, time);
            }

            if (cost <= f[i][time] && cost != INFINITY) {
              g[i][time] = f[i][time];
              gGamma[i][time] = fPi[i][time];
              f[i][time] = cost;
              fPi[i][time] = h;
              fCurrentCustomer[i][time] = i;
            }
          }
        }
      }
    }
    checkDominance();
    /* deploy states */
    for (int p = 1; p <= N; p++) {
      setHead(k, p, z);
      for (int l = 0; l < TIMES; l++) {
        if (f[p][l] != INFINITY) {
          stateTime[z] = l;
          F[z] = f[p][l];
          G[z] = g[p][l];
          Pi[z] = fPi[p][l];
          currentCustomer[z] = fCurrentCustomer[p][l];
          Gamma[z] = gGamma[p][l];
          z++;
        }
      }
    }
  }
  setHead(4, 1, z);
  printStates(3);

}