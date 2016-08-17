/*
 Copyright (c) 2016, Jack Miles Hunt
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * Neither the name of Jack Miles Hunt nor the
      names of contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Jack Miles Hunt BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEANFIELD_SHARED_BILATERAL_FILTER_HEADER
#define MEANFIELD_SHARED_BILATERAL_FILTER_HEADER

#include "code_sharing.h"
#include <cmath>

namespace MeanField{
	namespace Filtering{
		__SHARED_CODE__
		inline void applyBilateralKernel(const float *spatial_kernel, const float *intensity_kernel,
										 const float *input, const unsigned char *rgb, float *output,
										 float sd_spatial, float sd_intensity, int dim, int x, int y, int W, int H){
			float normaliser = 0.0;
			float spatialFactor, intensityFactor;
			float *channelSum = new float[dim];
			for(int i=0; i<dim; i++){
				channelSum[i] = 0.0;
			}

			int sd_int = (sd_spatial > sd_intensity) ? (int)sd_spatial : (int)sd_intensity;
			int idx_x, idx_y, idx_c, idx_n;
			//Convolve for each channel.
			for(int i=-sd_int; i<sd_int; i++){
				idx_y = y+i;
				if(idx_y < 0 || idx_y >= H){
					continue;
				}
				
				for(int j=-sd_int; j<sd_int; j++){
					idx_x = x+j;
					if(idx_x < 0 || idx_x >= W){
						continue;
					}

					idx_c = 3*(y*W + x);//Current pixel idx.
					idx_n = 3*(idx_y*W + idx_x);//Neighbour idx.
					
					spatialFactor = spatial_kernel[(int)fabs((float)i)]*spatial_kernel[(int)fabs((float)j)];
					intensityFactor = intensity_kernel[(int)fabs((float)rgb[idx_n] - (float)rgb[idx_c])]*//R
						intensity_kernel[(int)fabs((float)rgb[idx_n + 1] - (float)rgb[idx_c + 1])]*//G
						intensity_kernel[(int)fabs((float)rgb[idx_n + 2] - (float)rgb[idx_c + 2])];//B
					
					normaliser += spatialFactor*intensityFactor;

					//Update cumulative output for each dimension/channel.
					for(int k=0; k<dim; k++){
						channelSum[k] += input[(idx_y*W + idx_x)*dim + k]*spatialFactor*intensityFactor;
					}
				}
			}

			//Normalise outputs.
			if(normaliser > 0.0){
				for(int i=0; i<dim; i++){
					output[(y*W + x)*dim + i] = channelSum[i]/normaliser;
				}
			}
			delete[] channelSum;
		}
	}
}

#endif
