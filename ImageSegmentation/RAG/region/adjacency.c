/*
 * adjacency.c
 *
 * compute adjacency representation of region pair
 * (based on contour pixels)
 *
 * Sven Wachsmuth, 27.11.2002
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "adjacency.h"

#define SECTION_PAIR_BUF (16)
#define _STRCAT(s,t,n,max) \
(strcat(((n+=strlen(t)) >= max) ? s=realloc(s, max=n+1) : s, t))

contour_section_t *contour_section_init(contour_section_t *sec)
{
  if (!sec)
    sec = (contour_section_t *) malloc(sizeof(contour_section_t));
  memset(sec, 0, sizeof(contour_section_t));

  sec->start = -1;
  sec->end   = -1;

  return (sec);
}

contour_section_t *contour_section_create(void)
{
  return contour_section_init(NULL);
}

void contour_section_free(contour_section_t *sec)
{
  return;
}

void contour_section_destroy(contour_section_t *sec)
{
  if (!sec) return;
  contour_section_free(sec);
  free(sec);
}

contour_section_t *contour_section_dup(contour_section_t *sec)
{
  return contour_section_cpy(NULL,sec);
}

contour_section_t *contour_section_cpy(contour_section_t *copy, 
				       contour_section_t *sec)
{
  if (!sec) return (copy);
  if (!copy)
    copy = contour_section_create();
  
  memcpy(copy, sec, sizeof(contour_section_t));
  return (copy);
}

/* -------------------------------------------------------- */

contour_sectionPair_t *contour_sectionPair_init(contour_sectionPair_t *sec)
{
  if (!sec)
    sec = (contour_sectionPair_t *) malloc(sizeof(contour_sectionPair_t));
  contour_section_init(&sec->sec1);
  contour_section_init(&sec->sec2);

  return (sec);
}

contour_sectionPair_t *contour_sectionPair_create(void)
{
  return contour_sectionPair_init(NULL);
}

void contour_sectionPair_free(contour_sectionPair_t *sec)
{
  return;
}

void contour_sectionPair_destroy(contour_sectionPair_t *sec)
{
  if (!sec) return;
  contour_sectionPair_free(sec);
  free(sec);
}

contour_sectionPair_t *contour_sectionPair_dup(contour_sectionPair_t *sec)
{
  return contour_sectionPair_cpy(NULL,sec);
}

contour_sectionPair_t *contour_sectionPair_cpy(contour_sectionPair_t *copy, 
				       contour_sectionPair_t *sec)
{
  if (!sec) return (copy);
  if (!copy)
    copy = contour_sectionPair_create();
  
  memcpy(copy, sec, sizeof(contour_sectionPair_t));
  return (copy);
}

contour_sectionPair_t *contour_sectionPair_update(contour_sectionPair_t *sec,
						  region_info_t *reg1,
						  region_info_t *reg2,
						  int *indexMap, int cols)
{
  int x,y,len;
  contour_point_t *p,*q;

  if (!sec || !indexMap) return (sec);

  x = sec->sec1.checked_start_point.x;
  y = sec->sec1.checked_start_point.y;
  p = &reg1->contour[sec->sec1.start];
  q = &reg1->contour[((sec->sec1.start > 0) ? 
		      sec->sec1.start-1 : reg1->n_contour-1)];
  /* sec->sec2.end = indexMap[cols * y + x]-1; */
  sec->sec2.end = region_indexMapN(indexMap,x,y,p->x,p->y,q->x,q->y,cols)-1;

  x = sec->sec1.checked_end_point.x;
  y = sec->sec1.checked_end_point.y;
  p = &reg1->contour[sec->sec1.end];
  q = &reg1->contour[((sec->sec1.end > 0) ? 
		      sec->sec1.end-1 : reg1->n_contour-1)];
  /* sec->sec2.start = indexMap[cols * y + x]-1; */
  sec->sec2.start = region_indexMapN(indexMap,x,y,p->x,p->y,q->x,q->y,cols)-1;

  if (sec->sec2.start < 0 || sec->sec2.end < 0)
    sec->sec2.length = 0;
  else {
    len = sec->sec2.end - sec->sec2.start;
    if (len < 0) {
      len += reg2->n_contour;
    }
    sec->sec2.length = len+1;
  }
  return (sec);
}

