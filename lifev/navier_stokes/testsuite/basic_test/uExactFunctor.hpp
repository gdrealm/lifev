/* -*- mode: c++ -*-
 
 This file is part of the LifeV Applications.
 
 Author(s): Christophe Prud'homme <christophe.prudhomme@epfl.ch>
 Gwenol Grandperrin <gwenol.grandperrin@epfl.ch>
 Date: 2011-03-09
 
 Copyright (C) 2010 EPFL
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA
 */
/*!
 @file uExactFunctor.hpp
 @author Davide Forti <davide.forti@epfl.ch>
 @contributor Claudia Colciago <claudia.colciago@epfl.ch>
 @date 2014-01-31
 */

#ifndef UEXACTFUNCTOR_H
#define UEXACTFUNCTOR_H 1

#include <lifev/core/LifeV.hpp>
#include <lifev/navier_stokes/function/RossEthierSteinmanDec.hpp>

namespace LifeV
{
    
class uExactFunctor
{
public:
    typedef VectorSmall<3> return_Type;
    
    return_Type operator() ( const Real time, const VectorSmall<3> spaceCoordinates )
    {
        VectorSmall<3> uComponent;
        uComponent[0] = RossEthierSteinmanUnsteadyDec::uexact( time, spaceCoordinates[0], spaceCoordinates[1], spaceCoordinates[2] , 0  ) ;
        uComponent[1] = RossEthierSteinmanUnsteadyDec::uexact( time, spaceCoordinates[0], spaceCoordinates[1], spaceCoordinates[2] , 1  ) ;
        uComponent[2] = RossEthierSteinmanUnsteadyDec::uexact( time, spaceCoordinates[0], spaceCoordinates[1], spaceCoordinates[2] , 2  ) ;
        
        return uComponent;
    }
    
    uExactFunctor() {}
    uExactFunctor (const uExactFunctor&) {}
    ~uExactFunctor() {}
};
    
} // end namespace LifeV
    
#endif // UEXACTFUNCTOR_H
