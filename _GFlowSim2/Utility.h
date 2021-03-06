//
//  Utility.h
//  Predictive1
//
//  Created by Nathaniel Rupprecht on 1/25/16.
//  Copyright (c) 2016 Nathaniel Rupprecht. All rights reserved.
//

#ifndef Predictive1_Utility_h
#define Predictive1_Utility_h

#include "DefaultConstants.h"

#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <list>
#include <math.h>
#include <omp.h>
#include <ostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <tuple>
#include <chrono>

#include <sys/stat.h> // For linux

using std::vector;
using std::pair;
using std::list;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::time;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

const double PI = 3.14159265;

static std::mt19937 generator;
static std::normal_distribution<double> normal_dist(0., 1.);
static std::poisson_distribution<int> poisson_dist();

/// Random number function
inline double getRand() { 
  return drand48(); 
}

inline void seedRandomGenerator() {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator = std::mt19937(seed);
}

/// Random number, normal distribution
inline double randNormal() {
  return normal_dist(generator);
}

/// Get the current time
inline auto current_time() {
  return high_resolution_clock::now();
}

// high_resolution_clock::time_point
template<typename T> inline double time_span(T end, T start) {
  duration<double> span = duration_cast<duration<double>>(end-start);
  return span.count();
}

/// Precision clamp
inline double limit_prec(double x) { 
  return fabs(x)<1e-4 ? 0 : x; 
}

/// Signum
inline double sign(double x) {
  if (x>0) return 1;
  else if (x<0) return -1;
  else return 0;
}

/// Clamp function
inline double clamp(double x) { return x>0 ? x : 0; }

/// Sigmoid
inline double sigmoid(double x) {
  return 1./(1.+exp(-x));
}

/// Inverse sigmoid 
inline double invSigmoid(double x) {
  return log(x/(1.-x));
}

/// Safe array delete function
template<typename T> inline void safe_delete(T* P) { 
  if (P) { 
    delete [] P; 
    P=0; 
  }
}

/// Swap function
template<typename T> inline void swap(T &a, T &b) { 
  T t=a; a=b; b=t; 
}

// Min/Max functions
template<typename T> T min(T a, T b) { return a<b?a:b; }
template<typename T> T max(T a, T b) { return a<b?b:a; }

/// Helper function two argument absmin function
template<typename T> inline T absmin(T a, T b) {
  return fabs(a)<fabs(b) ? a : b;
}

/// Helper three argument min function
template<typename T> inline T min(T a, T b, T c) {
  return min(min(a, b), c);
}

/// Helper three argument absmin function
template<typename T> inline T absmin(T a, T b, T c) {
  return absmin(absmin(a, b), c);
}

/// Helper squaring function
template<typename T> inline T sqr(T x) { return x*x; }

// Print as (hrs):(mins):(sec)
inline string printAsTime(double seconds) {
  stringstream stream;
  string str;
  int hours = floor(seconds/3600.);
  seconds -= 3600*hours;
  int minutes = floor(seconds/60.);
  seconds -= 60*minutes;
  if (hours<10) stream << "0" << hours << ":";
  else stream << hours << ":";
  if (minutes<10) stream << "0" << minutes << ":";
  else stream << minutes << ":";
  if (seconds<10) stream << "0" << seconds;
  else stream << seconds;
  stream >> str;
  return str;
}