contour_sectionPair_t *contour_sectionPair_reverse(contour_sectionPair_t *rev,
						   contour_sectionPair_t *sec)
{
  if (!sec || !rev) return (rev);

  if (sec->sec2.start >= 0 && sec->sec2.end >= 0) {
    rev->sec1 = sec->sec2;
    rev->sec2 = sec->sec1;
  }
  return (rev);
}

/* ------------------------------------------------------------ */

region_adjacency_t *region_adjacency_init(region_adjacency_t *adj)
{
  if (!adj)
    adj = (region_adjacency_t *) malloc(sizeof(region_adjacency_t));
  memset(adj, 0, sizeof(region_adjacency_t));
  return (adj);
}

region_adjacency_t *region_adjacency_create(void)
{
  return (region_adjacency_init(NULL));
}

void region_adjacency_free(region_adjacency_t *adj)
{
  if (!adj) return;
  if (adj->sections && adj->max_sections > 0) {
    free(adj->sections);
  }
}

void region_adjacency_destroy(region_adjacency_t *adj)
{
  if (!adj) return;
  region_adjacency_free(adj);
  free(adj);
}

region_adjacency_t *region_adjacency_dup(region_adjacency_t *adj)
{
  return (region_adjacency_cpy(NULL,adj));
}

region_adjacency_t *region_adjacency_cpy(region_adjacency_t *copy, 
					 region_adjacency_t *adj)
{
  if (!adj) return (copy);
  if (!copy)
    copy = region_adjacency_create();

  memcpy(copy,adj,sizeof(region_adjacency_t));
  if (copy->max_sections > 0) {
    copy->sections = (contour_sectionPair_t *)
      malloc((copy->max_sections=copy->n_sections) *
	     sizeof(contour_sectionPair_t));
    memcpy(copy->sections, adj->sections,
	   copy->n_sections * sizeof(contour_sectionPair_t));
  }
  return (copy);
}

int region_adjacency_startSection(region_adjacency_t *adj,
				  int index,
				  int checked_x,
				  int checked_y)
{
  int i_section;
  if (!adj) return (-1);

  i_section = adj->n_sections++;
  if (i_section >= adj->max_sections) {
    adj->sections = (contour_sectionPair_t *)
      realloc(adj->sections, (adj->max_sections+=SECTION_PAIR_BUF) *
	      sizeof(contour_sectionPair_t));
  }
  adj->sections[i_section].sec1.start = index;
  adj->sections[i_section].sec1.checked_start_point.x = checked_x;
  adj->sections[i_section].sec1.checked_start_point.y = checked_y;
  
  return (i_section);
}

region_adjacency_t *region_adjacency_endSection(region_adjacency_t *adj,
						int index,
						int checked_x,
						int checked_y)
{
  int i_section, len;
  if (!adj) return (NULL);

  i_section = adj->n_sections - 1;
  adj->sections[i_section].sec1.end = index;

  len = adj->sections[i_section].sec1.end-adj->sections[i_section].sec1.start;
  if (len < 0) {
    len += adj->region1->n_contour;
  }
  adj->sections[i_section].sec1.length = len+1;

  adj->sections[i_section].sec1.checked_end_point.x = checked_x;
  adj->sections[i_section].sec1.checked_end_point.y = checked_y;

  return (adj);
}

