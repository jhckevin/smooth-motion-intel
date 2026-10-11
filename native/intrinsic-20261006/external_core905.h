static int phaseEnd851(unsigned i){static const unsigned ends[13]={1,17,58,99,140,181,182,210,238,266,294,298,299};for(int j=0;j<13;j++)if(i==ends[j])return j;return -1;}
typedef struct {const char*first;const char*last;const char*shader;} FusionSpec842;
static const FusionSpec842 fusionSpecs842[]={
{"parallel837-0-0","parallel837-0-1","shaders/sequence841_sg32_o_0.spv"},
{"parallel837-0-2","parallel837-0-3","shaders/sequence841_sg32_o_1.spv"},
{"parallel837-0-6","parallel837-0-7","shaders/sequence841_sg32_o_2.spv"},
{"parallel837-0-8","parallel837-0-9","shaders/sequence841_sg32_o_3.spv"},
{"parallel837-0-10","parallel837-0-11","shaders/sequence841_sg32_o_4.spv"},
{"parallel837-1-0","parallel837-1-1","shaders/const903_sg32_o_d07ae1105a44.spv"},
{"parallel837-1-2","parallel837-1-3","shaders/const903_sg32_o_f3bcf74da4ab.spv"},
{"parallel837-1-4","parallel837-1-5","shaders/const903_sg32_o_10cfafcc6d1c.spv"},
{"parallel837-1-6","parallel837-1-7","shaders/const903_sg32_o_6c10873ba8d7.spv"},
{"parallel837-1-8","parallel837-1-9","shaders/const903_sg32_o_3cec81996bde.spv"},
{"parallel837-2-0","parallel837-2-1","shaders/sequence841_sg32_o_10.spv"},
{"parallel837-2-2","parallel837-2-3","shaders/sequence841_sg32_o_11.spv"},
{"parallel837-2-6","parallel837-2-7","shaders/sequence841_sg32_o_12.spv"},
{"parallel837-2-8","parallel837-2-9","shaders/sequence841_sg32_o_13.spv"},
{"parallel837-2-10","parallel837-2-11","shaders/sequence841_sg32_o_14.spv"},
{"parallel837-3-0","parallel837-3-1","shaders/const903_sg32_o_39eea302d72d.spv"},
{"parallel837-3-2","parallel837-3-3","shaders/const903_sg32_o_36368ad0bffb.spv"},
{"parallel837-3-4","parallel837-3-5","shaders/const903_sg32_o_ee30c00b6683.spv"},
{"parallel837-3-6","parallel837-3-7","shaders/const903_sg32_o_e01075c6d849.spv"},
{"parallel837-3-8","parallel837-3-9","shaders/const903_sg32_o_9e1843c2a45d.spv"},
{"parallel837-4-0","parallel837-4-1","shaders/sequence841_sg32_o_20.spv"},
{"parallel837-4-2","parallel837-4-3","shaders/sequence841_sg32_o_21.spv"},
{"parallel837-4-6","parallel837-4-7","shaders/sequence841_sg32_o_22.spv"},
{"parallel837-4-8","parallel837-4-9","shaders/sequence841_sg32_o_23.spv"},
{"parallel837-4-10","parallel837-4-11","shaders/sequence841_sg32_o_24.spv"},
{"parallel837-5-0","parallel837-5-1","shaders/const903_sg32_o_8fba62539b05.spv"},
{"parallel837-5-2","parallel837-5-3","shaders/const903_sg32_o_736a7a9cc7b0.spv"},
{"parallel837-5-4","parallel837-5-5","shaders/const903_sg32_o_9f7887ff1e96.spv"},
{"parallel837-5-6","parallel837-5-7","shaders/const903_sg32_o_6b56408187cb.spv"},
{"parallel837-5-8","parallel837-5-9","shaders/const903_sg32_o_6efbbeba5344.spv"},
{"parallel837-6-0","parallel837-6-1","shaders/sequence841_sg32_o_30.spv"},
{"parallel837-6-2","parallel837-6-3","shaders/sequence841_sg32_o_31.spv"},
{"parallel837-6-6","parallel837-6-7","shaders/sequence841_sg32_o_32.spv"},
{"parallel837-6-8","parallel837-6-9","shaders/sequence841_sg32_o_33.spv"},
{"parallel837-6-10","parallel837-6-11","shaders/sequence841_sg32_o_34.spv"},
{"parallel837-7-0","parallel837-7-1","shaders/const903_sg32_o_b9834804a3ed.spv"},
{"parallel837-7-2","parallel837-7-3","shaders/const903_sg32_o_c0b5c403d403.spv"},
{"parallel837-7-4","parallel837-7-5","shaders/const903_sg32_o_31ad4cb75eba.spv"},
{"parallel837-7-6","parallel837-7-7","shaders/const903_sg32_o_682932ea9b3d.spv"},
{"parallel837-7-8","parallel837-7-9","shaders/const903_sg32_o_517b2904994f.spv"},
};
typedef struct ExternalCoreState {
 GraphBuffer*graphBuffers;GraphNode*nodes;GraphPipeline*pipelines;uint32_t nn,qi;int fine;
 VkDescriptorSetLayout buffers;VkPipelineCache cache;
 VkQueryPool asyncQueries905;VkDescriptorSet imageSet;VkDescriptorSetLayout images;VkDescriptorPool descriptors;VkPipelineLayout layout;
 VkPipeline fused842[40];VkShaderModule fusedModules842[40];VkCommandBuffer baseline839;VkPipeline expWarp860;VkShaderModule expModule860;VkPipeline first,last;VkSampler sampler;VkShaderModule firstModule,lastModule;VkCommandPool pool;
} ExternalCoreState;
static ExternalCoreState directCoreStates[2];static unsigned directCoreIndex;
#define directCoreState directCoreStates[directCoreIndex]
static VkPipeline external_pipeline(const char*path,VkShaderModule*module,unsigned subgroup){
 void*code;size_t size=read_file(path,&code);
 VkShaderModuleCreateInfo sci={VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};sci.codeSize=size;sci.pCode=code;
 CHECK(vkCreateShaderModule(device,&sci,NULL,module));free(code);
 VkPipelineShaderStageRequiredSubgroupSizeCreateInfo sgi={VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO};sgi.requiredSubgroupSize=subgroup;
 VkComputePipelineCreateInfo pci={VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};pci.stage.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 pci.stage.pNext=&sgi;pci.stage.flags=VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;pci.stage.stage=VK_SHADER_STAGE_COMPUTE_BIT;pci.stage.module=*module;pci.stage.pName="main";pci.layout=directCoreState.layout;
 VkPipeline p;CHECK(vkCreateComputePipelines(device,directCoreState.cache,1,&pci,NULL,&p));return p;
}
static int bind_external_images(const VkImageView*views){
 if(directCoreIndex>1||directCoreState.pool)return -1;
 VkDescriptorSetLayoutBinding bindings[3]={0};
 for(uint32_t i=0;i<3;i++){bindings[i].binding=i;bindings[i].descriptorType=i<2?VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;bindings[i].descriptorCount=1;bindings[i].stageFlags=VK_SHADER_STAGE_COMPUTE_BIT;}
 VkDescriptorSetLayoutCreateInfo li={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};li.bindingCount=3;li.pBindings=bindings;CHECK(vkCreateDescriptorSetLayout(device,&li,NULL,&directCoreState.images));
 VkDescriptorSetLayout layouts[2]={directCoreState.buffers,directCoreState.images};
 VkPushConstantRange push={VK_SHADER_STAGE_COMPUTE_BIT,0,64};VkPipelineLayoutCreateInfo pi={VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};pi.setLayoutCount=2;pi.pSetLayouts=layouts;pi.pushConstantRangeCount=1;pi.pPushConstantRanges=&push;CHECK(vkCreatePipelineLayout(device,&pi,NULL,&directCoreState.layout));
 VkDescriptorPoolSize ps[2]={{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2},{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,1}};VkDescriptorPoolCreateInfo dp={VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};dp.maxSets=1;dp.poolSizeCount=2;dp.pPoolSizes=ps;CHECK(vkCreateDescriptorPool(device,&dp,NULL,&directCoreState.descriptors));
 VkDescriptorSetAllocateInfo da={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};da.descriptorPool=directCoreState.descriptors;da.descriptorSetCount=1;da.pSetLayouts=&directCoreState.images;VkDescriptorSet set;CHECK(vkAllocateDescriptorSets(device,&da,&set));directCoreState.imageSet=set;
 VkSamplerCreateInfo si={VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};si.magFilter=si.minFilter=VK_FILTER_LINEAR;si.mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST;si.addressModeU=si.addressModeV=si.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;si.maxLod=0;CHECK(vkCreateSampler(device,&si,NULL,&directCoreState.sampler));
 VkDescriptorImageInfo ii[3]={0};VkWriteDescriptorSet writes[3]={0};
 for(uint32_t i=0;i<3;i++){ii[i].imageView=views[i];if(i<2)ii[i].sampler=directCoreState.sampler;ii[i].imageLayout=VK_IMAGE_LAYOUT_GENERAL;writes[i].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;writes[i].dstSet=set;writes[i].dstBinding=i;writes[i].descriptorCount=1;writes[i].descriptorType=bindings[i].descriptorType;writes[i].pImageInfo=&ii[i];}
 vkUpdateDescriptorSets(device,3,writes,0,NULL);
 directCoreState.first=external_pipeline(getenv("SMX_NATIVE_DOWNSCALE")?"shaders/native830-down.spv":"shaders/sampled1080-down.spv",&directCoreState.firstModule,16);const char*warpFlavor=getenv("SMX_WARP_FLAVOR");const char*warpPath="shaders/sampled1080-warp.spv";
 if(warpFlavor&&!strcmp(warpFlavor,"native-final"))warpPath="shaders/sampled1080-warp-bgra.spv";
 if(warpFlavor&&!strcmp(warpFlavor,"native-flow"))warpPath="shaders/native830-warp.spv";
 directCoreState.last=external_pipeline(warpPath,&directCoreState.lastModule,16);int ab860=getenv("SMX_AB_WARP860")!=NULL,exp860=ab860||getenv("SMX_EXP_WARP860")!=NULL;if(exp860)directCoreState.expWarp860=external_pipeline("shaders/native860-warp-exp.spv",&directCoreState.expModule860,16);
 VkCommandPoolCreateInfo cp={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};cp.queueFamilyIndex=directCoreState.qi;CHECK(vkCreateCommandPool(device,&cp,NULL,&directCoreState.pool));
 int fusion842=getenv("SMX_AB_FUSION842")!=NULL||getenv("SMX_FUSE842")!=NULL;if(fusion842&&directCoreState.fine){fprintf(stderr,"Fusion A/B is aggregate-only; disable fine profiling\n");return -1;}
 if(fusion842)for(unsigned j=0;j<40;j++)directCoreState.fused842[j]=external_pipeline(fusionSpecs842[j].shader,&directCoreState.fusedModules842[j],32);
 unsigned variants839=(getenv("SMX_AB_BARRIERS839")||getenv("SMX_AB_FUSION842")||ab860)?2:1;for(unsigned variant839=0;variant839<variants839;variant839++){forceBarriers839=variant839==1&&!fusion842;unsigned merged842=0;
 VkCommandBufferAllocateInfo ca={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=directCoreState.pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;VkCommandBuffer cmd;CHECK(vkAllocateCommandBuffers(device,&ca,&cmd));
 VkCommandBufferBeginInfo begin={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CHECK(vkBeginCommandBuffer(cmd,&begin));
 
 if(!directCoreState.asyncQueries905){directCoreState.asyncQueries905=interopContext.queries;
 if(directCoreIndex==1&&getenv("SMX_PIPELINE905")){VkQueryPoolCreateInfo q={VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};q.queryType=VK_QUERY_TYPE_TIMESTAMP;q.queryCount=interopContext.queryCount;CHECK(vkCreateQueryPool(device,&q,NULL,&directCoreState.asyncQueries905));}}
 vkCmdResetQueryPool(cmd,directCoreState.asyncQueries905,0,interopContext.queryCount);
 // The same queue shares model intermediates. Explicit inter-job RAW/WAR/WAW dependency.
 if(getenv("SMX_PIPELINE905")){VkMemoryBarrier job={VK_STRUCTURE_TYPE_MEMORY_BARRIER};job.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT|VK_ACCESS_SHADER_READ_BIT;job.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&job,0,NULL,0,NULL);}

 VkMemoryBarrier mb={VK_STRUCTURE_TYPE_MEMORY_BARRIER};
 Dependency395 dep395={0};int useDependency395=getenv("SMX_DEPENDENCY_BARRIERS")&&!getenv("SMX_STRICT_BARRIERS");
 for(uint32_t i=0;i<directCoreState.nn;i++){
  GraphNode*n=&directCoreState.nodes[i];int merge842=-1;if(fusion842&&(variant839==0||ab860)&&i+1<directCoreState.nn)for(unsigned j=0;j<40;j++)if(!strcmp(n->name,fusionSpecs842[j].first)&&!strcmp(directCoreState.nodes[i+1].name,fusionSpecs842[j].last)){merge842=(int)j;break;}
  if(useDependency395)dependency395(cmd,&dep395,n,&directCoreState.pipelines[n->pipeline],directCoreState.graphBuffers);
  else if(i&&!independent(&directCoreState.nodes[i-1],n)){mb.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&mb,0,NULL,0,NULL);}
  if(directCoreState.fine||i==0)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,directCoreState.asyncQueries905,directCoreState.fine?i*2:0);
  int first=!strcmp(n->name,"downscale-first"),last=!strcmp(n->name,"warp");
  VkPipeline p=merge842>=0?directCoreState.fused842[merge842]:first?directCoreState.first:last?directCoreState.last:directCoreState.pipelines[n->pipeline].pipeline;
  if(last&&exp860&&(!ab860||variant839==0))p=directCoreState.expWarp860;
  vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,p);
  if(i==0||directCoreState.nodes[i-1].set!=n->set)vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,directCoreState.layout,0,1,&n->set,0,NULL);
  if(first||last)vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,directCoreState.layout,1,1,&set,0,NULL);
  uint32_t nativeCfg830[16];memcpy(nativeCfg830,n->cfg,64);if(first||last){nativeCfg830[0]=interopContext.width;nativeCfg830[1]=interopContext.height;}
  vkCmdPushConstants(cmd,directCoreState.layout,VK_SHADER_STAGE_COMPUTE_BIT,0,64,nativeCfg830);vkCmdDispatch(cmd,last?(interopContext.width*interopContext.height+255)/256:n->dispatch[0],n->dispatch[1],n->dispatch[2]);
  if(getenv("SMX_BLOCK851")){int e=phaseEnd851(i);if(e>=0)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,directCoreState.asyncQueries905,e+1);}else if(directCoreState.fine||i+1==directCoreState.nn)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,directCoreState.asyncQueries905,directCoreState.fine?i*2+1:1);
  if(merge842>=0){dep395.previous837=&directCoreState.nodes[++i];++merged842;}
 }
 fprintf(stderr,"FUSION842 variant=%u dispatches=%u fused_pairs=%u\n",variant839,directCoreState.nn-merged842,merged842);
 fprintf(stderr,"DIRECT DEPENDENCY837 enabled=%d calls=%u dependencies=%u proven_skips=%u\n",useDependency395,dep395.calls,dep395.dependencies,dep395.skipped837);CHECK(vkEndCommandBuffer(cmd));if(variant839==0){interopContext.core=cmd;interopContext.alternates[directCoreIndex]=cmd;}else directCoreState.baseline839=cmd;
 }forceBarriers839=0;++directCoreIndex;return 0;
}
static void cleanup_external(void){for(directCoreIndex=0;directCoreIndex<2;directCoreIndex++){
 if(!directCoreState.pool)continue;
 for(unsigned j=0;j<40;j++){if(directCoreState.fused842[j])vkDestroyPipeline(device,directCoreState.fused842[j],NULL);if(directCoreState.fusedModules842[j])vkDestroyShaderModule(device,directCoreState.fusedModules842[j],NULL);}
 if(directCoreState.expWarp860)vkDestroyPipeline(device,directCoreState.expWarp860,NULL);if(directCoreState.expModule860)vkDestroyShaderModule(device,directCoreState.expModule860,NULL);
 if(directCoreState.asyncQueries905&&directCoreState.asyncQueries905!=interopContext.queries)vkDestroyQueryPool(device,directCoreState.asyncQueries905,NULL);
 vkDestroyCommandPool(device,directCoreState.pool,NULL);vkDestroyPipeline(device,directCoreState.first,NULL);vkDestroyPipeline(device,directCoreState.last,NULL);
 vkDestroyShaderModule(device,directCoreState.firstModule,NULL);vkDestroyShaderModule(device,directCoreState.lastModule,NULL);
 vkDestroySampler(device,directCoreState.sampler,NULL);vkDestroyDescriptorPool(device,directCoreState.descriptors,NULL);vkDestroyPipelineLayout(device,directCoreState.layout,NULL);vkDestroyDescriptorSetLayout(device,directCoreState.images,NULL);
 memset(&directCoreState,0,sizeof(directCoreState));
}}

int smx_graph_update_images(unsigned index,const VkImageView*views){
 if(index>=directCoreIndex||!directCoreStates[index].imageSet)return -1;
 ExternalCoreState*d=&directCoreStates[index];VkDescriptorImageInfo ii[3]={0};VkWriteDescriptorSet writes[3]={0};
 for(unsigned i=0;i<3;i++){ii[i].imageView=views[i];if(i<2)ii[i].sampler=d->sampler;ii[i].imageLayout=VK_IMAGE_LAYOUT_GENERAL;
 writes[i].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;writes[i].dstSet=d->imageSet;writes[i].dstBinding=i;writes[i].descriptorCount=1;
 writes[i].descriptorType=i<2?VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;writes[i].pImageInfo=&ii[i];}
 vkUpdateDescriptorSets(device,3,writes,0,NULL);return 0;
}

VkCommandBuffer smx_graph_core839(unsigned index,unsigned baseline){if(index>=directCoreIndex)return VK_NULL_HANDLE;return baseline&&directCoreStates[index].baseline839?directCoreStates[index].baseline839:interopContext.alternates[index];}

VkQueryPool smx_graph_queries905(unsigned i){return i<directCoreIndex?directCoreStates[i].asyncQueries905:VK_NULL_HANDLE;}
