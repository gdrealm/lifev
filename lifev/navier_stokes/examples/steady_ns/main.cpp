/* -*- mode: c++ -*-

  This file is part of the LifeV library.
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
/**
   \file main.cpp
   \author Davide Forti <davide.forti@epfl.ch>
   \date 2014-02-06
 */


#include <Epetra_ConfigDefs.h>
#ifdef EPETRA_MPI
#include <mpi.h>
#include <Epetra_MpiComm.h>
#else
#include <Epetra_SerialComm.h>
#endif


#include <lifev/core/LifeV.hpp>
#include <lifev/core/mesh/MeshData.hpp>
#include <lifev/core/mesh/MeshPartitioner.hpp>
#include <lifev/navier_stokes/solver/NavierStokesSolver.hpp>
#include <lifev/core/fem/TimeAndExtrapolationHandler.hpp>
#include <lifev/core/filter/ExporterEnsight.hpp>
#include <lifev/core/filter/ExporterHDF5.hpp>
#include <lifev/core/filter/ExporterVTK.hpp>
#include <Teuchos_XMLParameterListHelpers.hpp>

#include "boundaryConditions.hpp"

using namespace LifeV;

int
main ( int argc, char** argv )
{
    bool verbose (false);
#ifdef HAVE_MPI
    MPI_Init (&argc, &argv);
    boost::shared_ptr<Epetra_Comm> Comm ( new Epetra_MpiComm (MPI_COMM_WORLD) );
    if ( Comm->MyPID() == 0 )
    {
        verbose = true;
    }
#else
    boost::shared_ptr<Epetra_Comm> Comm( new Epetra_SerialComm () );
    verbose = true;
#endif

    {

    typedef RegionMesh<LinearTetra> mesh_Type;
    typedef VectorEpetra vector_Type;
    typedef boost::shared_ptr<vector_Type> vectorPtr_Type;

    // Reading the dataFile
    const std::string defaultDataName = "data";
    GetPot command_line (argc, argv);
    std::string data_file_name = command_line.follow (defaultDataName.c_str(), 2, "-f", "--file");
    GetPot dataFile( data_file_name );

    // reading the mesh
    boost::shared_ptr<mesh_Type > fullMeshPtr ( new mesh_Type ( Comm ) );
    MeshData meshData;
    meshData.setup (dataFile, "fluid/space_discretization");
    readMesh (*fullMeshPtr, meshData);

    // mesh partitioner
    MeshPartitioner< mesh_Type >  meshPart (fullMeshPtr, Comm);
    boost::shared_ptr<mesh_Type > localMeshPtr ( new mesh_Type ( Comm ) );
    localMeshPtr = meshPart.meshPartition();
    fullMeshPtr.reset();

    // create the solver
    NavierStokesSolver ns( dataFile, Comm);
    ns.setup(localMeshPtr);
    ns.setParameters();
    ns.buildSystem();

    // Exporter
    std::string outputName = dataFile ( "exporter/filename", "result");
    boost::shared_ptr< Exporter<mesh_Type > > exporter;
    std::string const exporterType =  dataFile ( "exporter/type", "ensight");

#ifdef HAVE_HDF5
    if (exporterType.compare ("hdf5") == 0)
    {
    	exporter.reset ( new ExporterHDF5<mesh_Type > ( dataFile, outputName ) );
    	exporter->setPostDir ( "./" ); // This is a test to see if M_post_dir is working
    	exporter->setMeshProcId ( localMeshPtr, Comm->MyPID() );
    }
#endif
    else if(exporterType.compare ("vtk") == 0)
    {
    	exporter.reset ( new ExporterVTK<mesh_Type > ( dataFile, outputName ) );
    	exporter->setPostDir ( "./" ); // This is a test to see if M_post_dir is working
    	exporter->setMeshProcId ( localMeshPtr, Comm->MyPID() );
    }

    vectorPtr_Type velocity( new vector_Type(ns.uFESpace()->map(), exporter->mapType() ) );
    vectorPtr_Type pressure( new vector_Type(ns.pFESpace()->map(), exporter->mapType() ) );
    exporter->addVariable ( ExporterData<mesh_Type>::VectorField, "velocity", ns.uFESpace(), velocity, UInt (0) );
    exporter->addVariable ( ExporterData<mesh_Type>::ScalarField, "pressure", ns.pFESpace(), pressure, UInt (0) );
    exporter->postProcess ( 0.0 );

    // Boundary conditions
    boost::shared_ptr<BCHandler> bc ( new BCHandler (*BCh_fluid ()) );

    std::string preconditioner = dataFile("fluid/preconditionerType","none");

    if ( preconditioner.compare("PCD") == 0 )
    {
    	boost::shared_ptr<BCHandler> bc_pcd ( new BCHandler (*BCh_PCD ()) );
    	ns.setBCpcd(bc_pcd);
    }

    ns.setBC( bc );

    ns.iterate_steady( bc );

    ns.updateVelocity(velocity);
    ns.updatePressure(pressure);

    exporter->postProcess ( 1.0 );

    exporter->closeFile();

	}

#ifdef HAVE_MPI
    if (verbose)
    {
        std::cout << "\nMPI Finalization" << std::endl;
    }
    MPI_Finalize();
#endif
    return ( EXIT_SUCCESS );
}