region_adjacency_t **region_adjacency_set(region_adjacency_t **adj,
					  region_info_t **reg,
					  int i_region, int n_regions,
					  int *regionMap, int cols)
{
  /* mapping from offset -> state_index
   */
  static int direction[3][3] = { { 1, 4, 4 },
				 { 1, 0, 3 },
				 { 2, 2, 3 } };
  
  /* mapping from direction -> offset: 1 2 3
   *                                   8 0 4
   *                                   7 6 5 
   */
  static int offset[9][2] = { {0,0}, {-1,-1}, { 0,-1}, { 1,-1},
			      {1,0}, { 1, 1}, { 0, 1}, {-1,-1}, { -1,0} };

  /* mapping from direction x direction -> state_number 
   */
  static int number[5][5] = { { 4, 0, 0, 0, 0 },
			      { 0, 1, 2, 3, 0 },
			      { 0, 0, 1, 2, 3 },
			      { 0, 3, 0, 1, 2 },
			      { 0, 2, 3, 0, 1 } };

  /* mapping from direction x number -> test_direction
   * where number = [1..state_number]
   */
  static int test_direction[5][4] = { { 2, 4, 6, 8 },
				      { 2, 4, 6, 0 },
				      { 8, 2, 4, 0 },
				      { 6, 8, 2, 0 },
				      { 4, 6, 8, 0 } };
			       
  int i,n,k,l;
  int act_x, act_y;
  int last_direction;
  int last_label = -1;
  int xx = -1, yy = -1;

  if (!adj || !reg || !reg[i_region]) return (adj);

  /*
  for (i=0; i < n_regions; i++)
    if (adj[i] && reg[i]) {
      region_adjacency_init(adj[i]);
      adj[i]->region1 = reg[i_region];
      adj[i]->region2 = reg[i];
    }
  */
  n = reg[i_region]->n_contour;
  if (n < 1) return (adj);

  act_x = reg[i_region]->contour[n-1].x;
  act_y = reg[i_region]->contour[n-1].y;
  if (n >= 2) {
    int last_x, last_y;
    int last_dx, last_dy;
    last_x = reg[i_region]->contour[n-2].x;
    last_y = reg[i_region]->contour[n-2].y;
    last_dx = act_x - last_x + 1;
    last_dy = act_y - last_y + 1;
    last_direction = direction[last_dy][last_dx];
  } else {
    /*fprintf(stderr,"adjacency: one pixel region.\n");*/
    last_direction = 0;
  }
  for (l=n-2,k=n-1,i=0; i < n; i++) {
    int next_x = reg[i_region]->contour[i].x;
    int next_y = reg[i_region]->contour[i].y;
    int next_dx = next_x - act_x + 1;
    int next_dy = next_y - act_y + 1;
    int next_direction = direction[next_dy][next_dx];
    int state_number = number[last_direction][next_direction];
    int j;
    /*fprintf(stderr,"%d (%d,%d) [%d,%d]:",i,act_x,act_y,
     *    last_direction, next_direction); */
    for (j=state_number-1; j >= 0; j--) {
      int x = act_x + offset[test_direction[next_direction][j]][0];
      int y = act_y + offset[test_direction[next_direction][j]][1];
      int label = regionMap[y * cols + x];
      /* fprintf(stderr," (%d,%d:%d)",x,y,label); 
       */
      if (label != last_label) {
	if (last_label >= 0) 
	  region_adjacency_endSection(adj[last_label],l,xx,yy);
	if (label >= 0)
	  region_adjacency_startSection(adj[label],k,x,y);
      }
      last_label = label;
      /* the last pixel l must change to the actual pixel,
       * because you cannot have two sections in parallel that have
       * non-zero length */
      l = k; /* CORRECTION: the actual pixel must not change: k = i; */ 
      xx = x; yy = y;
    }
    /*fprintf(stderr,"\n");*/
    /* CORRECTION: l and k must proceed if state_number == 0 */
    l = k; k = i;
    last_direction = next_direction;
    act_x = next_x;
    act_y = next_y;
  }
  if (last_label >= 0)
    region_adjacency_endSection(adj[last_label],l,xx,yy);

  return (adj);
}

