%module "enum_thorough"

// Suppress warning messages from the Ruby module for all of the following...
%warnfilter(801) red;
%warnfilter(801) blue;
%warnfilter(801) blue;
%warnfilter(801) SpeedClass::slow;
%warnfilter(801) SpeedClass::medium;
%warnfilter(801) SpeedClass::fast;
%warnfilter(801) TemplateClass::einstein;
%warnfilter(801) TemplateClass::galileo;
%warnfilter(801) Name::TClass::faraday;
%warnfilter(801) Name::TClass::bell;
%warnfilter(801) argh;
%warnfilter(801) OldNameStruct::whizz;
%warnfilter(801) OldNameStruct::bang;
%warnfilter(801) OldNameStruct::pop;
%warnfilter(801) OldNameStruct::simple1;
%warnfilter(801) OldNameStruct::singlename1;
%warnfilter(801) OldNameStruct::doublename1;
%warnfilter(801) TreesClass::oak;
%warnfilter(801) TreesClass::fir;
%warnfilter(801) TreesClass::pine;
%warnfilter(801) curly::greasy::HairStruct::blonde;
%warnfilter(801) curly::greasy::HairStruct::ginger;
%warnfilter(801) Obscure::one;
%warnfilter(801) Obscure::onetrail;
%warnfilter(801) Obscure::two;
%warnfilter(801) Obscure::twoagain;
%warnfilter(801) Obscure::twotrail;
%warnfilter(801) Obscure::twotrailagain;