// Get rid of all the "e-"s for mathematica, truncate doubles after <precision> decimal places
inline string mmPreproc(string s, int precision=-1) {
  int size = s.size();
  string out;
  out.reserve(size);
  for (int i=0; i<size; i++) {
    // Get numbers
    if (precision>=0 && isdigit(s.at(i))) {
      string number;
      number.push_back(s.at(i));
      i++;
      int count = 0, decimal = 0; // decimal == 1 when we detect a '.'
      for (; i<size && (isdigit(s.at(i)) || s.at(i)=='.'); i++) {
	if (s.at(i)=='.') { decimal = 1; number.push_back('.');	}
	else if (decimal==0) number.push_back(s.at(i));
	else if (count<precision) { number.push_back(s.at(i)); count++;	}
      }
      // Done getting the number
      out += number;
      if (i==size) return out; // We are done
      i--;
    }
    // Get nan, replace with 10^10
    else if (s.at(i)=='n' && i+2<size && s.at(i+1)=='a' && s.at(i+2)=='n') {
      out += "10^10";
      i+=2;
    }
    // Get scientific notation
    else if (s.at(i)=='e' && i+2<size && (s.at(i+1)=='-' || s.at(i+1)=='+') && isdigit(s.at(i+2))) out += "*10^";
    else out.push_back(s.at(i));
  }
  return out;
}

template<typename T> inline string mmPreproc(T s, int precision=-1) {
  stringstream stream;
  string str;
  stream << s;
  stream >> str;
  return mmPreproc(str, precision);
}

template<typename T> inline string toStr(T x) {
  stringstream stream;
  string str;
  stream << x;
  stream >> str;
  return str;
}

/// A simple 2-D vector struct
template<typename T=double> struct vect {
vect(T x, T y) : x(x), y(y) {};
vect() : x(T(0)), y(T(0)) {};
vect(T v) : x(v), y(v) {};
vect(const vect<T>& V) : x(V.x), y(V.y) {};
    
  T norm() { return sqrtf(x*x + y*y); }
  T normSqr() { return x*x + y*y; }
  
  void normalize() {
    T nrm = norm();
    T n = nrm!=0 ? 1.f/nrm : 1;
    x *= n; y *= n;
  }
  
  vect<T>& operator=(const vect<T>& V) {
    x = V.x; y = V.y;
    return *this;
  }
    
  vect<T>& operator=(const T v) {
    x = v; y = v;
    return *this;
  }
    
  bool operator==(const vect<T>& V) const {
    return x==V.x && y==V.y;
  }
  
  bool operator!=(const vect<T>& V) const {
    return x!=V.x || y!=V.y;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const vect& v) {
    stringstream stream;
    string str;
    stream << "{" << v.x << "," << v.y << "}";
    stream >> str;
    os << str;
    return os;
  }

  string csv() {
    stringstream stream;
    string str;
    stream << v.x << ',' << v.y;
    stream >> str;
    return str;
  }
    
  T operator*(const vect<T>& B) const {
    return x*B.x + y*B.y;
  }

  friend vect<T> operator*(vect<T>& A, T B) {
    return vect<T>(A.x*B, A.y*B);
  }

  // Cross product
  T operator^(const vect<T>& B) const {
    return x*B.y-y*B.x;
  }
    
  vect<T> operator+(const vect<T>& B) const {
    return vect<T>(x+B.x, y+B.y);
  }

  vect<T> operator-(const vect<T>& B) const {
    return vect<T>(x-B.x, y-B.y);
  }
    
  vect<T> operator-() const {
    return vect<T>(-x,-y);
  }

  vect<T> operator+=(const vect<T>& B) {
    x += B.x; y += B.y;
    return *this;
  }

  vect<T> operator-=(const vect<T>& B) {
    x -= B.x; y -= B.y;
    return *this;
  }
    
  vect<T> operator*=(const T num) {
    x *= num;
    y *= num;
    return *this;
  }
    
  friend vect<T> operator*(const T num, const vect<T>& V) {
    return vect<T>(V.x*num, V.y*num);
  }
    
  static vect<> rand() {
    return vect<>(0.5-drand48(), 0.5-drand48());
  }

  T& operator[] (int i) {
    if (i<0 || i>1) throw 1;
    return i==0 ? x : y;
  }
    
  /// The actual data
  T x, y;
};

// --- What type of vector we will use as our main vector type ---
typedef vect<double> vec2;
typedef pair<vec2, vec2> VPair;
typedef std::tuple<vec2, double, double> Tri;
// PData records all the data you need to print particles: 
// { position, sigma, theta, interaction, 'color' }
typedef std::tuple<vec2, double, double, double, double> PData;