region_adjacency_t *region_adjacency_setOLD(region_adjacency_t *adj,
					    region_info_t *reg,
					    int i_region, int n_regions,
					    int *regionMap, int cols)
{
  /* mapping from offset -> state_index
   */
  static int direction[3][3] = { { 1, 4, 4 },
				 { 1, 0, 3 },
				 { 2, 2, 3 } };
  
  /* mapping from direction -> offset: 1 2 3
   *                                   8 0 4
   *                                   7 6 5 
   */
  static int offset[9][2] = { {0,0}, {-1,-1}, { 0,-1}, { 1,-1},
			      {1,0}, { 1, 1}, { 0, 1}, {-1,-1}, { -1,0} };

  /* mapping from direction x direction -> state_number 
   */
  static int number[5][5] = { { 4, 0, 0, 0, 0 },
			      { 0, 1, 2, 3, 0 },
			      { 0, 0, 1, 2, 3 },
			      { 0, 3, 0, 1, 2 },
			      { 0, 2, 3, 0, 1 } };

  /* mapping from direction x number -> test_direction
   * where number = [1..state_number]
   */
  static int test_direction[5][4] = { { 2, 4, 6, 8 },
				      { 2, 4, 6, 0 },
				      { 8, 2, 4, 0 },
				      { 6, 8, 2, 0 },
				      { 4, 6, 8, 0 } };
			       
  int i,n,k,l;
  int act_x, act_y;
  int last_direction;
  int last_label = -1;
  int xx = -1, yy = -1;

  if (!adj) {
    adj = (region_adjacency_t *) 
      malloc(n_regions * sizeof(region_adjacency_t));
  }
  for (i=0; i < n_regions; i++) {
    region_adjacency_init(&adj[i]);
    adj[i].region1 = &reg[i_region];
    adj[i].region2 = &reg[i];
  }
  n = reg[i_region].n_contour;
  if (n < 1) return (adj);

  act_x = reg[i_region].contour[n-1].x;
  act_y = reg[i_region].contour[n-1].y;
  if (n >= 2) {
    int last_x, last_y;
    int last_dx, last_dy;
    last_x = reg[i_region].contour[n-2].x;
    last_y = reg[i_region].contour[n-2].y;
    last_dx = act_x - last_x + 1;
    last_dy = act_y - last_y + 1;
    last_direction = direction[last_dy][last_dx];
  } else {
    /*fprintf(stderr,"adjacency: one pixel region.\n");*/
    last_direction = 0;
  }
  for (l=n-2,k=n-1,i=0; i < n; i++) {
    int next_x = reg[i_region].contour[i].x;
    int next_y = reg[i_region].contour[i].y;
    int next_dx = next_x - act_x + 1;
    int next_dy = next_y - act_y + 1;
    int next_direction = direction[next_dy][next_dx];
    int state_number = number[last_direction][next_direction];
    int j;
    /*fprintf(stderr,"%d (%d,%d) [%d,%d]:",i,act_x,act_y,
     *    last_direction, next_direction); */
    for (j=state_number-1; j >= 0; j--) {
      int x = act_x + offset[test_direction[next_direction][j]][0];
      int y = act_y + offset[test_direction[next_direction][j]][1];
      int label = regionMap[y * cols + x];
      /* fprintf(stderr," (%d,%d:%d)",x,y,label); 
       */
      if (label != last_label) {
	if (last_label >= 0) 
	  region_adjacency_endSection(&adj[last_label],l,xx,yy);
	if (label >= 0)
	  region_adjacency_startSection(&adj[label],k,x,y);
      }
      last_label = label;
      l = k; k = i;
      xx = x; yy = y;
    }
    /*fprintf(stderr,"\n");*/
    last_direction = next_direction;
    act_x = next_x;
    act_y = next_y;
  }
  if (last_label >= 0)
    region_adjacency_endSection(&adj[last_label],l,xx,yy);

  return (adj);
}
    
