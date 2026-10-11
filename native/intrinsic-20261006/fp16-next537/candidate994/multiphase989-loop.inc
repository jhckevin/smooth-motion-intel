   if(frameMultiplier989>2){
    // Each non-midpoint is a real recursive inference. No repeated Present is counted as generated.
    // The unchanged two-slot 2x path below remains the production default.
    if(!async905||wsi958)throw std::runtime_error("Experimental multi-frame mode requires native DXGI asynchronous bridge");
    drain905(true);
    const auto plan989=smx989::midpointPlan(frameMultiplier989);
    std::array<std::shared_ptr<GpuFrames>,9> frames989{};
    frames989[0]=pairFirstOriginal;frames989[frameMultiplier989]=latestOriginal;
    bool poolReady989=true;
    for(unsigned index989=1;index989<frameMultiplier989;++index989){
     for(auto& candidate989:texturePool)if(candidate989.use_count()==1){frames989[index989]=candidate989;break;}
     if(!frames989[index989]){poolReady989=false;break;}
    }
    if(!poolReady989){++poolSkipCount965;latest.Close();lastTime=now;lastSequence=latestSequence;continue;}
    std::array<uint64_t,9> tickets989{};std::array<double,9> dues989{};
    const double batchStart989=timelineNow();
    // Experimental mode is one bounded batch at a time. Reserve future slots, do not reuse
    // the 2x pipeline predictor (which estimates source period, not recursive inference cost).
    // Late completion remains visible and fails timing acceptance; no due is moved.
    bool admitted989=true;
    {
     std::lock_guard lock(queueMutex);
     for(const auto& step989:plan989){
      auto due989=timeline.pairDueFraction989(lastTime,now,step989.output,frameMultiplier989);
      if(!due989){admitted989=false;break;}dues989[step989.output]=*due989;
      tickets989[step989.output]=timeline.reserveFraction989(lastTime,now,step989.output,frameMultiplier989,batchStart989,0.0,16);
      if(!tickets989[step989.output]){admitted989=false;break;}
     }
     if(!admitted989)for(auto& ticket989:tickets989)if(ticket989){timeline.cancelBeforeComplete(ticket989);ticket989=0;}
    }
    if(!admitted989){latest.Close();lastTime=now;lastSequence=latestSequence;continue;}
    CallbackCleanup groupCleanup989{[&](){std::lock_guard lock(queueMutex);for(auto& ticket989:tickets989)if(ticket989){timeline.cancelBeforeComplete(ticket989);ticket989=0;}SetEvent(presentationChanged.value);}};
    SetEvent(presentationChanged.value);
    for(const auto& step989:plan989){
     if(stopFlag||capturePaused989())break;
     auto source989=[&](unsigned index989)->ID3D11Texture2D* {return (index989==0||index989==frameMultiplier989)?frames989[index989]->first.Get():frames989[index989]->middle.Get();};
     bridge.setDirectPair(source989(step989.left),source989(step989.right));
     bridge.setDirectOutput(frames989[step989.output]->middle.Get());
     // Generated textures do not carry capture sequence keys: explicitly disable adjacent-source reuse.
     bridge.setPairKeys943(0,0);
     unsigned slot989=nextSlot905;nextSlot905^=1u;const double runBegin989=timelineNow();
     if(stopFlag||capturePaused989())break; // latest foreground check before submission; uncomputed reservations stay under groupCleanup.
     bridge.submit905(slot989);hudSession988->metrics.setInflight(1);
     const double gpu989=bridge.finish905(slot989),finished989=timelineNow(),wall989=(finished989-runBegin989)*1000;
     ++inferReturned700;++generated;gpuSum+=gpu989;runWallSum+=wall989;
     hudSession988->metrics.recordGeneration(gpu989,wall989,bridge.queueWaitMs988);
     deadlineGuard664.completePipeline(bridge.submitTime930(slot989),bridge.fenceTime930(slot989),wall989,finished989>dues989[step989.output],finished989);
     {
      std::lock_guard lock(queueMutex);
      auto completion989=timeline.completeWithStatus980(tickets989[step989.output],frames989[step989.output],finished989);
      tickets989[step989.output]=0;
      hudSession988->metrics.setLateCounts(timeline.counts.lateCompleted,timeline.counts.lateSubmitted);
      if(!completion989.onTime&&!completion989.lifecycleCancelled){latenessFailure700=true;performanceWarning775=true;}
     }
     hudSession988->metrics.setInflight(0);SetEvent(presentationChanged.value);queueChanged.notify_one();
     smx_log_fprintf989(stderr,"MULTIPHASE989 multiplier=%u fraction=%u/%u recursive_rank=%u gpu_ms=%.6f wall_ms=%.6f repeated_frame=0\n",frameMultiplier989,step989.output,frameMultiplier989,step989.rank,gpu989,wall989);
    }
    latest.Close();if(heldFirst){heldFirst.Close();heldFirst=nullptr;}lastTime=now;lastSequence=latestSequence;
    if(selftest&&generated.load()>=frameMultiplier989-1){
     if(getenv("SMX_PROOF989")){std::vector<ID3D11Texture2D*> proofTextures989;for(unsigned k=0;k<=frameMultiplier989;++k)proofTextures989.push_back((k==0||k==frameMultiplier989)?frames989[k]->first.Get():frames989[k]->middle.Get());smxPixelProof989(device.get(),context.get(),proofTextures989);}
     finishProbe989();break; // bounded one-batch correctness probe, preserve tail display
    }
    continue;
   }
