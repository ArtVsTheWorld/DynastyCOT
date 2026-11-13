#include "../Source/CrowdRegeneration.h"
#include "../Source/VisitorPlans.h"
#include <cassert>
#include <sstream>
#include <iostream>
int main(){
 using namespace attendance;
 turnout::Seed s;s.dynasty=-1972245012;s.regular=true;s.serial=9;s.dynastyGeneration=2;
 regeneration::Request request;assert(!request.fresh(s));request.observe(true);assert(request.fresh(s));
 for(int n=0;n<4;++n){auto excluded=s;if(n==0)excluded.resumed=true;if(n==1)excluded.dynasty=0;if(n==2)excluded.regular=false;if(n==3)excluded.serial=0;
  assert(!request.fresh(excluded)&&!request.commit(excluded)&&request.pending);}
 // Cancelled/unsupported launches leave the explicit request queued.
 assert(request.fresh(s)&&request.pending);assert(request.commit(s)&&!request.pending&&request.disarmed);
 assert(request.fresh(s)&&!request.commit(s)); // duplicate copies of the same launch keep its fresh-plan marker
 auto next=s;++next.serial;assert(!request.fresh(next));request.observe(true);assert(!request.fresh(next)); // failed/deferred INI reset cannot repeat
 request.observe(false);request.observe(true);assert(request.commit(next));
 auto another=next;++another.dynastyGeneration;assert(!request.fresh(another));another=next;++another.dynasty;assert(!request.fresh(another));
 request.observe(false);request.observe(true);auto spring=next;++spring.serial;spring.regular=false;spring.spring=true;assert(request.fresh(spring)&&request.commit(spring));
 visitors::FixtureKey one{-1972245012,1100,1200,5,6,33,4,0},two=one;++two.week;
 visitors::PlanBook book;visitors::Plan old;old.valid=true;old.share=.30f;old.layoutVersion=1;visitors::Plan frozen;unsigned seed=0;bool reused=false;
 assert(book.resolve(one,old,123,frozen,seed,reused));assert(book.bind(one,77));assert(book.resolve(two,old,456,frozen,seed,reused));
 auto preserved=*book.find(two);assert(book.rebuild(one)&&!book.find(one)&&book.find(two)->seed==preserved.seed&&book.find(two)->share==preserved.share);
 assert(!book.rebuild(one));auto invalid=one;invalid.dynasty=0;assert(!book.rebuild(invalid));
 visitors::Plan current=old;current.share=.12f;current.layoutVersion=2;
 assert(book.resolve(one,current,789,frozen,seed,reused)&&!reused&&seed==789&&frozen.share==current.share);
 assert(book.source(one)==0&&book.find(two)->seed==456);
 std::stringstream disk;book.write(disk);visitors::PlanBook restored;assert(visitors::PlanBook::read(disk,restored));
 assert(restored.resolve(one,old,999,frozen,seed,reused)&&reused&&seed==789&&frozen.share==current.share&&frozen.layoutVersion==2);
 std::cout<<"PASS regeneration: default preservation, cancelled/unsupported/resumed rejection, signed Dynasty, one-shot/duplicate-launch/reset-failure guards, spring support, exact-fixture replacement, other-plan preservation and stable new-plan reload; no game access\n";
}