region_adjacency_t *region_adjacency_update(region_adjacency_t *adj,
					    int *indexMap, int cols)
{
  int i;
  if (!adj) return(NULL);
  
  for (i=0; i < adj->n_sections; i++) {
    contour_sectionPair_update(&adj->sections[i], adj->region1, adj->region2,
			       indexMap, cols);
  }

  region_adjacency_setValues(adj,NULL,0);

  return (adj);
}

region_adjacency_t *region_adjacency_revSection(region_adjacency_t *adj,
						contour_sectionPair_t *sec)
{
  int i_section;

  if (!adj) return (NULL);

  i_section = adj->n_sections++;
  if (i_section >= adj->max_sections) {
    adj->sections = (contour_sectionPair_t *)
      realloc(adj->sections, (adj->max_sections+=SECTION_PAIR_BUF) *
	      sizeof(contour_sectionPair_t));
  }
  contour_sectionPair_reverse(&adj->sections[i_section], sec);

  return (adj);
}

region_adjacency_t *region_adjacency_cpySection(region_adjacency_t *adj,
						contour_sectionPair_t *sec)
{
  int i_section;

  if (!adj) return (NULL);

  i_section = adj->n_sections++;
  if (i_section >= adj->max_sections) {
    adj->sections = (contour_sectionPair_t *)
      realloc(adj->sections, (adj->max_sections+=SECTION_PAIR_BUF) *
	      sizeof(contour_sectionPair_t));
  }
  adj->sections[i_section] = *sec;

  return (adj);
}

region_adjacency_t *region_adjacency_cpySec1(region_adjacency_t *adj,
					     contour_sectionPair_t *sec)
{
  int i_section;

  if (!adj) return (NULL);

  i_section = adj->n_sections++;
  if (i_section >= adj->max_sections) {
    adj->sections = (contour_sectionPair_t *)
      realloc(adj->sections, (adj->max_sections+=SECTION_PAIR_BUF) *
	      sizeof(contour_sectionPair_t));
  }
  adj->sections[i_section].sec1 = sec->sec1;
  contour_section_init(&adj->sections[i_section].sec2);
  
  return (adj);
}

int region_isAdjacent(region_adjacency_t *adj)
{
  if (!adj) return (0);

  return (adj->n_sections > 0);
}

int region_isBiAdjacent(region_adjacency_t *adj)
{
  int i;
  if (!adj) return (0);
  
  for (i=0; i < adj->n_sections; i++)
    if (adj->sections[i].sec2.start >= 0) return (1);

  return (0);
}

region_adjacency_t *region_adjacency_setValues(region_adjacency_t *adj,
					       region_info_t **regions,
					       int max_regions)
{
  int i;
  if (!adj) return (adj);

  if (regions) {
    adj->region1 = ((adj->region1_label>=0 || adj->region1_label<max_regions) ?
		    regions[adj->region1_label] : NULL);
    adj->region2 = ((adj->region1_label>=0 || adj->region1_label<max_regions) ?
		    regions[adj->region2_label] : NULL);
  }
  region_adjacency_setAValues(adj);
  return (adj);
}

region_adjacency_t *region_adjacency_setRValues(region_adjacency_t *adj,
						region_info_t *region1,
						region_info_t *region2)
{
  adj->region1 = region1;
  adj->region2 = region2;

  region_adjacency_setAValues(adj);
  return (adj);
}

region_adjacency_t *region_adjacency_setAValues(region_adjacency_t *adj)
{
  int i;
  if (!adj) return (adj);

  adj->contour_length1 = 0;
  adj->contour_length2 = 0;

  for (i=0; i < adj->n_sections; i++) {
    int len;
    if (adj->sections[i].sec1.start >= 0) {
      len = adj->sections[i].sec1.end - adj->sections[i].sec1.start;
      if (len < 0 && adj->region1) len += adj->region1->n_contour;
    } else
      len = -1;

    adj->contour_length1 += adj->sections[i].sec1.length = len + 1;

    if (adj->sections[i].sec2.start >= 0) {
      len = adj->sections[i].sec2.end - adj->sections[i].sec2.start;
      if (len < 0 && adj->region1) len += adj->region1->n_contour;
    } else
      len = -1;

    adj->contour_length2 += adj->sections[i].sec2.length = len + 1;
  }
  return (adj);
}

