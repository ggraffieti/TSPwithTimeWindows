#include <stdio.h>
#include <math.h>

#define MAX_SIZE 50

int main(int argc, char *argv[]) {
  printf("%s\n\n", argv[1]);
  FILE* file = fopen (argv[1], "r");
  int tot;
  int x[MAX_SIZE];
  int y[MAX_SIZE];
  int a[MAX_SIZE];
  int b[MAX_SIZE]; 
  int service[MAX_SIZE];
  int tmp = 0;

  fscanf (file, "%d", &tot);  
  printf ("tot = %d\n", tot);
  for (int i = 0; i < tot; i++)
  { 
    fscanf (file, "%d", &tmp);
    fscanf (file, "%d", &x[i]);
    fscanf (file, "%d", &y[i]);
    fscanf (file, "%d", &tmp);
    fscanf (file, "%d", &a[i]);
    fscanf (file, "%d", &b[i]);
    fscanf (file, "%d", &service[i]);
    printf("x = %d, y = %d, a = %d, b = %d, service = %d\n", x[i], y[i], a[i], b[i], service[i]);

  }
  fclose (file);    

  printf("\n\n** DISTANCES\n\n");
  int dist = 0;
  int time = 0;
  for (int u = 1; u < tot; u++) {
    printf("*distance 0 - %d\n", u);
    dist = sqrt(pow(x[0] - x[u], 2) + pow(y[0] - y[u], 2));
    printf("dist = %d, time = %d\n", dist, dist + service[u]);
  }


}