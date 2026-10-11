// Included inside VulkanPresenter958. Diagnostic-only, no extra submission or wait.
// GPU copies go into distinct immutable per-present offsets; host access only after
// the presenter's existing cleanup device-idle succeeds. Off means no allocation.
static constexpr uint32_t ProbeRois976=5,ProbeSide976=128,ProbeMaxFrames976=120;
static constexpr VkDeviceSize ProbeRoiBytes976=VkDeviceSize(ProbeSide976)*ProbeSide976*4;
static constexpr VkDeviceSize ProbeSideBytes976=ProbeRoiBytes976*ProbeRois976;
static constexpr VkDeviceSize ProbeFrameBytes976=ProbeSideBytes976*2;
struct ProbeFrame976 {uint64_t id=0;double prepareQpc=0;bool generated=false,submitted=false,completed=false,presented=false;};
struct Probe976 {
 bool enabled=false,flushed=false;uint32_t limit=120,count=0;double firstQpc=0;const char*end="session_stop";
 VkBuffer buffer{};VkDeviceMemory memory{};unsigned char*map=nullptr;VkDeviceSize allocation=0;
 VkBufferImageCopy roi[ProbeRois976]{};ProbeFrame976 frames[ProbeMaxFrames976]{};
}probe976;
void configureProbe976(){
 const char*flag=getenv("SMX_PIXEL_PROBE976");probe976.enabled=flag&&strcmp(flag,"1")==0;
 if(!probe976.enabled)return;
 const char*count=getenv("SMX_PIXEL_PROBE976_FRAMES");if(count&&*count){char*end=nullptr;unsigned long n=strtoul(count,&end,10);if(!end||*end||n<1||n>ProbeMaxFrames976)throw std::runtime_error("SMX_PIXEL_PROBE976_FRAMES must be 1..120");probe976.limit=uint32_t(n);}
 if(width<ProbeSide976||height<ProbeSide976)throw std::runtime_error("Pixel probe needs native extent at least128x128");
 const char*marker=getenv("SMX_MARKER967");if(marker&&strcmp(marker,"1")==0)throw std::runtime_error("Pixel probe976 requires marker OFF; preserve default capture affinity");
}
void allocateProbe976(){
 if(!probe976.enabled)return;
 const uint32_t cx[5]={width/2,width/4,3*width/4,width/4,3*width/4};
 const uint32_t cy[5]={height/2,height/4,height/4,3*height/4,3*height/4};
 for(uint32_t i=0;i<ProbeRois976;i++){auto&r=probe976.roi[i];r.bufferOffset=ProbeRoiBytes976*i;r.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};r.imageOffset={int32_t(std::min(width-ProbeSide976,cx[i]>64?cx[i]-64:0u)),int32_t(std::min(height-ProbeSide976,cy[i]>64?cy[i]-64:0u)),0};r.imageExtent={ProbeSide976,ProbeSide976,1};}
 VkBufferCreateInfo bc{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};bc.size=ProbeFrameBytes976*probe976.limit;bc.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT;
 ck(vkCreateBuffer(device,&bc,nullptr,&probe976.buffer),"Probe976 buffer");VkMemoryRequirements rq{};vkGetBufferMemoryRequirements(device,probe976.buffer,&rq);
 if(rq.size>128ull*1024*1024)throw std::runtime_error("Probe976 allocation exceeds128MiB bound");
 VkPhysicalDeviceMemoryProperties mp{};vkGetPhysicalDeviceMemoryProperties(physical,&mp);uint32_t mt=UINT32_MAX;
 for(uint32_t pass=0;pass<2&&mt==UINT32_MAX;pass++)for(uint32_t k=0;k<mp.memoryTypeCount;k++){
  VkMemoryPropertyFlags need=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|(pass==0?VK_MEMORY_PROPERTY_HOST_CACHED_BIT:0);
  if((rq.memoryTypeBits&(1u<<k))&&(mp.memoryTypes[k].propertyFlags&need)==need){mt=k;break;}}
 if(mt==UINT32_MAX)throw std::runtime_error("Probe976 coherent host memory unavailable");
 VkMemoryAllocateInfo ma{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ma.allocationSize=rq.size;ma.memoryTypeIndex=mt;
 ck(vkAllocateMemory(device,&ma,nullptr,&probe976.memory),"Probe976 allocation");probe976.allocation=rq.size;
 ck(vkBindBufferMemory(device,probe976.buffer,probe976.memory,0),"Probe976 bind");void*p=nullptr;ck(vkMapMemory(device,probe976.memory,0,VK_WHOLE_SIZE,0,&p),"Probe976 map");probe976.map=(unsigned char*)p;
 fprintf(stderr,"PIXEL_PROBE976 enabled=1 frames_max=%u deadline_seconds=3 rois=5 roi=128x128 stages=source_then_wsi payload_max=%llu allocation=%llu new_submit=0 new_wait=0 per_frame_io=0 flush=normal_stop_only capture_affinity_unchanged=1 diagnostic_only=1\n",probe976.limit,(unsigned long long)bc.size,(unsigned long long)rq.size);
}
uint32_t beginProbe976(uint64_t id,bool generated,double qpc){
 if(!probe976.enabled||probe976.count>=probe976.limit){if(probe976.enabled)probe976.end="frame_limit";return UINT32_MAX;}
 if(!probe976.firstQpc)probe976.firstQpc=qpc;
 if(qpc-probe976.firstQpc>=3.0){probe976.end="elapsed_limit";return UINT32_MAX;}
 uint32_t i=probe976.count++;probe976.frames[i].id=id;probe976.frames[i].generated=generated;probe976.frames[i].prepareQpc=qpc;return i;
}
void recordProbe976(VkCommandBuffer command,VkImage image,VkImageLayout layout,uint32_t index,bool wsi){
 if(index==UINT32_MAX)return;VkBufferImageCopy r[ProbeRois976];
 for(uint32_t i=0;i<ProbeRois976;i++){r[i]=probe976.roi[i];r[i].bufferOffset+=ProbeFrameBytes976*index+(wsi?ProbeSideBytes976:0);}
 vkCmdCopyImageToBuffer(command,image,layout,probe976.buffer,ProbeRois976,r);
}
static uint64_t hashProbe976(const unsigned char*p,size_t n){uint64_t h=14695981039346656037ull;for(size_t i=0;i<n;i++){h^=p[i];h*=1099511628211ull;}return h;}
void flushProbe976(VkResult idle)noexcept{
 if(!probe976.enabled||probe976.flushed)return;probe976.flushed=true;
 // An unsuccessful idle (including device lost) forbids accessing GPU-written bytes.
 bool safe=idle==VK_SUCCESS&&probe976.map;FILE*json=fopen("app-pixel-probe976.json","wb");FILE*csv=fopen("app-pixel-probe976.csv","wb");FILE*bin=safe?fopen("app-pixel-probe976.bin","wb"):nullptr;
 if(!json||!csv||(safe&&!bin)){fprintf(stderr,"PIXEL_PROBE976 flush_error=file_open no_runtime_retry=1\n");if(json)fclose(json);if(csv)fclose(csv);if(bin)fclose(bin);return;}
 fprintf(csv,"index,present_id,kind,prepare_qpc,submitted,completed,present_returned,roi,x,y,w,h,file_frame_offset,source_relative_offset,wsi_relative_offset,mismatch_bytes,mismatch_pixels,source_fnv1a64,wsi_fnv1a64\n");
 uint64_t fileOffset=0,totalMismatch=0,completed=0;bool io=true;
 for(uint32_t i=0;i<probe976.count;i++){
  auto&f=probe976.frames[i];bool ready=safe&&f.submitted;if(ready){f.completed=true;++completed;}
  int64_t frameOffset=ready?int64_t(fileOffset):-1;
  if(ready){const unsigned char*base=probe976.map+size_t(ProbeFrameBytes976*i);if(fwrite(base,1,size_t(ProbeFrameBytes976),bin)!=size_t(ProbeFrameBytes976))io=false;fileOffset+=ProbeFrameBytes976;}
  for(uint32_t k=0;k<ProbeRois976;k++){
   uint64_t mb=0,mp=0,sh=0,wh=0;if(ready){const unsigned char*s=probe976.map+size_t(ProbeFrameBytes976*i+ProbeRoiBytes976*k);const unsigned char*w=s+size_t(ProbeSideBytes976);for(size_t b=0;b<size_t(ProbeRoiBytes976);b+=4){bool different=false;for(unsigned c=0;c<4;c++)if(s[b+c]!=w[b+c]){++mb;different=true;}mp+=different;}sh=hashProbe976(s,size_t(ProbeRoiBytes976));wh=hashProbe976(w,size_t(ProbeRoiBytes976));totalMismatch+=mb;}
   fprintf(csv,"%u,%llu,%u,%.9f,%u,%u,%u,%u,%d,%d,128,128,%lld,%llu,%llu,%llu,%llu,%016llx,%016llx\n",i,(unsigned long long)f.id,unsigned(f.generated),f.prepareQpc,unsigned(f.submitted),unsigned(ready),unsigned(f.presented),k,probe976.roi[k].imageOffset.x,probe976.roi[k].imageOffset.y,(long long)frameOffset,(unsigned long long)(ProbeRoiBytes976*k),(unsigned long long)(ProbeSideBytes976+ProbeRoiBytes976*k),(unsigned long long)mb,(unsigned long long)mp,(unsigned long long)sh,(unsigned long long)wh);
  }
 }
 if(bin){if(ferror(bin))io=false;if(fclose(bin)!=0)io=false;}if(ferror(csv))io=false;if(fclose(csv)!=0)io=false;
 fprintf(json,"{\n\"revision\":976,\"scope\":\"pre-Present source and copied WSI ROI bytes; NOT composition or physical scanout\",\"status\":\"%s\",\"endReason\":\"%s\",\"idleResult\":%d,\"nativeWidth\":%u,\"nativeHeight\":%u,\"frameLimit\":%u,\"framesRecorded\":%u,\"framesComplete\":%llu,\"roiCount\":5,\"roiWidth\":128,\"roiHeight\":128,\"format\":\"BGRA8_UNORM\",\"frameBytes\":%llu,\"sideBytes\":%llu,\"binaryBytes\":%llu,\"allocationBytes\":%llu,\"mismatchBytes\":%llu,\"extraSubmits\":0,\"extraWaits\":0,\"perFrameFileIO\":false,\"captureAffinityChangedByProbe\":false,\"sourceIdentity\":\"join present_id to app-present/app-job-inputs logs\",\"limits\":[\"Five ROIs do not cover the complete image\",\"Copy and extra TRANSFER_SRC image usage can perturb timing or driver layout\",\"No power or throughput acceptance\",\"No claim about composition or physical panel\"]\n}\n",!safe?"unsafe_idle_no_pixels":io?"complete":"file_write_error",probe976.end,int(idle),width,height,probe976.limit,probe976.count,(unsigned long long)completed,(unsigned long long)ProbeFrameBytes976,(unsigned long long)ProbeSideBytes976,(unsigned long long)fileOffset,(unsigned long long)probe976.allocation,(unsigned long long)totalMismatch);
 if(fclose(json)!=0)io=false;
 fprintf(stderr,"PIXEL_PROBE976 flushed=%u sampled=%u completed=%llu bytes=%llu mismatch_bytes=%llu idle=%d no_display_acceptance=1\n",unsigned(io),probe976.count,(unsigned long long)completed,(unsigned long long)fileOffset,(unsigned long long)totalMismatch,int(idle));
}
void destroyProbe976()noexcept{if(probe976.map){vkUnmapMemory(device,probe976.memory);probe976.map=nullptr;}if(probe976.buffer){vkDestroyBuffer(device,probe976.buffer,nullptr);probe976.buffer={};}if(probe976.memory){vkFreeMemory(device,probe976.memory,nullptr);probe976.memory={};}}
