// Include inside GpuGraphBridge. Opt-in diagnostic; never a performance result.
struct PixelJob967 {
 VkBuffer buffer{};VkDeviceMemory memory{};unsigned char*mapped=nullptr;
 uint64_t id=0,first=0,second=0,flowOffset=0,flowBytes=0;VkBuffer flow{};VkImage output{};bool active=false;
};
struct PixelResult967 {uint64_t id=0,first=0,second=0;VkImage image{};std::vector<unsigned char> output,wsi;bool model=false,display=false;};
PixelJob967 pixelJobs967[2];std::mutex pixelMutex967;std::vector<PixelResult967> pixelResults967;
uint64_t pixelCount967=0;bool pixelAudit967()const{const char*x=getenv("SMX_PIXEL_AUDIT967");return x&&strcmp(x,"1")==0;}
size_t pixelBytes967()const{return size_t(g->width)*g->height*4;}
std::string pixelPath967(uint64_t id,const char*stage,const char*ext)const{
 const char*dir=getenv("SMX_BENCH_DIR");if(!dir||!*dir)throw std::runtime_error("Pixel diagnostic requires workspace BENCH_DIR");
 return std::string(dir)+"\\pixel967-job"+std::to_string(id)+"-"+stage+ext;
}
void pixelWrite967(uint64_t id,const char*stage,const void*data,size_t bytes,const char*ext=".bgra"){
 const auto path=pixelPath967(id,stage,ext);FILE*f=fopen(path.c_str(),"wb");if(!f)throw std::runtime_error("Pixel diagnostic file open failed");bool ok=fwrite(data,1,bytes,f)==bytes;ok=fclose(f)==0&&ok;if(!ok)throw std::runtime_error("Pixel diagnostic file write failed");
}
void pixelMeta967(PixelResult967&r){
 if(!r.model||!r.display)return;
 const char*marker969=getenv("SMX_MARKER967");const bool excludeMarker969=marker969&&strcmp(marker969,"1")==0;
 size_t bad=0;unsigned maxdiff=0;for(size_t i=0;i<r.output.size();++i){size_t pixel=i/4;if(excludeMarker969&&pixel%g->width>=8&&pixel%g->width<136&&pixel/g->width>=g->height-8&&pixel/g->width<g->height-4)continue;unsigned a=r.output[i],b=r.wsi[i];if(a!=b)++bad;maxdiff=std::max(maxdiff,a>b?a-b:b-a);}
 char text[1536];int n=snprintf(text,sizeof(text),"{\"diagnostic_only\":true,\"performance_eligible\":false,\"job\":%llu,\"first_source_id\":%llu,\"second_source_id\":%llu,\"width\":%u,\"height\":%u,\"row_pitch\":%u,\"format\":\"BGRA8_UNORM\",\"orientation\":\"top-down\",\"bytes_per_image\":%llu,\"model_wsi_byte_mismatches\":%llu,\"model_wsi_max_channel_diff\":%u,\"actual_scanout_unverified\":true,\"limitations\":\"Readback changes timing; comparison excludes optional marker x8-135 yheight-8..height-5; UNORM images cannot expose NaN/Inf; private-input dump is after capture and interop; WSI readback is before scanout\"}\n",(unsigned long long)r.id,(unsigned long long)r.first,(unsigned long long)r.second,g->width,g->height,g->width*4,(unsigned long long)pixelBytes967(),(unsigned long long)bad,maxdiff);
 if(n<0||size_t(n)>=sizeof(text))throw std::runtime_error("Pixel diagnostic metadata overflow");pixelWrite967(r.id,"meta",text,size_t(n),".json");
 fprintf(stderr,"PIXEL967 job=%llu model_wsi_byte_mismatches=%llu max=%u diagnostic_only=1 scanout_unverified=1\n",(unsigned long long)r.id,(unsigned long long)bad,maxdiff);fflush(stderr);
 if(bad)throw std::runtime_error("Pixel diagnostic model-to-WSI mismatch; stop, no retry");
 r.output.clear();r.output.shrink_to_fit();r.wsi.clear();r.wsi.shrink_to_fit();
}
void pixelBegin967(unsigned slot){
 auto&j=pixelJobs967[slot];j.active=false;if(!pixelAudit967()||pixelCount967>=4)return;
 if(uint64_t(g->width)*g->height>2073600)throw std::runtime_error("Pixel diagnostic bounded to at most 2073600 native pixels");
 if(smx_pixel_flow967(slot,&j.flow,&j.flowOffset,&j.flowBytes)||j.flowBytes!=663552)throw std::runtime_error("Pixel flow descriptor mismatch");
 if(!j.buffer){VkBufferCreateInfo bc{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};bc.size=VkDeviceSize(pixelBytes967())*3+j.flowBytes;bc.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT;CK(vkCreateBuffer(device,&bc,nullptr,&j.buffer));VkMemoryRequirements req{};vkGetBufferMemoryRequirements(device,j.buffer,&req);VkMemoryAllocateInfo ma{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ma.allocationSize=req.size;ma.memoryTypeIndex=typeFor(g->memory,req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);CK(vkAllocateMemory(device,&ma,nullptr,&j.memory));CK(vkBindBufferMemory(device,j.buffer,j.memory,0));void*p=nullptr;CK(vkMapMemory(device,j.memory,0,VK_WHOLE_SIZE,0,&p));j.mapped=(unsigned char*)p;}
 j.id=++pixelCount967;j.first=pairFirst943;j.second=pairSecond943;j.output=directOutput->image;j.active=true;
 {std::lock_guard lock(pixelMutex967);PixelResult967 r;r.id=j.id;r.first=j.first;r.second=j.second;r.image=j.output;pixelResults967.push_back(std::move(r));}
 fprintf(stderr,"PIXEL967 begin job=%llu slot=%u source_ids=%llu,%llu native=%ux%u diagnostic_only=1\n",(unsigned long long)j.id,slot,(unsigned long long)j.first,(unsigned long long)j.second,g->width,g->height);
}
void pixelCopy967(VkCommandBuffer cmd,unsigned slot,VkImage image,unsigned plane,VkAccessFlags before,VkPipelineStageFlags beforeStage,bool restore){
 auto&j=pixelJobs967[slot];if(!j.active)return;
 VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=im.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;im.srcAccessMask=before;im.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.image=image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};vkCmdPipelineBarrier(cmd,beforeStage,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&im);
 VkBufferImageCopy copy{};copy.bufferOffset=VkDeviceSize(pixelBytes967())*plane;copy.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};copy.imageExtent={g->width,g->height,1};vkCmdCopyImageToBuffer(cmd,image,VK_IMAGE_LAYOUT_GENERAL,j.buffer,1,&copy);
 if(restore){im.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&im);}
}
void pixelSnapshot967(unsigned slot){
 auto&j=pixelJobs967[slot];if(!j.active)return;auto cmd=snapshots929[slot].command;
 auto first=reuseFirst943?snapshots929[previousSlot943].images[1].image:snapshots929[slot].images[0].image;
 pixelCopy967(cmd,slot,first,0,VK_ACCESS_TRANSFER_WRITE_BIT|VK_ACCESS_SHADER_READ_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT|VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,true);
 pixelCopy967(cmd,slot,snapshots929[slot].images[1].image,1,VK_ACCESS_TRANSFER_WRITE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,true);
}
void pixelOutput967(unsigned slot,VkCommandBuffer cmd){
 if(!pixelJobs967[slot].active)return;
 pixelCopy967(cmd,slot,directOutput->image,2,VK_ACCESS_SHADER_WRITE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,false);
 auto&j=pixelJobs967[slot];VkMemoryBarrier flowBefore{VK_STRUCTURE_TYPE_MEMORY_BARRIER};flowBefore.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;flowBefore.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,1,&flowBefore,0,nullptr,0,nullptr);
 VkBufferCopy copy{j.flowOffset,VkDeviceSize(pixelBytes967())*3,j.flowBytes};vkCmdCopyBuffer(cmd,j.flow,j.buffer,1,&copy);
 VkMemoryBarrier flowAfter{VK_STRUCTURE_TYPE_MEMORY_BARRIER};flowAfter.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;flowAfter.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&flowAfter,0,nullptr,0,nullptr);
 VkMemoryBarrier host{VK_STRUCTURE_TYPE_MEMORY_BARRIER};host.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;host.dstAccessMask=VK_ACCESS_HOST_READ_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&host,0,nullptr,0,nullptr);
}
void pixelFinish967(unsigned slot){
 auto&j=pixelJobs967[slot];if(!j.active)return;const auto bytes=pixelBytes967();pixelWrite967(j.id,"first-private-input",j.mapped,bytes);pixelWrite967(j.id,"second-private-input",j.mapped+bytes,bytes);pixelWrite967(j.id,"model-output",j.mapped+2*bytes,bytes);pixelWrite967(j.id,"flow-logits-192x108x16",j.mapped+3*bytes,size_t(j.flowBytes),".fp16");
 {std::lock_guard lock(pixelMutex967);auto it=std::find_if(pixelResults967.begin(),pixelResults967.end(),[&](auto&r){return r.id==j.id;});if(it==pixelResults967.end())throw std::runtime_error("Pixel job metadata missing");it->output.assign(j.mapped+2*bytes,j.mapped+3*bytes);it->model=true;pixelMeta967(*it);}
 j.active=false;
}
uint64_t pixelWsiJob967(VkImage image){if(!pixelAudit967())return 0;std::lock_guard lock(pixelMutex967);for(auto&r:pixelResults967)if(r.image==image&&!r.display)return r.id;return 0;}
void pixelWsiFinish967(uint64_t id){
 if(!id)return;auto*p=presenter958->verifiedPixels();pixelWrite967(id,"wsi-before-present",p,pixelBytes967());
 std::lock_guard lock(pixelMutex967);auto it=std::find_if(pixelResults967.begin(),pixelResults967.end(),[&](auto&r){return r.id==id;});if(it==pixelResults967.end())throw std::runtime_error("Pixel WSI metadata missing");it->wsi.assign(p,p+pixelBytes967());it->display=true;pixelMeta967(*it);
}
void pixelCleanup967(){for(auto&j:pixelJobs967){if(j.mapped)vkUnmapMemory(device,j.memory);if(j.buffer)vkDestroyBuffer(device,j.buffer,nullptr);if(j.memory)vkFreeMemory(device,j.memory,nullptr);j={};}}