/// Some common vectors
const vect<double> Zero(0,0);
const vect<double> E0(1,0);
const vect<double> E1(0,1);

/// Vector Squaring function
template<typename T> T sqr(vect<T> V) { 
  return V*V; 
}

template<typename T> vect<T> normalize(vect<T> V) {
  V.normalize();
  return V;
}

template<typename T> inline vect<T> normalV(T x, T y) {
  T ns = x*x + y*y;
  T n = sqrt(ns);
  return n>0 ? vect<T> (x/n, y/n) : vect<T>();
}

template<typename T> inline T distSqr(const vect<T>& A, const vect<T>& B) {
  return sqr(A.x-B.x)+sqr(A.y-B.y);
}

inline vec2 randV() {
  float a = drand48();
  return vec2(sinf(2*PI*a), cosf(2*PI*a));
}

template<typename S, typename T> inline std::ostream& operator<<(std::ostream& out, const pair<S,T>& p) {
  out << "{" << p.first << "," << p.second << "}";
  return out;
}

template<typename T> inline std::ostream& operator<<(std::ostream& out, const vector<T>& lst) {
  if (lst.empty()) {
    out << "{}";
    return out;
  }
  stringstream stream;
  string str;
  stream << "{";
  for (int i=0; i<lst.size(); i++) {
    stream << lst.at(i);
    if (i!=lst.size()-1) stream << ",";
  }
  stream << "}";
  stream >> str;
  out << str;
  return out;
}

inline string toCSV(const int n) { return toStr(n); }
inline string toCSV(const double n) { return toStr(n); }
inline string toCSV(const vec2& p) {
  stringstream stream;
  string str;
  stream << p.x << "," << p.y;
  stream >> str;
  return str;
}
template<typename... T> string toCSV(const PData& pdata) {
  stringstream stream;
  string str;
  int size = sizeof...(T);
  stream << toCSV(std::get<0>(pdata)) << "," << std::get<1>(pdata) << "," << std::get<2>(pdata) << "," << std::get<3>(pdata) << "," << std::get<4>(pdata);
  stream >> str;
  return str;
}

template<typename T> bool printToCSV(string filename, const vector<T>& lst) {
  if (lst.empty()) return true;
  ofstream fout(filename);
  if (fout.fail()) return false;
  for (int i=0; i<lst.size(); i++) fout << toCSV(lst.at(i)) << endl;
  fout.close();
  return true;
}

template<typename T> bool printToCSV(string filename, const vector<T>& lst, int iter) {
  stringstream stream;
  stream << filename << iter << ".csv";
  stream >> filename;
  return printToCSV(filename, lst);
}

template<typename T> bool printToCSV(string filename, const vector<vector<T> >& lst) {
  if (lst.empty()) return true;
  ofstream fout(filename);
  if (fout.fail()) return false;
  for (int i=0; i<lst.size(); i++) {
    for (int j=0; j<lst.at(i).size(); ++j) {
      fout << lst.at(i).at(j);
      if (j!=lst.at(i).size()-1) fout << ",";
    }
    fout << endl;
  }
  fout.close();
  return true;
}

inline std::ostream& operator<<(std::ostream& out, const Tri& tup) {
  out << "{" << std::get<0>(tup) << "," << std::get<1>(tup) << "," << std::get<2>(tup) << "}";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const PData& pdata) {
  out << "{" << std::get<0>(pdata) << "," << std::get<1>(pdata) << "," << std::get<2>(pdata) << "," << std::get<3>(pdata) << "," << std::get<4>(pdata) << "}";
  return out;
}

inline void istream_get_whitespace(std::istream& in) {
  char c;
  if (!in.get(c)) return;
  while ((c==' ' || c=='\n' || c=='\r') && !in.eof()) {
    if (!in.get(c)) return;
  }
  if (!in.eof()) in.putback(c);
}

