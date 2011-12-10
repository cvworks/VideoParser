#include "genrl.h"
#include <iostream>
#include <Tools/MathUtils.h>
#include <map>


void distseq(int N)
{
  std::multimap<double,bool> m,dm; int i,j; 

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      if (i>=j)
      {
         double d = sqrt(double(i * i + j * j));
         m.insert(std::multimap<double,bool>::value_type(d,true));
   
      }

  std::cout<<"SZ "<<m.size();

  std::multimap<double,bool>::iterator it = m.begin();
  double dp = (*it).first;

  for(it++;it!=m.end();it++)
  {
      double d    = (*it).first;
      double diff = fabs(dp-d);
      if (dm.find(diff)==dm.end())
         dm.insert(std::multimap<double,bool>::value_type(diff,true));
      dp = d;
  }

  int k=0;
  for(it=dm.begin();it!=dm.end() && k<10; it++,k++)
      std::cout<<"Smallest diffs: "<<(*it).first<<std::endl;

}
