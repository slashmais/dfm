#ifndef _VersionRBT_h_
#define _VersionRBT_h_

#define V_RELEASE 2
#define V_BUILD 0
#define V_TEST 0
#define N_2_S0(x) #x
#define N_2_S(x) N_2_S0(x)
#define V_VERSION "v " N_2_S(V_RELEASE) "." N_2_S(V_BUILD) "." N_2_S(V_TEST)


//=============================================================================
//history will be updated by the Versioning-macro..
// and can be accessed with something like:
//	{
//		std:: string s{"History\n"};
//		for (auto p:VersionHistory<>) { s+="    ["; s+=p.first; s+="] -> "; s+=p.second; s+="\n"; }
//		PromptOK(DeQtf(s.c_str()));
//	}

#include <map>

template<typename S=std::string> struct MVHIST
{
    std::map<S, S> m;
    MVHIST(const S &k, const S &v) { m[k]=v; }
    MVHIST<S>& operator()(const S &k, const S &v) { m[k]=v; return *this; }
    operator std::map<S, S>() { return m; }
};

template<typename S=std::string> std::map<S, S> VersionHistory=MVHIST<S>
("v0.0.1", "initial compile with versioning")
("v0.0.2", "fucking bug hunt")
("v0.0.3", "testing versioning")
("v0.0.4", "test secure delete, sorting using CanExpand()")
("v0.1.0", "first build with versioning")
("v1.0.0", "first release with versioning")
("v1.0.1", "tab-tip")
("v2.0.0", "with processes")
//HISTORY (insertion-point, leave this comment in place - see macro)
;


#endif
