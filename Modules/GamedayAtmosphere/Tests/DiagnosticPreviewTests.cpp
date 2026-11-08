#include "../Source/DiagnosticPreview.h"
#include <cassert>
#include <iostream>
using namespace attendance;
int main(){
 diagnostic::Request r;assert(diagnostic::parse("DGA-DIAGNOSTIC-1 42 10000 123abc eruption 20 1 1",42,10001,r));
 for(const auto& bad:{"DGA-DIAGNOSTIC-1 99 10000 1 roar 20 1 0","DGA-DIAGNOSTIC-1 42 10000 0 roar 20 1 0","DGA-DIAGNOSTIC-1 42 10000 1 invalid 20 1 0","DGA-DIAGNOSTIC-1 42 10000 1 roar 999 1 0","DGA-DIAGNOSTIC-1 42 10000 1 roar 20 1 0 trailing","DGA-DIAGNOSTIC-1 42 10000 1 roar 20 0 0"})assert(!diagnostic::parse(bad,42,10001,r));
 assert(!diagnostic::parse("DGA-DIAGNOSTIC-1 42 10000 1 roar 20 1 0",42,30001,r));
 assert(!diagnostic::parse("DGA-DIAGNOSTIC-1 42 90000 1 roar 20 1 0",42,10001,r));
 core::Settings settings;motion::Settings camera;diagnostic::Context c{100,1,1000,true,1,.9f};diagnostic::Model model;
 r={42,20,10000,123,diagnostic::Preset::Eruption,true,true};auto p=model.update(r,c,settings,camera);
 assert(p.active&&p.sound&&p.camera&&p.intent.deepCheers&&p.intent.celebration&&p.boost<=50.001f&&p.cue.impact==30);
 uint64_t original=p.until;c.now=11000;p=model.update(r,c,settings,camera);assert(p.until==original&&p.intent.reaction&&!p.intent.continuous);
 c.now=21000;p=model.update(r,c,settings,camera);assert(!p.active&&p.cooling);c.now=25000;assert(!model.update(r,c,settings,camera).active);
 r.token=124;c.now=26000;p=model.update(r,c,settings,camera);assert(p.active);c.phase=3;c.now=26200;p=model.update(r,c,settings,camera);assert(!p.active&&p.cooling);
 c.phase=1;c.now=31000;r.token=125;r.preset=diagnostic::Preset::Lineup;assert(model.update(r,c,settings,camera).active);
 c.match=200;c.now=31100;p=model.update(r,c,settings,camera);assert(!p.active&&!p.fresh(c.match,c.epoch,c.now));
 c.now=36000;r.token=126;c.allowed=false;assert(!model.update(r,c,settings,camera).active);
 c.allowed=true;r.token=127;settings.enabled=0;assert(!model.update(r,c,settings,camera).active);settings.enabled=1;
 r.token=128;settings.cameraShake=false;assert(!model.update(r,c,settings,camera).active);settings.cameraShake=true;
 r.token=129;r.preset=diagnostic::Preset::Roar;r.camera=false;c.now=41000;assert(model.update(r,c,settings,camera).active);
 r.token=130;r.preset=diagnostic::Preset::Stop;c.now=41100;assert(!model.update(r,c,settings,camera).active);
 c.now=46000;r.token=131;r.preset=diagnostic::Preset::Lineup;r.camera=true;assert(model.update(r,c,settings,camera).active);
 settings.cameraShake=false;c.now=46100;assert(!model.update(r,c,settings,camera).active);settings.cameraShake=true;
 c.now=51000;r.token=132;r.deadline=50000;assert(!model.update(r,c,settings,camera).active);r.deadline=0;
 std::cout<<"PASS: diagnostic protocol ownership/freshness, bounded previews, one-shot IDs, unchanged caps, real motion profiles, fades, repeat/stop/disabled and snap/pause/replay/context cancellation.\n";
}