char *region_adjacency_sprint(char *s, region_adjacency_t *adj)
{
  char tmp[256];
  int  i,n,max;
  if (!adj) return (s);
  
  sprintf(tmp,"adjacency %d %d-%d: #%d-#%d %%%d-%%%d [#%d",
	  adj->label,
	  (adj->region1) ? adj->region1->label : -1,
	  (adj->region2) ? adj->region2->label : -1,
	  adj->contour_length1,
	  adj->contour_length2,
	  (adj->region1)?(adj->contour_length1*100)/adj->region1->n_contour:-1,
	  (adj->region2)?(adj->contour_length2*100)/adj->region2->n_contour:-1,
	  adj->n_sections);
  n = strlen(tmp);
  max = n + adj->n_sections * 20 + 8;
  strcpy(s = realloc(s, max),tmp);

  for (i=0; i < adj->n_sections; i++) {
    sprintf(tmp, " (%d..%d,%d..%d;%d,%d:%d,%d)",
	    adj->sections[i].sec1.start,
	    adj->sections[i].sec1.end,
	    adj->sections[i].sec2.start,
	    adj->sections[i].sec2.end,
	    adj->sections[i].sec1.checked_start_point.x,
	    adj->sections[i].sec1.checked_start_point.y,
	    adj->sections[i].sec1.checked_end_point.x,
	    adj->sections[i].sec1.checked_end_point.y);
    s = _STRCAT(s,tmp,n,max);
  }
  sprintf(tmp, "]");
  _STRCAT(s,tmp,n,max);
  
  return (s);
}

int region_adjacency_sscanR(region_adjacency_t *adj, char *s, 
			    region_info_t **regions, int max)
{
  if (!region_adjacency_sscan(adj,s))
    return (0);

  region_adjacency_setValues(adj, regions, max);

  return (1);
}
  
int region_adjacency_sscan(region_adjacency_t *adj, char *s)
{
  int i,j,x,y;
  int n_sections;
  if (!adj || !s) return (0);
  if ((i=sscanf(s,"adjacency %d %d-%d: #%d-#%d %%%d-%%%d [#%d",
		&adj->label,
		&adj->region1_label,
		&adj->region2_label,
		&adj->contour_length1,
		&adj->contour_length2,
		&x,&y,
		&n_sections))
       != 8) {
    fprintf(stderr,"region_adjacency_sscan: illegal format (%d/8 values read).\n",i);
    return (0);
  }
  s = strstr(s,"[#");

  if (adj->max_sections < n_sections) {
    if (adj->max_sections > 0) {
      adj->sections = (contour_sectionPair_t *)
	realloc(adj->sections, n_sections * sizeof(contour_sectionPair_t));
    } else {
      adj->sections = (contour_sectionPair_t *)
	malloc(n_sections * sizeof(contour_sectionPair_t));
    }
    adj->max_sections = n_sections;
  }
  for (j=0; strchr(++s,' ') && j < n_sections; j++) {
    s = strchr(s,' ');
    if (sscanf(s," (%d..%d,%d..%d;%d,%d:%d,%d)",
	       &adj->sections[j].sec1.start,
	       &adj->sections[j].sec1.end,
	       &adj->sections[j].sec2.start,
	       &adj->sections[j].sec2.end,
	       &adj->sections[j].sec1.checked_start_point.x,
	       &adj->sections[j].sec1.checked_start_point.y,
	       &adj->sections[j].sec1.checked_end_point.x,
	       &adj->sections[j].sec1.checked_end_point.y)
	!= 8) {
      fprintf(stderr,
	      "region_adjacency_sscan: illegal format (section[%d]).\n",
	      j);
      return (0);
    }
    adj->n_sections++;
  }
  return (1);
}

