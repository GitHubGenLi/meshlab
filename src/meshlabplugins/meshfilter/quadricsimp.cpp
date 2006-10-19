/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.																											 *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/
/****************************************************************************
  History
$Log$
Revision 1.3  2006/10/19 07:34:24  cignoni
added callback

Revision 1.2  2006/10/15 17:08:52  cignoni
typenames and qualifiers for gcc compliance

Revision 1.1  2006/10/10 21:13:08  cignoni
Added remove non manifold and quadric simplification filter.

****************************************************************************/
#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include <limits>

#include "meshfilter.h"
#include <vcg/complex/trimesh/update/position.h>
#include <vcg/complex/trimesh/update/bounding.h>
#include <vcg/complex/local_optimization.h>
#include <vcg/complex/local_optimization/tri_edge_collapse_quadric.h>
#include <vcg/container/simple_temporary_data.h>
using namespace vcg;
using namespace std;


typedef	SimpleTempData<CMeshO::VertContainer, math::Quadric<double> > QuadricTemp;


class QHelper
		{
		public:
			QHelper(){};
      static void Init(){};
      static math::Quadric<double> &Qd(CVertexO &v) {return TD()[v];}
      static math::Quadric<double> &Qd(CVertexO *v) {return TD()[*v];}
      static CVertexO::ScalarType W(CVertexO * /*v*/) {return 1.0;};
      static CVertexO::ScalarType W(CVertexO & /*v*/) {return 1.0;};
      static void Merge(CVertexO & /*v_dest*/, CVertexO const & /*v_del*/){};
      static QuadricTemp* &TDp() {static QuadricTemp *td; return td;}
      static QuadricTemp &TD() {return *TDp();}
		};


class MyTriEdgeCollapse: public vcg::tri::TriEdgeCollapseQuadric< CMeshO, MyTriEdgeCollapse, QHelper > {
						public:
						typedef  vcg::tri::TriEdgeCollapseQuadric< CMeshO,  MyTriEdgeCollapse, QHelper> TECQ;
            typedef  CMeshO::VertexType::EdgeType EdgeType;
            inline MyTriEdgeCollapse(  const EdgeType &p, int i) :TECQ(p,i){}
};


void QuadricSimplification(CMeshO &cm,int  TargetFaceNum, CallBackPos *cb)
{
  math::Quadric<double> QZero;
  QZero.Zero();
  QuadricTemp TD(cm.vert);
  QHelper::TDp()=&TD;

  TD.Start(QZero);
  tri::TriEdgeCollapseQuadricParameter qparams;
  MyTriEdgeCollapse::SetDefaultParams();
  qparams.QualityThr  =.3;
  vcg::LocalOptimization<CMeshO> DeciSession(cm);
	cb(1,"Initializing simplification");
	DeciSession.Init<MyTriEdgeCollapse >();

	DeciSession.SetTargetSimplices(TargetFaceNum);
	DeciSession.SetTimeBudget(0.1f); // this allow to update the progress bar 10 time for sec...
//  if(TargetError< numeric_limits<double>::max() ) DeciSession.SetTargetMetric(TargetError);
  int startFn=cm.fn;
  int faceToDel=cm.fn-TargetFaceNum;
 while( DeciSession.DoOptimization() && cm.fn>TargetFaceNum )
 {
   cb(100-100*(cm.fn-TargetFaceNum)/(faceToDel), "Simplifing");
 };

 
}
