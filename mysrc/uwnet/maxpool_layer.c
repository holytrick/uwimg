#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include "uwnet.h"

// Run a maxpool layer on input
// layer l: pointer to layer to run
// matrix in: input to layer
// returns: the result of running the layer
matrix forward_maxpool_layer(layer l, matrix in)
{
    int pad=(l.size>>1)-(l.size&1?0:1);
    int outw = (l.width+ 2*pad - l.size)/l.stride + 1;
    int outh = (l.height+ 2*pad -l.size)/l.stride + 1;
    matrix out = make_matrix(in.rows, outw*outh*l.channels);

    // TODO: 6.1 - iterate over the input and fill in the output with max values
	int i,j;
	for(i = 0; i < in.rows; ++i){
        image example = float_to_image(in.data + i*in.cols, l.width, l.height, l.channels);
        matrix x = im2col(example, l.size, l.stride);
        matrix wx = matmax(l.size*l.size, x);
        for(j = 0; j < wx.rows*wx.cols; ++j){
            out.data[i*out.cols + j] = wx.data[j];
        }
        FM(x);
        FM(wx);
    }
	
    l.in[0] = in;
    FM(l.out[0]);
    l.out[0] = out;
    FM(l.delta[0]);
    l.delta[0] = make_matrix(out.rows, out.cols);
    return out;
}

// Run a maxpool layer backward
// layer l: layer to run
// matrix prev_delta: error term for the previous layer

void backward_maxpool_layer(layer l, matrix prev_delta)
{
    matrix in    = l.in[0];
    matrix out   = l.out[0];
    matrix delta = l.delta[0];

    int pad=(l.size>>1)-(l.size&1?0:1);
    int outw = (l.width+ 2*pad - l.size)/l.stride + 1;
    int outh = (l.height+ 2*pad -l.size)/l.stride + 1;
	
	
    // TODO: 6.2 - find the max values in the input again and fill in the
    // corresponding delta with the delta from the output. This should be
    // similar to the forward method in structure.	
	int i, j, k, m;
    int dx, dy;
	float delMin=delta.data[0];
	for(i=1;i<delta.rows*delta.cols;++i)if(delMin>delta.data[i])delMin=delta.data[i];
	
	for(i = 0; i < in.rows; ++i)
	for(j = 0; j < l.channels; ++j)
    for(k = 0; k < outh; ++k)
	for(m = 0; m < outw; ++m){
		float max = delMin;//-FLT_MAX;
		int maxi = -1;
		for(dy = 0; dy < l.size; ++dy)
		for(dx = 0; dx < l.size; ++dx){
			int inx = m*l.stride - pad + dx;
			int iny = k*l.stride - pad + dy;
			int index = i*in.cols + j*l.height*l.width + iny*l.width + inx;
			if (inx >= 0 && inx < l.width && iny >= 0 && iny < l.height && in.data[index] > max){
				max = in.data[index];
				maxi = index;
			}

		}
		if(maxi>=0){
			int index = i*out.cols + j*outh*outw + k*outw + m;
			prev_delta.data[maxi]+= delta.data[index];
		}
	}
}

// Update maxpool layer
// Leave this blank since maxpool layers have no update
void update_maxpool_layer(layer l, float rate, float momentum, float decay)
{
}

// Make a new maxpool layer
// int w: width of input image
// int h: height of input image
// int c: number of channels
// int size: size of maxpool filter to apply
// int stride: stride of operation
layer make_maxpool_layer(int w, int h, int c, int size, int stride)
{
    layer l = {0};
    l.width = w;
    l.height = h;
    l.channels = c;
    l.size = size;
    l.stride = stride;
    l.in = calloc(1, sizeof(matrix));
    l.out = calloc(1, sizeof(matrix));
    l.delta = calloc(1, sizeof(matrix));
    l.forward  = forward_maxpool_layer;
    l.backward = backward_maxpool_layer;
    l.update   = update_maxpool_layer;
    return l;
}

