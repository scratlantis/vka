#ifndef PT_SMD_LIB_H
#define PT_SMD_LIB_H

#ifndef PT_USCENE_H
#ifndef PT_USCENE_MOCK_H
#include "../modules/mock/pt_uscene_mock.glsl"
#endif
#endif

#ifdef SELECT_SINGLE_INVOCATION_H
#define SINGLE_INVOCATION( x ) if(invocationIsSelected()) { x }
#else
#define SINGLE_INVOCATION( x )
#endif

// SI stands for Single/Selected Invocation
#ifdef HISTOGRAM_SMD_H
#define SI_initHist( binCount, valueCount, rMin, rMax ) SINGLE_INVOCATION ( initHist( binCount, valueCount, rMin, rMax ); )
#define SI_nextHist() SINGLE_INVOCATION ( nextHist(); )
#define SI_setHistValue( val, sampleIdx ) SINGLE_INVOCATION ( setHistValue( val, sampleIdx ); )
#else
#define SI_initHist( binCount, valueCount, rMin, rMax )
#define SI_nextHist()
#define SI_setHistValue( val, sampleIdx )
#endif

#ifdef Y_LIST_PLOT_SMD_H
#define SI_initPlot( stride ) SINGLE_INVOCATION ( initPlot( stride ); )
#define SI_nextPlot() SINGLE_INVOCATION ( nextPlot(); )
#define SI_setPlotStride( stride ) SINGLE_INVOCATION ( setPlotStride( stride ); )
#define SI_addPlotValue( value ) SINGLE_INVOCATION ( addPlotValue( value ); )
#else
#define SI_initPlot( stride )
#define SI_nextPlot()
#define SI_setPlotStride( stride )
#define SI_addPlotValue( value )
#endif

#ifdef PT_SHADER_STATE_H
#define SI_initPts( primaryRay ) SINGLE_INVOCATION ( initPts( primaryRay ); )
#define SI_nextBounce( ray, hitType ) SINGLE_INVOCATION ( nextBounce( ray, hitType ); )
#define SI_finalizeSample() SINGLE_INVOCATION ( finalizeSample(); )
#define SI_nextSample( domain ) SINGLE_INVOCATION ( nextSample( domain ); )
#define SI_resetPts() SINGLE_INVOCATION ( resetPts(); )

#define SI_setBSDF( bsdf ) SINGLE_INVOCATION ( setBSDF( bsdf ); )
#define SI_setCosineTerm( clampedCosine ) SINGLE_INVOCATION ( setCosineTerm( clampedCosine ); )
#define SI_setIncomingRadiance( L_i ) SINGLE_INVOCATION ( setIncomingRadiance( L_i ); )

#define SI_setTransmittance( transmittance ) SINGLE_INVOCATION ( setTransmittance( transmittance ); )
#define SI_setSourceDistance( sourceDistance ) SINGLE_INVOCATION ( setSourceDistance( sourceDistance ); )
#define SI_setTotalContribution( totalContribution ) SINGLE_INVOCATION ( setTotalContribution( totalContribution ); )
#else
#define SI_initPts( primaryRay )
#define SI_nextBounce( ray, hitType )
#define SI_finalizeSample()
#define SI_nextSample( domain )
#define SI_resetPts()

#define SI_setBSDF( bsdf )
#define SI_setCosineTerm( clampedCosine )
#define SI_setIncomingRadiance( L_i )
#define SI_setTransmittance( transmittance )
#define SI_setSourceDistance( sourceDistance )
#define SI_setTotalContribution( totalContribution )
#endif

#define SI_writePtPlot()
#ifdef PT_PLOT_INTERFACE_SMD_H
#ifdef HISTOGRAM_SMD_H
#ifdef PT_SHADER_STATE_H
#ifdef GENERATE_PT_STATE_HISTOGRAM_H

#undef SI_writePtPlot
#define SI_writePtPlot() SINGLE_INVOCATION ( writePtPlot(); )

#endif
#endif
#endif
#endif

#ifdef DEBUG
#define SI_print( x ) SINGLE_INVOCATION ( debugPrintfEXT( x ); )
#define SI_printf( x, y ) SINGLE_INVOCATION ( debugPrintfEXT( x, y ); )
#endif

#endif