%inline %{

enum { AnonEnum1, AnonEnum2 };
enum { ReallyAnInteger = 200 };
//enum { AnonEnum3, AnonEnum4 } instance;
namespace AnonSpace {
  enum { AnonSpaceEnum1, AnonSpaceEnum2 };
  struct AnonStruct {
    enum { AnonStructEnum1, AnonStructEnum2 };
  };
}


enum colour { red, blue };
colour colourTest1(colour e) { return e; }
enum colour colourTest2(enum colour e) { return e; }
const colour colourTest3(const colour e) { return e; }
const enum colour colourTest4(const enum colour e) { return e; }

colour myColour;

struct SpeedClass {
  enum speed { slow=10, medium=20, fast=30, lightning };
  typedef enum speed speedtd1;

  speed                         speedTest1(speed s) { return s; }
  enum speed                    speedTest2(enum speed s) { return s; }
  const speed                   speedTest3(const speed s) { return s; }
  const enum speed              speedTest4(const enum speed s) { return s; }
  speedtd1                      speedTest5(speedtd1 s) { return s; }
  const speedtd1                speedTest6(const speedtd1 s) { return s; }

  const colour myColour2;
  speedtd1 mySpeedtd1;
  SpeedClass() : myColour2(red), mySpeedtd1(slow) { }
};

SpeedClass::speed              speedTest1(SpeedClass::speed s) { return s; }
enum SpeedClass::speed         speedTest2(enum SpeedClass::speed s) { return s; }
const SpeedClass::speed        speedTest3(const SpeedClass::speed s) { return s; }
const enum SpeedClass::speed   speedTest4(const enum SpeedClass::speed s) { return s; }


typedef enum { NamedAnon1, NamedAnon2 } namedanon;

namedanon                       namedanonTest1(namedanon e) { return e; } 

typedef enum twonamestag { TwoNames1, TwoNames2 } twonames;

twonames                        twonamesTest1(twonames e) { return e; } 
twonamestag                     twonamesTest2(twonamestag e) { return e; } 
enum twonamestag                twonamesTest3(enum twonamestag e) { return e; } 

struct TwoNamesStruct {
  typedef enum twonamestag { TwoNamesStruct1, TwoNamesStruct2 } twonames;
  twonames                      twonamesTest1(twonames e) { return e; } 
  twonamestag                   twonamesTest2(twonamestag e) { return e; } 
  enum twonamestag              twonamesTest3(enum twonamestag e) { return e; } 
};

namespace AnonSpace{
  typedef enum { NamedAnonSpace1, NamedAnonSpace2 } namedanonspace;
  namedanonspace                namedanonspaceTest1(namedanonspace e) { return e; } 
  AnonSpace::namedanonspace     namedanonspaceTest2(AnonSpace::namedanonspace e) { return e; } 
}
AnonSpace::namedanonspace       namedanonspaceTest3(AnonSpace::namedanonspace e) { return e; } 
using namespace AnonSpace;
namedanonspace                  namedanonspaceTest4(namedanonspace e) { return e; } 


template<typename T> struct TemplateClass {
  enum scientists { einstein, galileo }; 
  typedef enum scientists scientiststd1;
  typedef scientists scientiststd2;
  typedef scientiststd1 scientiststd3;
  scientists                                scientistsTest1(scientists e) { return e; }
  enum scientists                           scientistsTest2(enum scientists e) { return e; }
  const scientists                          scientistsTest3(const scientists e) { return e; }
  const enum scientists                     scientistsTest4(const enum scientists e) { return e; }
  TemplateClass<T>::scientists              scientistsTest5(TemplateClass<T>::scientists e) { return e; }
  const TemplateClass<T>::scientists        scientistsTest6(const TemplateClass<T>::scientists e) { return e; }
  enum TemplateClass<T>::scientists         scientistsTest7(enum TemplateClass<T>::scientists e) { return e; }
  const enum TemplateClass<T>::scientists   scientistsTest8(const enum TemplateClass<T>::scientists e) { return e; }
  TemplateClass::scientists                 scientistsTest9(TemplateClass::scientists e) { return e; }
//  enum TemplateClass::scientists            scientistsTestA(enum TemplateClass::scientists e) { return e; }
  const TemplateClass::scientists           scientistsTestB(const TemplateClass::scientists e) { return e; }
//  const enum TemplateClass::scientists      scientistsTestC(const enum TemplateClass::scientists e) { return e; }
  scientiststd1                             scientistsTestD(scientiststd1 e) { return e; }
  scientiststd2                             scientistsTestE(scientiststd2 e) { return e; }
  scientiststd3                             scientistsTestF(scientiststd3 e) { return e; }
  TemplateClass<T>::scientiststd1           scientistsTestG(TemplateClass<T>::scientiststd1 e) { return e; }
  TemplateClass<T>::scientiststd2           scientistsTestH(TemplateClass<T>::scientiststd2 e) { return e; }
  TemplateClass<T>::scientiststd3           scientistsTestI(TemplateClass<T>::scientiststd3 e) { return e; }
};

TemplateClass<int>::scientists              scientistsTest1(TemplateClass<int>::scientists e) { return e; }
const TemplateClass<int>::scientists        scientistsTest2(const TemplateClass<int>::scientists e) { return e; }
enum TemplateClass<int>::scientists         scientistsTest3(enum TemplateClass<int>::scientists e) { return e; }
const enum TemplateClass<int>::scientists   scientistsTest4(const enum TemplateClass<int>::scientists e) { return e; }
TemplateClass<int>::scientiststd1           scientistsTest5(TemplateClass<int>::scientiststd1 e) { return e; }
TemplateClass<int>::scientiststd2           scientistsTest6(TemplateClass<int>::scientiststd2 e) { return e; }
TemplateClass<int>::scientiststd3           scientistsTest7(TemplateClass<int>::scientiststd3 e) { return e; }


namespace Name {
template<typename T> struct TClass {
  enum scientists { faraday, bell }; 
  typedef enum scientists scientiststd1;
  typedef scientists scientiststd2;
  typedef scientiststd1 scientiststd3;
  scientists                                scientistsNameTest1(scientists e) { return e; }
  enum scientists                           scientistsNameTest2(enum scientists e) { return e; }
  const scientists                          scientistsNameTest3(const scientists e) { return e; }
  const enum scientists                     scientistsNameTest4(const enum scientists e) { return e; }
  TClass<T>::scientists                     scientistsNameTest5(TClass<T>::scientists e) { return e; }
  const TClass<T>::scientists               scientistsNameTest6(const TClass<T>::scientists e) { return e; }
  enum TClass<T>::scientists                scientistsNameTest7(enum TClass<T>::scientists e) { return e; }
  const enum TClass<T>::scientists          scientistsNameTest8(const enum TClass<T>::scientists e) { return e; }
  TClass::scientists                        scientistsNameTest9(TClass::scientists e) { return e; }
//  enum TClass::scientists                   scientistsNameTestA(enum TClass::scientists e) { return e; }
  const TClass::scientists                  scientistsNameTestB(const TClass::scientists e) { return e; }
//  const enum TClass::scientists             scientistsNameTestC(const enum TClass::scientists e) { return e; }
  scientiststd1                             scientistsNameTestD(scientiststd1 e) { return e; }
  scientiststd2                             scientistsNameTestE(scientiststd2 e) { return e; }
  scientiststd3                             scientistsNameTestF(scientiststd3 e) { return e; }
  TClass<T>::scientiststd1                  scientistsNameTestG(TClass<T>::scientiststd1 e) { return e; }
  TClass<T>::scientiststd2                  scientistsNameTestH(TClass<T>::scientiststd2 e) { return e; }
  TClass<T>::scientiststd3                  scientistsNameTestI(TClass<T>::scientiststd3 e) { return e; }

  Name::TClass<T>::scientists               scientistsNameSpaceTest1(Name::TClass<T>::scientists e) { return e; }
  const Name::TClass<T>::scientists         scientistsNameSpaceTest2(const Name::TClass<T>::scientists e) { return e; }
  enum Name::TClass<T>::scientists          scientistsNameSpaceTest3(enum Name::TClass<T>::scientists e) { return e; }
  const enum Name::TClass<T>::scientists    scientistsNameSpaceTest4(const enum Name::TClass<T>::scientists e) { return e; }
  Name::TClass<T>::scientiststd1            scientistsNameSpaceTest5(Name::TClass<T>::scientiststd1 e) { return e; }
  Name::TClass<T>::scientiststd2            scientistsNameSpaceTest6(Name::TClass<T>::scientiststd2 e) { return e; }
  Name::TClass<T>::scientiststd3            scientistsNameSpaceTest7(Name::TClass<T>::scientiststd3 e) { return e; }

  // Test TemplateClass::scientists rather then TClass::scientists
  ::TemplateClass<int>::scientists              scientistsOtherTest1(::TemplateClass<int>::scientists e) { return e; }
  const ::TemplateClass<int>::scientists        scientistsOtherTest2(const ::TemplateClass<int>::scientists e) { return e; }
  enum ::TemplateClass<int>::scientists         scientistsOtherTest3(enum ::TemplateClass<int>::scientists e) { return e; }
  const enum ::TemplateClass<int>::scientists   scientistsOtherTest4(const enum ::TemplateClass<int>::scientists e) { return e; }
  ::TemplateClass<int>::scientiststd1           scientistsOtherTest5(::TemplateClass<int>::scientiststd1 e) { return e; }
  ::TemplateClass<int>::scientiststd2           scientistsOtherTest6(::TemplateClass<int>::scientiststd2 e) { return e; }
  ::TemplateClass<int>::scientiststd3           scientistsOtherTest7(::TemplateClass<int>::scientiststd3 e) { return e; }
};

TClass<int>::scientists                     scientistsNameTest1(TClass<int>::scientists e) { return e; }
const TClass<int>::scientists               scientistsNameTest2(const TClass<int>::scientists e) { return e; }
enum TClass<int>::scientists                scientistsNameTest3(enum TClass<int>::scientists e) { return e; }
const enum TClass<int>::scientists          scientistsNameTest4(const enum TClass<int>::scientists e) { return e; }
TClass<int>::scientiststd1                  scientistsNameTest5(TClass<int>::scientiststd1 e) { return e; }
TClass<int>::scientiststd2                  scientistsNameTest6(TClass<int>::scientiststd2 e) { return e; }
TClass<int>::scientiststd3                  scientistsNameTest7(TClass<int>::scientiststd3 e) { return e; }

Name::TClass<int>::scientists               scientistsNameSpaceTest1(Name::TClass<int>::scientists e) { return e; }
const Name::TClass<int>::scientists         scientistsNameSpaceTest2(const Name::TClass<int>::scientists e) { return e; }
enum Name::TClass<int>::scientists          scientistsNameSpaceTest3(enum Name::TClass<int>::scientists e) { return e; }
const enum Name::TClass<int>::scientists    scientistsNameSpaceTest4(const enum Name::TClass<int>::scientists e) { return e; }
Name::TClass<int>::scientiststd1            scientistsNameSpaceTest5(Name::TClass<int>::scientiststd1 e) { return e; }
Name::TClass<int>::scientiststd2            scientistsNameSpaceTest6(Name::TClass<int>::scientiststd2 e) { return e; }
Name::TClass<int>::scientiststd3            scientistsNameSpaceTest7(Name::TClass<int>::scientiststd3 e) { return e; }
}

Name::TClass<int>::scientists               scientistsNameSpaceTest8(Name::TClass<int>::scientists e) { return e; }
const Name::TClass<int>::scientists         scientistsNameSpaceTest9(const Name::TClass<int>::scientists e) { return e; }
enum Name::TClass<int>::scientists          scientistsNameSpaceTestA(enum Name::TClass<int>::scientists e) { return e; }
const enum Name::TClass<int>::scientists    scientistsNameSpaceTestB(const enum Name::TClass<int>::scientists e) { return e; }
Name::TClass<int>::scientiststd1            scientistsNameSpaceTestC(Name::TClass<int>::scientiststd1 e) { return e; }
Name::TClass<int>::scientiststd2            scientistsNameSpaceTestD(Name::TClass<int>::scientiststd2 e) { return e; }
Name::TClass<int>::scientiststd3            scientistsNameSpaceTestE(Name::TClass<int>::scientiststd3 e) { return e; }

using namespace Name;
TClass<int>::scientists                     scientistsNameSpaceTestF(TClass<int>::scientists e) { return e; }
const TClass<int>::scientists               scientistsNameSpaceTestG(const TClass<int>::scientists e) { return e; }
enum TClass<int>::scientists                scientistsNameSpaceTestH(enum TClass<int>::scientists e) { return e; }
const enum TClass<int>::scientists          scientistsNameSpaceTestI(const enum TClass<int>::scientists e) { return e; }
TClass<int>::scientiststd1                  scientistsNameSpaceTestJ(TClass<int>::scientiststd1 e) { return e; }
TClass<int>::scientiststd2                  scientistsNameSpaceTestK(TClass<int>::scientiststd2 e) { return e; }
TClass<int>::scientiststd3                  scientistsNameSpaceTestL(TClass<int>::scientiststd3 e) { return e; }

%}

