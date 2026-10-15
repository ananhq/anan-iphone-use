// SPDX-License-Identifier: Apache-2.0
// Defined before compact-cache implementation; synthetic native graphics only.
#include "avatar_native.hpp"
#include <cassert>
#include <climits>
#include <cstdio>
int main(){
    avatar_native_set_activity({});
    M5.Display.startWrite();
    M5.Display.fillTriangle(-32768,-32768,32767,0,0,32767,0xffff);
    M5.Display.fillRect(32767,-32768,1,1,0xffff);
    M5.Display.endWrite();assert(!avatar_native_overflow()&&avatar_native_command_count()==2);
    const int outside[]={-32769,32768,INT_MAX,INT_MIN};
    for(int x:outside){
        M5.Display.startWrite();M5.Display.fillRect(x,0,1,1,0xffff);M5.Display.endWrite();
        avatar_pose frame=avatar_native_frame();
        assert(avatar_native_overflow()&&!avatar_frame_ready(&frame));
    }
    M5.Display.startWrite();
    for(unsigned i=0;i<160;i++)M5.Display.fillRect(i,i,1,1,0xffff);
    M5.Display.endWrite();assert(!avatar_native_overflow()&&avatar_native_command_count()==160);
    M5.Display.startWrite();
    for(unsigned i=0;i<161;i++)M5.Display.fillRect(i,i,1,1,0xffff);
    M5.Display.endWrite();assert(avatar_native_overflow());
    M5.Display.startWrite();M5.Display.fillRect(100,100,2,2,0x1234);M5.Display.endWrite();
    assert(!avatar_native_overflow());
    puts("Compact cache: signed coordinate boundaries, out-of-range rejection, retained 160-command limit and recovery passed");
}