template<typename T> inline std::istream& operator>>(std::istream &in, vect<T> &v) {
  v = Zero;
  char c;
  double x, y;
  istream_get_whitespace(in);
  if (!in.get(c)) return in;
  if (c=='{') {
    in >> x;
    istream_get_whitespace(in);
    if (!in.get(c)) return in;
    if (c!=',') return in;
    in >> y;
    istream_get_whitespace(in);
    if (!in.get(c)) return in;
    if (c!='}') return in;
    v.x = x; v.y = y;
  }
  else in.putback(c);
  return in;
}

template<typename T> inline std::istream& operator>>(std::istream& in, vector<T> &lst) {
  // istream.get(c) returns false if EOF
  lst.clear();
  char c;
  T data;
  double x,y;
  istream_get_whitespace(in);
  if (!in.get(c)) return in;
  if (c=='{') {
    while(c!='}') {
      istream_get_whitespace(in);
      in >> data;
      lst.push_back(data);
      istream_get_whitespace(in);
      if (!in.get(c)) break; // Get either ',' or '}'
      if (c=='}') break; // Ending brace
    }
  }
  else in.putback(c);
  return in;
}

template<typename T> inline std::ostream& operator<<(std::ostream& out, const std::list<T>&lst) {
  if (lst.empty()) {
    out << "{}";
    return out;
  }
  stringstream stream;
  string str;
  stream << "{";
  int i=0;
  for (auto l : lst) {
    stream << l;
    if (i!=lst.size()-1) stream << ",";
    i++;
  }
  stream << "}";
  stream >> str;
  out << str;
  return out;
}

template<typename T> inline string bare_representation(const vector<T>& lst) {
  stringstream stream;
  string str;
  for (int i=0; i<lst.size(); i++) {
    stream << lst.at(i);
    if (i!=lst.size()-1) stream << ",";
  }
  stream >> str;
  return mmPreproc(str);
}

/// Helper averaging function
template<typename T> inline T average(vector<vect<T>> lst) {
  if (lst.empty()) return 0;
  T ave = 0;
  for (auto V : lst) ave += V.y;
  return ave/lst.size();
}

// A useful typedef
typedef pair<vect<float>, bool> vtype;
typedef pair<int,int> ipair;

/// The bounds structure
struct Bounds {
  Bounds() : left(0), right(0), bottom(0), top(0) {};
  Bounds(double l, double r, double b, double t) : left(l), right(r), bottom(b), top(t) {};
  // Data
  double left, right, bottom, top;
  // Member and friend functions
  friend std::ostream& operator<<(std::ostream& out, const Bounds& B) {
    out << "{" << B.left << "," << B.right << "," << B.bottom << "," << B.top << "}";
    return out;
  }
  void cut(double c) {
    left+=c; right-=c; bottom+=c; top-=c;
  }
  Bounds& operator=(const Bounds& b) {
    left = b.left; right = b.right;
    bottom = b.bottom; top = b.top;
    return *this;
  }
  bool operator==(const Bounds& b) const {
    return b.left==left && b.right==right && b.bottom==bottom && b.top==top;
  }
  bool contains(const vec2 position) const {
    return position.x<right && left<position.x && position.y<top && bottom<position.y;
  }
  bool contains(const vec2 position, const double sigma) const {
    return position.x-sigma<right && left<position.x+sigma && position.y-sigma<top && bottom<position.y+sigma;
  }
  bool contains(const double x, const double y) const {
    return x<right && left<x && y<top && bottom<y;
  }
};

const Bounds NullBounds(0., -1., 0., -1.);

struct Trio {
  Trio() : x(0), y(0), z(0) {};
  Trio(double x, double y, double z) : x(x), y(y), z(z) {};

  friend std::ostream& operator<<(std::ostream& out, const Trio &T) {
    out << "{" << T.x << "," << T.y << "," << T.z << "}";
    return out;
  }

  double x,y,z;
};

inline string toCSV(const Bounds& b) {
  stringstream stream;
  string str;
  stream << b.left << "," << b.right << "," << b.bottom << "," << b.top;
  stream >> str;
  return str;
}


#endif
