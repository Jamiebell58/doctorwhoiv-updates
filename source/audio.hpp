// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <algorithm>
namespace dw {
struct Clip {WAVEFORMATEX format{};std::vector<char> pcm;float duration()const{return float(pcm.size())/format.nAvgBytesPerSec;}};
inline Clip loadWav(const std::filesystem::path& path){
 std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Missing audio file");
 f.seekg(0,std::ios::end);auto size=f.tellg();if(size<44||size>16*1024*1024)throw std::runtime_error("Invalid WAV size");f.seekg(0);
 std::vector<char>b(static_cast<size_t>(size));f.read(b.data(),b.size());
 if(!f||std::memcmp(b.data(),"RIFF",4)||std::memcmp(b.data()+8,"WAVE",4))throw std::runtime_error("Invalid WAV");
 Clip out;bool fmt=false;for(size_t p=12;p+8<=b.size();){uint32_t n=0;std::memcpy(&n,b.data()+p+4,4);if(n>b.size()-p-8)throw std::runtime_error("Truncated WAV");
  if(!std::memcmp(b.data()+p,"fmt ",4)&&n>=16){std::memcpy(&out.format,b.data()+p+8,16);fmt=true;}
  if(!std::memcmp(b.data()+p,"data",4))out.pcm.assign(b.begin()+p+8,b.begin()+p+8+n);
  p+=8+size_t(n)+(n&1);
 }
 if(!fmt||out.pcm.empty()||out.format.wFormatTag!=WAVE_FORMAT_PCM||out.format.nChannels!=2||out.format.nSamplesPerSec!=44100||out.format.wBitsPerSample!=16||out.format.nBlockAlign!=4||out.format.nAvgBytesPerSec!=176400||out.pcm.size()%4)throw std::runtime_error("WAV must be PCM stereo 44100 Hz 16-bit");
 return out;
}
class Audio {
 HWAVEOUT dev=nullptr;WAVEHDR hdr{};bool prepared=false;const Clip* current=nullptr;
 public:Clip arrival,departure,open,close;
 void init(const std::filesystem::path& dir){arrival=loadWav(dir/"mat.wav");departure=loadWav(dir/"demat.wav");open=loadWav(dir/"door_open.wav");close=loadWav(dir/"door_close.wav");
  if(waveOutOpen(&dev,WAVE_MAPPER,&arrival.format,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR){dev=nullptr;throw std::runtime_error("Cannot open audio output");}}
 bool available()const{return dev!=nullptr;}
 void stop(){if(dev){waveOutReset(dev);if(prepared)waveOutUnprepareHeader(dev,&hdr,sizeof(hdr));}prepared=false;current=nullptr;hdr={};}
 void play(const Clip& clip){stop();if(!dev)return;hdr.lpData=const_cast<char*>(clip.pcm.data());hdr.dwBufferLength=DWORD(clip.pcm.size());
  if(waveOutPrepareHeader(dev,&hdr,sizeof(hdr))!=MMSYSERR_NOERROR)return;
  prepared=true;
  if(waveOutWrite(dev,&hdr,sizeof(hdr))!=MMSYSERR_NOERROR){stop();return;}current=&clip;}
 float seconds()const{if(!current)return -1;if(hdr.dwFlags&WHDR_DONE)return current->duration();MMTIME t{};t.wType=TIME_BYTES;
  if(waveOutGetPosition(dev,&t,sizeof(t))!=MMSYSERR_NOERROR)return -1;
  if(t.wType==TIME_BYTES)return float(t.u.cb)/current->format.nAvgBytesPerSec;
  if(t.wType==TIME_SAMPLES)return float(t.u.sample)/44100;
  if(t.wType==TIME_MS)return float(t.u.ms)/1000;
  return -1;}
 void pause(bool p){if(dev){if(p)waveOutPause(dev);else waveOutRestart(dev);}}
 void volume(float v){if(dev){DWORD n=DWORD(std::clamp(v,0.f,1.f)*65535);waveOutSetVolume(dev,n|(n<<16));}}
 // No native cleanup in DllMain. The process owns this device until exit.
};
}
