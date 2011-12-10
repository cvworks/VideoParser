/*
 * RAGraph/Adjacency.C
 *
 * C++ class for RAG edges
 *
 * Sven Wachsmuth, 28.11.2002
 * Sven Wachsmuth, 1.5.2003 (totally re-organized)
 */
#include "Adjacency.h"

using namespace std;

/*static istream& _drop(istream& s, char *pattern)
{
  char c;
  int n = (pattern) ? strlen(pattern) : 0;
  int i;
  for (i=0; i<n; i++)  s >> c;
  return (s);
};*/


RAG_RefAdjacency& RAG_RefAdjacency::init(void)
{
  rag_info = NULL;
  label    = -1;
  info_string = "";
  total_pixelLength1 = 0;
  total_pixelLength2 = 0;
  contour1_pixelLength = 0;
  contour1_pixelLength = 0;

  return (*this);
}

RAG_RefAdjacency& RAG_RefAdjacency::initValues(void)
{
  region_adjacency_t *adj = (region_adjacency_t *) get();

  if (!adj) return (*this);

  contour1_pixelLength = (adj->region1) ? adj->region1->n_contour : 0;
  contour2_pixelLength = (adj->region2) ? adj->region2->n_contour : 0;
  total_pixelLength1 = 0;
  total_pixelLength2 = 0;
  int i;
  for (i=0; i < adj->n_sections; i++) {
    total_pixelLength1 += adj->sections[i].sec1.length;
    total_pixelLength2 += adj->sections[i].sec2.length;
  }
  setInfoString();

  return (*this);
}

const leda_string& RAG_RefAdjacency::setInfoString()
{
  leda_string *_info_string = newInfoString();
  info_string = *_info_string;
  delete _info_string;
  return (info_string);
}

leda_string *RAG_RefAdjacency::newInfoString(void) const
{
  leda_string *_string;
  region_adjacency_t *adj = (region_adjacency_t *) get();

  if (adj) {
    _string 
      = new leda_string("adjacency %d %d-%d: #%d-#%d %%%d-%%%d [#%d", 
			adj->label,
			(adj->region1) ? adj->region1->label : -1,
			(adj->region2) ? adj->region2->label : -1,
			total_pixelLength1, total_pixelLength2,
			(contour1_pixelLength > 0) ?
			(total_pixelLength1 * 100) / contour1_pixelLength : -1,
			(contour2_pixelLength > 0) ?
			(total_pixelLength2 * 100) / contour2_pixelLength : -1,
			adj->n_sections);
    int i;
    for (i=0; i < adj->n_sections; i++) {
      leda_string info_section(" (%d..%d,%d..%d)",
			       adj->sections[i].sec1.start,
			       adj->sections[i].sec1.end,
			       adj->sections[i].sec2.start,
			       adj->sections[i].sec2.end);
      
      *_string += info_section;
    }
    *_string += "]";
  } else {
    _string = new leda_string();
  }
  return (_string);
}

ostream& operator<<(ostream& s, const RAG_RefAdjacency& adj)
{ 
  char *str = NULL;
  str = region_adjacency_sprint(str,(region_adjacency_t *) adj.get());
  s << str;
  free(str);
  //s << adj.getInfoString();
  return (s);
}

istream& operator>>(istream& s, RAG_Adjacency& adj)
{ 
  //int x;
  if (!adj.info)
    adj.info = region_adjacency_create();
  else {
    region_adjacency_free(adj.info);
    region_adjacency_init(adj.info);
  }
  leda_string str;
  const char *_str;
  str.read(s,'}');
  region_adjacency_sscan(adj.info,(char *) (_str = str));

  /***********
  _drop(s,"adjacency") >> adj.info->label >> x;
  adj.refLabel() = adj.info->label;
  _drop(s,"-") >> x;
  _drop(s,":#") >> adj.refPixelLength1();
  _drop(s,"-#") >> adj.refPixelLength2();
  _drop(s,"%") >> x;
  _drop(s,"-%") >> x;
  _drop(s,"[#") >> adj.info->n_sections;

  if (adj.info->max_sections < adj.info->n_sections) {
    if (adj.info->max_sections > 0)
      adj.info->sections = (contour_sectionPair_t *)
	realloc(adj.info->sections, 
		adj.info->n_sections * sizeof(contour_sectionPair_t));
    else
      adj.info->sections = (contour_sectionPair_t *)
	malloc(adj.info->n_sections * sizeof(contour_sectionPair_t));
    adj.info->max_sections = adj.info->n_sections;
  }
  int i=0;
  char c;
  s >> c;
  while (c != ']') {
    contour_sectionPair_t sec;
    contour_sectionPair_init(&sec);
    s >> sec.sec1.start;
    _drop(s,"..") >> sec.sec1.end;
    _drop(s,",") >> sec.sec2.start;
    _drop(s,"..") >> sec.sec2.end;
    _drop(s,")") >> c;
    contour_sectionPair_cpy(&adj.info->sections[i++], &sec);
  }
  *******/

  adj.setInfoString();

  return (s);
}

RAG_RefAdjacency& 
RAG_RefAdjacency::initRegions(region_info_t *reg1, region_info_t *reg2)
{
  region_adjacency_t *adj = (region_adjacency_t *) get();

  region_adjacency_setRValues(adj,reg1,reg2);

  if (reg1)
    contour1_pixelLength = reg1->n_contour;
  if (reg2)
    contour2_pixelLength = reg2->n_contour;

  return (*this);
}

RAG_Adjacency& RAG_Adjacency::set(const RAG_Adjacency& adj)
{
  (RAG_RefAdjacency&) (*this) = (RAG_RefAdjacency&) adj;

  info = region_adjacency_cpy(info,adj.info);

  return (*this);
}
  

  

