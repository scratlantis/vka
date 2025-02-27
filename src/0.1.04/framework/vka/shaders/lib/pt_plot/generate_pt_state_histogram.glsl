#ifndef GENERATE_PT_STATE_HISTOGRAM_H
#define GENERATE_PT_STATE_HISTOGRAM_H

#extension GL_EXT_debug_printf : enable

#ifdef PT_PLOT_INTERFACE_SMD_H
#ifdef HISTOGRAM_SMD_H
#ifdef PT_SHADER_STATE_H


void writePtPlot()
{
	initHist(200, 10000, 0.0, 5.0);
	hist[hist_count - 1].histDim = 1;
	hist[hist_count - 1].bins = vec2(200);
	hist[hist_count - 1].rMin = vec2(0.0);
	hist[hist_count - 1].rMax = vec2(5.0);
	if(ptPlotOptions.writeTotalContribution != 0)
	{
		setHistValue(pt_state.totalContribution, getFrameIdx());
		ptPlot.totalContribution = int(hist_count - 1);
	}
	vec3 origin = pt_state.primaryOrigin;
	if(ptPlotOptions.bounce > 0)
	{
		origin = indirect_bounces[ptPlotOptions.bounce - 1].bouncePos;
	}
	bool writePlot = pt_state.bounce > ptPlotOptions.bounce;

	nextHist();
	hist[hist_count - 1].histDim = 2;
	hist[hist_count - 1].bins = vec2(100, 50);
	hist[hist_count - 1].rMin = vec2(-PI, 0);
	hist[hist_count - 1].rMax = vec2(PI);
	if(ptPlotOptions.writeIndirectDir != 0)
	{
		if(writePlot)
		{
			vec2 sphericalDir = cartesianToSpherical(indirect_bounces[ptPlotOptions.bounce].bounceDir);
			setHistValue(sphericalDir, getFrameIdx());
		}
		ptPlot.indirectRay.dir = int(hist_count - 1);
	}

	nextHist();
	hist[hist_count - 1].histDim = 1;
	hist[hist_count - 1].bins = vec2(200);
	hist[hist_count - 1].rMin = vec2(-1.0);
	hist[hist_count - 1].rMax = vec2(1.0);
	if(ptPlotOptions.writeIndirectT != 0)
	{
		if(writePlot)
		{
			float t;
			if(indirect_bounces[ptPlotOptions.bounce].pathVertexType != PATH_VERTEX_TYPE_LEAF_INFINITY)
			{
				t = distance(origin, indirect_bounces[ptPlotOptions.bounce].bouncePos);
			}
			else
			{
				t = -0.2;
			}
			setHistValue(t, getFrameIdx());
		}
		ptPlot.indirectRay.t = int(hist_count - 1);
	}

	nextHist();
	hist[hist_count - 1].histDim = 1;
	hist[hist_count - 1].bins = vec2(200);
	hist[hist_count - 1].rMin = vec2(0.0);
	hist[hist_count - 1].rMax = vec2(2.0);
	if(ptPlotOptions.writeIndirectWeight != 0)
	{
		if(writePlot)
		{
			setHistValue(indirect_bounces[ptPlotOptions.bounce].localWeight, getFrameIdx());
		}
		ptPlot.indirectRay.weight = int(hist_count - 1);
	}
	
}

#endif
#endif
#endif



#endif