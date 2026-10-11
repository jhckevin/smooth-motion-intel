/* Diagnostic-only accessor. Caller copies while the matching model queue job owns the data. */
int smx_pixel_flow967(unsigned index,VkBuffer*buffer,uint64_t*offset,uint64_t*bytes){
 if(index>=directCoreIndex||!buffer||!offset||!bytes)return -1;
 ExternalCoreState*d=&directCoreStates[index];
 for(unsigned i=0;i<d->nn;i++){GraphNode*n=&d->nodes[i];if(strcmp(n->name,"warp"))continue;
  uint64_t count=(uint64_t)n->cfg[2]*n->cfg[3]*16u*2u;
  if(count!=663552u||count>n->range[1])return -2;
  *buffer=d->graphBuffers[n->bi[1]].gpu.buffer;*offset=n->off[1];*bytes=count;return 0;
 }return -3;
}
