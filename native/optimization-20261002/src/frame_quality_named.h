#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
static void print_frame_quality(const unsigned char*a,const unsigned char*b,uint32_t w,uint32_t h,const char*label){
 uint64_t pixels=(uint64_t)w*h,changed=0,alpha=0,hist[256]={0},above[5]={0};double sum=0,sq=0;unsigned maxerr=0;
 for(uint64_t p=0;p<pixels;p++){
  unsigned emax=0;for(unsigned c=0;c<3;c++){int d=(int)a[p*4+c]-(int)b[p*4+c];unsigned ad=d<0?-d:d;sum+=ad;sq+=(double)d*d;if(ad>emax)emax=ad;}
  alpha+=a[p*4+3]!=b[p*4+3];hist[emax]++;changed+=emax>0;if(emax>maxerr)maxerr=emax;
  const unsigned cut[5]={1,2,4,8,16};for(unsigned i=0;i<5;i++)above[i]+=emax>cut[i];
 }
 double ssim=0;uint64_t windows=0;
 for(uint32_t by=0;by<h;by+=8)for(uint32_t bx=0;bx<w;bx+=8){
  double sa=0,sb=0,saa=0,sbb=0,sab=0;unsigned n=0;
  for(uint32_t y=by;y<h&&y<by+8;y++)for(uint32_t x=bx;x<w&&x<bx+8;x++){
   uint64_t p=((uint64_t)y*w+x)*4;double va=a[p]*0.2126+a[p+1]*0.7152+a[p+2]*0.0722,vb=b[p]*0.2126+b[p+1]*0.7152+b[p+2]*0.0722;
   sa+=va;sb+=vb;saa+=va*va;sbb+=vb*vb;sab+=va*vb;n++;
  }
  double ma=sa/n,mb=sb/n,va=fmax(0,saa/n-ma*ma),vb=fmax(0,sbb/n-mb*mb),cov=sab/n-ma*mb;
  ssim+=((2*ma*mb+6.5025)*(2*cov+58.5225))/((ma*ma+mb*mb+6.5025)*(va+vb+58.5225));windows++;
 }
 unsigned p95=0,p99=0;uint64_t count=0;for(unsigned i=0;i<256;i++){count+=hist[i];if(count*100<pixels*95)p95=i+1;if(count*100<pixels*99)p99=i+1;}
 printf("{\"reference\":\"%s\",\"rgb_mae_255\":%.9f,\"rgb_rmse_255\":%.9f,\"rgb_psnr_db\":",label,sum/(pixels*3),sqrt(sq/(pixels*3)));
 if(sq==0)printf("null");else printf("%.9f",10*log10(65025.0/(sq/(pixels*3))));
 printf(",\"luma_ssim_box8\":%.9f,\"rgb_max_error_255\":%u,\"pixel_max_error_p95\":%u,\"pixel_max_error_p99\":%u,\"changed_pixel_percent\":%.9f,\"pixels_above_1_percent\":%.9f,\"pixels_above_2_percent\":%.9f,\"pixels_above_4_percent\":%.9f,\"pixels_above_8_percent\":%.9f,\"pixels_above_16_percent\":%.9f,\"alpha_mismatches\":%llu,\"rgb_exact\":%s}",ssim/windows,maxerr,p95,p99,100.0*changed/pixels,100.0*above[0]/pixels,100.0*above[1]/pixels,100.0*above[2]/pixels,100.0*above[3]/pixels,100.0*above[4]/pixels,(unsigned long long)alpha,sq==0?"true":"false");
}
