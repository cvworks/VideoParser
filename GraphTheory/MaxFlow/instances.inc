#include "FlowGraph.h"

#ifdef _MSC_VER
#pragma warning(disable: 4661)
#endif

// Instantiations: <captype, tcaptype, flowtype>
// IMPORTANT: 
//    flowtype should be 'larger' than tcaptype 
//    tcaptype should be 'larger' than captype

template class FlowGraph<int,int,int>;
template class FlowGraph<short,int,int>;
template class FlowGraph<float,float,float>;
template class FlowGraph<double,double,double>;

