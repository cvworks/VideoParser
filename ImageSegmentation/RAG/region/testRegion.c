#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "regioninfo.h"
#include "adjacency.h"

int main(int argc, char *argv[])
{
  char line[10000];
  char *s = NULL;
  int i,n_regions,n_adjacencies;
  region_info_t **regions = NULL;

  if (fgets(line,10000,stdin)) {
    sscanf(line,"%d %d", &n_regions, &n_adjacencies);
  }
  regions = (region_info_t **) malloc(n_regions * sizeof(void *));

  i = 1;
  while ((--n_regions) >= 0 && fgets(line,10000,stdin)) {
    region_info_t *region = region_info_create();
    region_info_sscan(region,line);
    
    s = region_info_sprint(s, region);
    printf("%s\n",s);
    regions[i++] = region;
  }
  while ((--n_adjacencies) >= 0 && fgets(line,10000,stdin)) {
    region_adjacency_t adj;
    region_adjacency_init(&adj);
    region_adjacency_sscanR(&adj,line,regions, i);
    
    s = region_adjacency_sprint(s, &adj);
    printf("%s\n",s);
    region_adjacency_free(&adj);
  }
  
  exit(0);
}