%template(TemplateClassInt) TemplateClass<int>;
%template(TClassInt) Name::TClass<int>;


// %rename tests
%rename(NewNameStruct) OldNameStruct;
%rename(newname) oldname;
%rename(doublenamerenamed) doublename;
%rename(simplerenamed) simple;
%rename(singlenamerenamed) singlename;
%rename(bang) OldNameStruct::kerboom;

%inline %{
enum oldname { argh };
typedef oldname oldnametd;
oldname                             renameTest1(oldname e) { return e; }
oldnametd                           renameTest2(oldnametd e) { return e; }

struct OldNameStruct {
  enum enumeration {whizz, kerboom, pop};
  enumeration                       renameTest1(enumeration e) { return e; }
  OldNameStruct::enumeration        renameTest2(OldNameStruct::enumeration e) { return e; }

  enum simple {simple1};
  typedef enum doublenametag {doublename1} doublename;
  typedef enum {singlename1} singlename;

  simple                            renameTest3(simple e) { return e; }
  doublename                        renameTest4(doublename e) { return e; }
  doublenametag                     renameTest5(doublenametag e) { return e; }
  singlename                        renameTest6(singlename e) { return e; }
};

OldNameStruct::enumeration          renameTest3(OldNameStruct::enumeration e) { return e; }
OldNameStruct::simple               renameTest4(OldNameStruct::simple e) { return e; }
OldNameStruct::doublename           renameTest5(OldNameStruct::doublename e) { return e; }
OldNameStruct::doublenametag        renameTest6(OldNameStruct::doublenametag e) { return e; }
OldNameStruct::singlename           renameTest7(OldNameStruct::singlename e) { return e; }
%}

