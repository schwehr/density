// Kurt Schwehr - Sept 2004

 

#include <cmath>

#include <vector>
#include <iostream>

#include "kdsPmagL.H" // L is for local
#include "SiteSigma.H"

using namespace std;

//typedef vector<float> SVec ;

// Return site sigma using Hext method based on Tauxe s_hext.f
// For boot strapping with Parametric SITE mode
float SiteSigma (const vector<SVec> &s) {
  assert(0<s.size());
  //cout << "size: " << s.size() << endl;
  if (s.size()<2) { cerr << "Too small to bootstrap...  here comes a NaN" << endl;}

  float s0=0;
  vector<SVec> d; d.resize(s.size()); for (size_t i=0;i<d.size(); i++) d[i].resize(6);
  vector<float> avd; avd.resize(6); for (size_t i=0;i<6;i++) avd[i]=0.;

  for (size_t i=0;i<s.size();i++) {
    SVec curD = s[i];
    curD[4-1]=curD[4-1]+.5*(curD[1-1]+curD[2-1]);
    curD[5-1]=curD[5-1]+.5*(curD[2-1]+curD[3-1]);
    curD[6-1]=curD[6-1]+.5*(curD[1-1]+curD[3-1]);
    for (size_t j=0;j<6; j++) avd[j]+=curD[j];
    d[i]=curD;
  } // for i

  for (size_t j=0;j<6; j++) avd[j]/=s.size();
  for (size_t i=0;i<s.size();i++) {
    for (size_t j=0;j<6; j++) s0+=(d[i][j]-avd[j])*(d[i][j]-avd[j]);
  }

  const float nf( (s.size()-1) * 6.);
  const float sigma=sqrt(s0/nf);
  return sigma;
}

#ifdef REGRESSION_TEST
#include <iomanip>
#include <fstream>


static bool
isEqual (const float a, const float b, const float del) {
  return ( ( a<b+del && a > b-del) ? true : false );
}

bool testFile (const string &filename, const float expected_sigma)
{

  ifstream in(filename.c_str(),ios::in);
  if (!bool(in)) {cerr << "failed to open file: " << filename << endl; return false;}
  SVec tmp(6,0.);
  float tmpSigma;
  vector <SVec> s;
  while (in >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3] >> tmp[4] >> tmp[5] >> tmpSigma) {
    //{static int i=0; cout << i++ << ": ";}
    //for (size_t i=0;i<6;i++) cout << tmp[i] << " ";
    //cout << endl; // cout << sigma << endl;
    s.push_back(tmp);
  }
  const float sigma = SiteSigma(s);
  if (!isEqual(sigma,expected_sigma,0.0000001)) {
    cout << setprecision(8);
    cout << "For file " << filename << ": " << sigma << "!=" << expected_sigma << endl;
    FAILED_HERE;
    return false;
  }
  return true;
}

int main(UNUSED int argc, char *argv[]) {
  bool ok=true;
  cout << setprecision(8);

  // 2.s          Lisa's  sitesig =   0.000577592349
  // as1-crypts.s Lisa's sitesig =   0.00133387523
  if (!testFile("as1-crypt.s", 0.00133387523)) {ok=false;}

  cout << argv[0] << " :" << (ok?"ok":"FAILED") << endl;
  return (ok?EXIT_SUCCESS:EXIT_FAILURE);
}
#endif
