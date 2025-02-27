#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include <implot.h>
using namespace glm;
namespace vka
{
	namespace shader_plot
	{
		#include <vka/shaders/lib/plot/interface_structs.glsl>
	}

	template <typename T>
    struct render_plot;

	template <>
    struct render_plot<shader_plot::GLSLYListPlot>
	{
	    void operator()(const shader_plot::GLSLYListPlot &plot, void *plotData)
	    {
		    std::string label = "Line Plot " + std::to_string(plot.plotID);
		    if (plot.count > 0)
		    {
			    switch (plot.dataType)
			    {
				    case PLOT_DATA_TYPE_FLOAT:
					    if (ImPlot::BeginPlot(label.c_str()))
					    {
						    ImPlot::SetupAxes("x", "y");
							ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
						    ImPlot::PlotShaded("f(x)", static_cast<float *>(plotData) + plot.offset, static_cast<int>(plot.count), -INFINITY, plot.stride, 0.0, 0, 0);
						    ImPlot::PopStyleVar();
						    ImPlot::PlotLine("f(x)", static_cast<float *>(plotData) + plot.offset, static_cast<int>(plot.count), plot.stride, 0.0, 0, 0);
						    ImPlot::EndPlot();
					    }
					    break;
				    default:
					    DEBUG_BREAK;
					    break;
			    }
		    }
	    }
	};

	template <>
    struct render_plot<shader_plot::GLSLHistogram>
    {
	  private:
		  void floatPlot(const shader_plot::GLSLHistogram& plot, void* plotData)
		  {
		      //ImPlot::SetupAxes("x", "y", 0, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels);
		      ImPlot::SetupAxes("x", "y", 0, ImPlotAxisFlags_AutoFit);
		      if (plot.histDim == 1)
		      {
			      std::vector<ImVec4> fillStyles = {ImVec4(1, 0, 0, 0.25), ImVec4(0, 1, 0, 0.25), ImVec4(0, 0, 1, 0.25)};
			      VKA_ASSERT(plot.dataDim <= 3);
			      for (int i = 0; i < plot.dataDim; i++)
			      {
				      if (plot.dataDim > 1)
				      {
					      ImPlot::SetNextFillStyle(fillStyles[i]);
				      }
					  
				      //ImPlot::PlotHistogram(("plot_" + std::to_string(i)).c_str(), static_cast<float *>(plotData) + plot.offset + i * plot.count,
				      ImPlot::PlotHistogram("", static_cast<float *>(plotData) + plot.offset + i * plot.count,
				                            static_cast<int>(plot.count), plot.bins.x, 1.0,
				                            ImPlotRange(plot.rMin.x, plot.rMax.x), ImPlotHistogramFlags_NoOutliers);
			      }
		      }
		      else if (plot.histDim == 2)
		      {
			      ImPlot::PushColormap("Hot");
			      uint32_t maxCount = ImPlot::PlotHistogram2D("", static_cast<float *>(plotData) + plot.offset, static_cast<float *>(plotData) + plot.offset + plot.count,
			                                                  static_cast<int>(plot.count), plot.bins.x, plot.bins.y,
			                                                  ImPlotRect(plot.rMin.x, plot.rMax.x, plot.rMin.y, plot.rMax.y), 0);        // ImPlotHistogramFlags_NoOutliers | ImPlotHistogramFlags_Density
			      ImPlot::PopColormap();
		      }
		      ImPlot::EndPlot();
		}
	  public:
	    void operator()(const shader_plot::GLSLHistogram &plot, void *plotData)
	    {
		    std::string label = "Plot " + std::to_string(plot.plotID);
		    if (plot.count > 0)
		    {
			    switch (plot.dataType)
			    {
				    case PLOT_DATA_TYPE_FLOAT:
					    if (ImPlot::BeginPlot(label.c_str()))
					    {
						    floatPlot(plot, plotData);
					    }
					    break;
				    default:
					    DEBUG_BREAK;
					    break;
			    }
		    }
	    }
    };
    }		// namespace vka