%inline %{
struct TreesClass {
  enum trees {oak, fir, pine };
  typedef enum trees treestd1;
  typedef trees treestd2;
  typedef treestd1 treestd3;
  typedef TreesClass::trees treestd4;
  typedef treestd1 treestd5;

  trees                             treesTest1(trees e) { return e; }
  treestd1                          treesTest2(treestd1 e) { return e; }
  treestd2                          treesTest3(treestd2 e) { return e; }
  treestd3                          treesTest4(treestd3 e) { return e; }
  treestd4                          treesTest5(treestd4 e) { return e; }
  treestd5                          treesTest6(treestd5 e) { return e; }
  const trees                       treesTest7(const trees e) { return e; }
  const treestd1                    treesTest8(const treestd1 e) { return e; }
  const treestd2                    treesTest9(const treestd2 e) { return e; }
  const treestd3                    treesTestA(const treestd3 e) { return e; }
  const treestd4                    treesTestB(const treestd4 e) { return e; }
  const treestd5                    treesTestC(const treestd5 e) { return e; }
  TreesClass::trees                 treesTestD(TreesClass::trees e) { return e; }
  TreesClass::treestd1              treesTestE(TreesClass::treestd1 e) { return e; }
  TreesClass::treestd2              treesTestF(TreesClass::treestd2 e) { return e; }
  TreesClass::treestd3              treesTestG(TreesClass::treestd3 e) { return e; }
  TreesClass::treestd4              treesTestH(TreesClass::treestd4 e) { return e; }
  TreesClass::treestd5              treesTestI(TreesClass::treestd5 e) { return e; }
  const TreesClass::trees           treesTestJ(const TreesClass::trees e) { return e; }
  const TreesClass::treestd1        treesTestK(const TreesClass::treestd1 e) { return e; }
  const TreesClass::treestd2        treesTestL(const TreesClass::treestd2 e) { return e; }
  const TreesClass::treestd3        treesTestM(const TreesClass::treestd3 e) { return e; }
  const TreesClass::treestd4        treesTestN(const TreesClass::treestd4 e) { return e; }
  const TreesClass::treestd5        treesTestO(const TreesClass::treestd5 e) { return e; }
};

TreesClass::trees                   treesTest1(TreesClass::trees e) { return e; }
TreesClass::treestd1                treesTest2(TreesClass::treestd1 e) { return e; }
TreesClass::treestd2                treesTest3(TreesClass::treestd2 e) { return e; }
TreesClass::treestd3                treesTest4(TreesClass::treestd3 e) { return e; }
TreesClass::treestd4                treesTest5(TreesClass::treestd4 e) { return e; }
TreesClass::treestd5                treesTest6(TreesClass::treestd5 e) { return e; }
const TreesClass::trees             treesTest7(const TreesClass::trees e) { return e; }
const TreesClass::treestd1          treesTest8(const TreesClass::treestd1 e) { return e; }
const TreesClass::treestd2          treesTest9(const TreesClass::treestd2 e) { return e; }
const TreesClass::treestd3          treesTestA(const TreesClass::treestd3 e) { return e; }
const TreesClass::treestd4          treesTestB(const TreesClass::treestd4 e) { return e; }
const TreesClass::treestd5          treesTestC(const TreesClass::treestd5 e) { return e; }

typedef enum TreesClass::trees treesglobaltd1;
typedef TreesClass::trees treesglobaltd2;
typedef TreesClass::treestd1 treesglobaltd3;
typedef TreesClass::treestd2 treesglobaltd4;
typedef treesglobaltd4 treesglobaltd5;

treesglobaltd1                      treesTestD(treesglobaltd1 e) { return e; }
treesglobaltd2                      treesTestE(treesglobaltd2 e) { return e; }
treesglobaltd3                      treesTestF(treesglobaltd3 e) { return e; }
treesglobaltd4                      treesTestG(treesglobaltd4 e) { return e; }
treesglobaltd5                      treesTestH(treesglobaltd5 e) { return e; }
const treesglobaltd1                treesTestI(const treesglobaltd1 e) { return e; }
const treesglobaltd2                treesTestJ(const treesglobaltd2 e) { return e; }
const treesglobaltd3                treesTestK(const treesglobaltd3 e) { return e; }
const treesglobaltd4                treesTestL(const treesglobaltd4 e) { return e; }
const treesglobaltd5                treesTestM(const treesglobaltd5 e) { return e; }

typedef const enum TreesClass::trees treesglobaltd6;
typedef const TreesClass::trees treesglobaltd7;
typedef const TreesClass::treestd1 treesglobaltd8;
typedef const TreesClass::treestd2 treesglobaltd9;
typedef const treesglobaltd4 treesglobaltdA;

//treesglobaltd6                      treesTestN(treesglobaltd6 e) { return e; } // casting using an int instead of treesglobaltd6
treesglobaltd7                      treesTestO(treesglobaltd7 e) { return e; }
treesglobaltd8                      treesTestP(treesglobaltd8 e) { return e; }
treesglobaltd9                      treesTestQ(treesglobaltd9 e) { return e; }
treesglobaltdA                      treesTestR(treesglobaltdA e) { return e; }

namespace curly {
  namespace greasy {
    struct HairStruct {
      enum hair { blonde=0xFF0, ginger };
      typedef hair hairtd1;
      typedef HairStruct::hair hairtd2;
      typedef greasy::HairStruct::hair hairtd3;
      typedef curly::greasy::HairStruct::hair hairtd4;
      typedef ::curly::greasy::HairStruct::hair hairtd5;
      typedef hairtd1 hairtd6;
      typedef HairStruct::hairtd1 hairtd7;
      typedef greasy::HairStruct::hairtd1 hairtd8;
      typedef curly::greasy::HairStruct::hairtd1 hairtd9;
      typedef ::curly::greasy::HairStruct::hairtd1 hairtdA;
      hair                          hairTest1(hair e) { return e; }
      hairtd1                       hairTest2(hairtd1 e) { return e; }
      hairtd2                       hairTest3(hairtd2 e) { return e; }
      hairtd3                       hairTest4(hairtd3 e) { return e; }
      hairtd4                       hairTest5(hairtd4 e) { return e; }
      hairtd5                       hairTest6(hairtd5 e) { return e; }
      hairtd6                       hairTest7(hairtd6 e) { return e; }
      hairtd7                       hairTest8(hairtd7 e) { return e; }
      hairtd8                       hairTest9(hairtd8 e) { return e; }
      hairtd9                       hairTestA(hairtd9 e) { return e; }
      hairtdA                       hairTestB(hairtdA e) { return e; }

      ::colour                      colourTest1(::colour e) { return e; }
      enum colour                   colourTest2(enum colour e) { return e; }
      namedanon                     namedanonTest1(namedanon e) { return e; }
      AnonSpace::namedanonspace      namedanonspaceTest1(AnonSpace::namedanonspace e) { return e; }

      treesglobaltd1                treesGlobalTest1(treesglobaltd1 e) { return e; }
      treesglobaltd2                treesGlobalTest2(treesglobaltd2 e) { return e; }
      treesglobaltd3                treesGlobalTest3(treesglobaltd3 e) { return e; }
      treesglobaltd4                treesGlobalTest4(treesglobaltd4 e) { return e; }
      treesglobaltd5                treesGlobalTest5(treesglobaltd5 e) { return e; }

    };
    HairStruct::hair                hairTest1(HairStruct::hair e) { return e; }
    HairStruct::hairtd1             hairTest2(HairStruct::hairtd1 e) { return e; }
    HairStruct::hairtd2             hairTest3(HairStruct::hairtd2 e) { return e; }
    HairStruct::hairtd3             hairTest4(HairStruct::hairtd3 e) { return e; }
    HairStruct::hairtd4             hairTest5(HairStruct::hairtd4 e) { return e; }
    HairStruct::hairtd5             hairTest6(HairStruct::hairtd5 e) { return e; }
    HairStruct::hairtd6             hairTest7(HairStruct::hairtd6 e) { return e; }
    HairStruct::hairtd7             hairTest8(HairStruct::hairtd7 e) { return e; }
    HairStruct::hairtd8             hairTest9(HairStruct::hairtd8 e) { return e; }
    HairStruct::hairtd9             hairTestA(HairStruct::hairtd9 e) { return e; }
    HairStruct::hairtdA             hairTestB(HairStruct::hairtdA e) { return e; }
  }
  greasy::HairStruct::hair          hairTestA1(greasy::HairStruct::hair e) { return e; }
  greasy::HairStruct::hairtd1       hairTestA2(greasy::HairStruct::hairtd1 e) { return e; }
  greasy::HairStruct::hairtd2       hairTestA3(greasy::HairStruct::hairtd2 e) { return e; }
  greasy::HairStruct::hairtd3       hairTestA4(greasy::HairStruct::hairtd3 e) { return e; }
  greasy::HairStruct::hairtd4       hairTestA5(greasy::HairStruct::hairtd4 e) { return e; }
  greasy::HairStruct::hairtd5       hairTestA6(greasy::HairStruct::hairtd5 e) { return e; }
  greasy::HairStruct::hairtd6       hairTestA7(greasy::HairStruct::hairtd6 e) { return e; }
  greasy::HairStruct::hairtd7       hairTestA8(greasy::HairStruct::hairtd7 e) { return e; }
  greasy::HairStruct::hairtd8       hairTestA9(greasy::HairStruct::hairtd8 e) { return e; }
  greasy::HairStruct::hairtd9       hairTestAA(greasy::HairStruct::hairtd9 e) { return e; }
  greasy::HairStruct::hairtdA       hairTestAB(greasy::HairStruct::hairtdA e) { return e; }
}
curly::greasy::HairStruct::hair     hairTestB1(curly::greasy::HairStruct::hair e) { return e; }
curly::greasy::HairStruct::hairtd1  hairTestB2(curly::greasy::HairStruct::hairtd1 e) { return e; }
curly::greasy::HairStruct::hairtd2  hairTestB3(curly::greasy::HairStruct::hairtd2 e) { return e; }
curly::greasy::HairStruct::hairtd3  hairTestB4(curly::greasy::HairStruct::hairtd3 e) { return e; }
curly::greasy::HairStruct::hairtd4  hairTestB5(curly::greasy::HairStruct::hairtd4 e) { return e; }
curly::greasy::HairStruct::hairtd5  hairTestB6(curly::greasy::HairStruct::hairtd5 e) { return e; }
curly::greasy::HairStruct::hairtd6  hairTestB7(curly::greasy::HairStruct::hairtd6 e) { return e; }
curly::greasy::HairStruct::hairtd7  hairTestB8(curly::greasy::HairStruct::hairtd7 e) { return e; }
curly::greasy::HairStruct::hairtd8  hairTestB9(curly::greasy::HairStruct::hairtd8 e) { return e; }
curly::greasy::HairStruct::hairtd9  hairTestBA(curly::greasy::HairStruct::hairtd9 e) { return e; }
curly::greasy::HairStruct::hairtdA  hairTestBB(curly::greasy::HairStruct::hairtdA e) { return e; }

using curly::greasy::HairStruct;
HairStruct::hair                    hairTestC1(HairStruct::hair e) { return e; }
HairStruct::hairtd1                 hairTestC2(HairStruct::hairtd1 e) { return e; }
HairStruct::hairtd2                 hairTestC3(HairStruct::hairtd2 e) { return e; }
HairStruct::hairtd3                 hairTestC4(HairStruct::hairtd3 e) { return e; }
HairStruct::hairtd4                 hairTestC5(HairStruct::hairtd4 e) { return e; }
HairStruct::hairtd5                 hairTestC6(HairStruct::hairtd5 e) { return e; }
HairStruct::hairtd6                 hairTestC7(HairStruct::hairtd6 e) { return e; }
HairStruct::hairtd7                 hairTestC8(HairStruct::hairtd7 e) { return e; }
HairStruct::hairtd8                 hairTestC9(HairStruct::hairtd8 e) { return e; }
HairStruct::hairtd9                 hairTestCA(HairStruct::hairtd9 e) { return e; }
HairStruct::hairtdA                 hairTestCB(HairStruct::hairtdA e) { return e; }

namespace curly {
  namespace greasy {
    struct FirStruct : HairStruct {
      hair                          hairTestFir1(hair e) { return e; }
      hairtd1                       hairTestFir2(hairtd1 e) { return e; }
      hairtd2                       hairTestFir3(hairtd2 e) { return e; }
      hairtd3                       hairTestFir4(hairtd3 e) { return e; }
      hairtd4                       hairTestFir5(hairtd4 e) { return e; }
      hairtd5                       hairTestFir6(hairtd5 e) { return e; }
      hairtd6                       hairTestFir7(hairtd6 e) { return e; }
      hairtd7                       hairTestFir8(hairtd7 e) { return e; }
      hairtd8                       hairTestFir9(hairtd8 e) { return e; }
      hairtd9                       hairTestFirA(hairtd9 e) { return e; }
      hairtdA                       hairTestFirB(hairtdA e) { return e; }
    };
  }
}

struct Obscure {
  enum {};
  enum {,};
  enum Zero {};
  enum ZeroTrail {,};
  enum One {one};
  enum OneTrail {onetrail,};
  enum Two {two, twoagain};
  enum TwoTrail {twotrail, twotrailagain,};
  typedef enum {};
  typedef enum {,};
  typedef enum Empty {};
  typedef enum EmptyTrail {,};
  typedef enum {} AlsoEmpty;
  typedef enum {,} AlsoEmptyTrail;
};

// Unnamed enum instance
enum { globalinstance1, globalinstance2 } GlobalInstance;

struct Instances {
  enum { memberinstance1, memberinstance2, memberinstance3 } MemberInstance;
  Instances() : MemberInstance(memberinstance3) {}
};

%}

