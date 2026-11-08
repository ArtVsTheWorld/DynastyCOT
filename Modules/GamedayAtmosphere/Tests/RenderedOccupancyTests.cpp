#include "../Source/RenderedOccupancy.h"
#include "../Source/AttendanceAudio.h"
#include <cassert>
#include <cstdio>
using namespace attendance;
int main(){
    occupancy::Snapshot nominal{123,7,528,37947,50000};
    occupancy::RenderedBuilder ecu;ecu.add(0x10000,30464,27063);
    auto seats=ecu.finish(123,7,false);
    assert(seats.owns(123,7)&&seats.groups==1&&seats.seats==30464&&seats.original==27063);
    float density=occupancy::fraction(seats,nominal,123,7,.96f);
    assert(std::fabs(density-.8528256f)<.00001f&&density>.85f);
    assert(std::fabs(occupancy::fraction({},nominal,123,7,.96f)-.7285824f)<.00001f);
    assert(occupancy::fraction(seats,nominal,999,7,.96f)==.96f);
    assert(occupancy::fraction(seats,nominal,123,8,.96f)==.96f);
    assert(occupancy::fraction(seats,nominal,123,7,NAN)==0);
    assert(occupancy::fraction(seats,nominal,123,7,2)==seats.peak());
    assert(occupancy::fraction(seats,nominal,123,7,-1)==0);
    // Duplicate/alternate renderers of one geometry are not extra seats.
    ecu.add(0x10000,30464,27063);assert(ecu.finish(123,7,false).groups==1);
    ecu.add(0x20000,10000,4000);auto combined=ecu.finish(123,7,false);
    assert(combined.seats==40464&&combined.original==31063&&combined.groups==2);
    // A partially streamed replacement cannot change a verified full-layout
    // denominator or leak it into a new game/epoch at a recycled address.
    auto held=ecu.finish(123,7,true,seats);assert(held.seats==seats.seats);
    assert(!ecu.finish(123,8,true,seats).valid());
    occupancy::RenderedBuilder partial;partial.add(0x30000,1000,900);
    assert(partial.finish(123,7,false,combined).seats==combined.seats);
    assert(partial.finish(124,7,false,combined).seats==1000);
    occupancy::RenderedBuilder replacement;replacement.add(0x40000,30464,30464);
    auto preserved=replacement.finish(123,7,false,seats);
    assert(preserved.original==27063&&preserved.seats==30464);
    assert(replacement.finish(123,8,false,seats).original==30464); // New epoch can legitimately sell out.
    occupancy::RenderedBuilder sam;sam.add(0x50000,7468,2262);auto sparse=sam.finish(123,7,false);
    occupancy::RenderedBuilder allVisible;allVisible.add(0x60000,7468,7468);
    assert(allVisible.finish(123,7,false,sparse).original==2262); // Halftime Super Sim replacement.
    partial.add(0x30000,1000,899);assert(!partial.finish(124,7,false).valid());
    occupancy::RenderedBuilder invalid;invalid.add(0x10000,60,60);assert(!invalid.finish(123,7,false).valid());
    occupancy::RenderedBuilder empty;empty.add(0x10000,1000,0);auto zero=empty.finish(123,7,false);
    assert(zero.valid()&&occupancy::fraction(zero,nominal,123,7,1)==0);
    // Live absences scale the stored baseline. Historical replay visibility
    // and camera coverage do not enter this snapshot or volume calculation.
    float before=1;
    for(int retained=100;retained>=0;--retained){
        float f=occupancy::fraction(seats,nominal,123,7,retained*.01f);
        assert(f<=before&&f>=0&&balance::crowdBoost(f)<=25&&balance::mix(f,6060).net(1)==balance::mix(f,6060).net(2));before=f;
    }
    for(int nativeCount:{1000,40000,50000}){
        nominal.attendance=nativeCount;assert(occupancy::fraction(seats,nominal,123,7,.96f)==density);
    }
    std::puts("PASS: ECU rendered-density calibration, weighted seating groups, duplicate geometry, streaming/camera stability, fallback, epoch isolation, empty crowds, live scaling and preserved audio caps.");
}
