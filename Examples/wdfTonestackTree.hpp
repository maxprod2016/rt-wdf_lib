/*
 ==============================================================================

 This file is part of the RT-WDF library.
 Copyright (c) 2015,2016 - Maximilian Rest

 Permission is granted to use this software under the terms of either:
 a) the GPL v2 (or any later version)
 b) the Affero GPL v3

 Details of these licenses can be found at: www.gnu.org/licenses

 RT-WDF is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 -----------------------------------------------------------------------------
 To release a closed-source product which uses RT-WDF, commercial licenses are
 available: write to rt-wdf@e-rm.de for more information.

 ==============================================================================

 wdfTonestackTree.h
 Created:  1 May 2016 1:16:19pm
 Author:  mrest

 ==============================================================================
*/



#pragma once
#include "../Libs/rt-wdf/rt-wdf.h"

using namespace arma;

class wdfTonestackTree : public wdfTree
{

private:
    //----------------------------------------------------------------------
    const float C1 = 250e-12;
    const float C2 = 20e-9;
    const float C3 = 20e-9;


    const float R1 = 250e3;
    const float R2 = 1e6;
    const float R3 = 25e3;
    const float R4 = 56e3;

    std::unique_ptr<wdfTerminatedCap> Cap1;
    std::unique_ptr<wdfTerminatedCap> Cap2;
    std::unique_ptr<wdfTerminatedCap> Cap3;

    std::unique_ptr<wdfTerminatedRes> Res1p;
    std::unique_ptr<wdfTerminatedRes> Res1m;
    std::unique_ptr<wdfTerminatedRes> Res2;
    std::unique_ptr<wdfTerminatedRes> Res3m;
    std::unique_ptr<wdfTerminatedRes> Res3p;
    std::unique_ptr<wdfTerminatedRes> Res4;

    std::unique_ptr<wdfTerminatedResVSource> Vres;

    std::unique_ptr<wdfTerminatedSeries> S1;
    std::unique_ptr<wdfTerminatedSeries> S2;
    std::unique_ptr<wdfTerminatedSeries> S3;
    std::unique_ptr<wdfTerminatedSeries> S4;

    std::string treeName = "Fender Tone Stack";

public:
    //----------------------------------------------------------------------
    ~wdfTonestackTree( ) {

    }

    //----------------------------------------------------------------------
    wdfTonestackTree( ) {

        setSamplerate( 44100 );

        // parameters
        paramData bassParam;
        bassParam.name    = "Bass";
        bassParam.ID      = 0;
        bassParam.type    = doubleParam;
        bassParam.value   = 0.5;
        bassParam.units   = " ";
        bassParam.lowLim  = 0.0001;
        bassParam.highLim = 0.9999;

        paramData midParam;
        midParam.name    = "Mid";
        midParam.ID      = 1;
        midParam.type    = doubleParam;
        midParam.value   = 0.5;
        midParam.units   = " ";
        midParam.lowLim  = 0.0001;
        midParam.highLim = 0.9999;

        paramData trebleParam;
        trebleParam.name    = "Treble";
        trebleParam.ID      = 2;
        trebleParam.type    = doubleParam;
        trebleParam.value   = 0.5;
        trebleParam.units   = " ";
        trebleParam.lowLim  = 0.0001;
        trebleParam.highLim = 0.9999;


        params.push_back( bassParam );
        params.push_back( midParam );
        params.push_back( trebleParam );

        // build up the circuit.

        // ROOT PORT A
        Vres.reset( new wdfTerminatedResVSource(0,1) );
        Res3m.reset( new wdfTerminatedRes(params[1].value * R3) );
        S1.reset( new wdfTerminatedSeries(Vres.get(), Res3m.get()) );

        // ROOT PORT B
        Res2.reset( new wdfTerminatedRes(params[0].value * R2) );
        Res3p.reset( new wdfTerminatedRes((1 - params[1].value) * R3) );
        S3.reset( new wdfTerminatedSeries(Res2.get(), Res3p.get()) );

        // ROOT PORT C
        Res1p.reset( new wdfTerminatedRes((1 - params[2].value) * R1) );
        Res1m.reset( new wdfTerminatedRes(params[2].value * R1) );
        S4.reset( new wdfTerminatedSeries(Res1p.get(), Res1m.get()) );
        Cap1.reset( new wdfTerminatedCap(C1, 1) );
        S2.reset( new wdfTerminatedSeries(Cap1.get(), S4.get()) );

        // ROOT PORT D
        Cap2.reset( new wdfTerminatedCap(C2, 1) );

        // ROOT PORT E
        Res4.reset( new wdfTerminatedRes(R4) );

        // ROOT PORT F
        Cap3.reset( new wdfTerminatedCap(C3, 1) );

        subtreeCount      = 6;
        subtreeEntryNodes = new wdfTreeNode*[subtreeCount];
        subtreeEntryNodes[0] = S1.get();
        subtreeEntryNodes[1] = S3.get();
        subtreeEntryNodes[2] = S2.get();
        subtreeEntryNodes[3] = Cap2.get();
        subtreeEntryNodes[4] = Res4.get();
        subtreeEntryNodes[5] = Cap3.get();

        root.reset( new wdfRootRtype( subtreeCount ) );
        Rp   = new double[subtreeCount]( );
    }

    //----------------------------------------------------------------------
    int setRootMatrData( matData* rootMats,
                         double* Rp ) {

        //------------------------- S matrix -------------------------
        if( rootMats->Smat.is_empty() ) {
            return -1;
        }
        if( rootMats->Smat.n_cols != subtreeCount ) {
            return -1;
        }
        if( rootMats->Smat.n_rows != subtreeCount ) {
            return -1;
        }

        double RA = Rp[0];
        double RB = Rp[1];

        double arr[6][6] =
        {
            {                       1.0 - (2.0*RA*(351917.89300000000002910383045673*RB + 398678212.55944902048431738512591))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                       (462491596.2328980237769439117983*RA)/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                  (2.0*RA*(56000.0*RB + 63813383.673449003292626526672393))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                                          -(2.0*RA*(56000.0*RB - 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                        -(2.0*RA*(295917.89300000000002910383045673*RB + 334864828.88600001719169085845351))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                        -(2.0*RA*(351917.89300000000002910383045673*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)},
            {                                                                           (462491596.2328980237769439117983*RB)/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),   1.0 - (2.0*RB*(351917.89300000000002910383045673*RA + 16770901798.116449011888471955899))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                        -(2.0*RB*(566.8930000000000291038304567337*RA + 63813383.673449003292626526672393))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                                         -(2.0*RB*(351351.0*RA + 16707088414.443000008595845429227))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                         -(2.0*RB*(566.8930000000000291038304567337*RA - 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                            -(2.0*RB*(351917.89300000000002910383045673*RA + 16539656000.0))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)},
            {                                                     (590702.0*(56000.0*RB + 63813383.673449003292626526672393))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),       -(590702.0*(566.8930000000000291038304567337*RA + 63813383.673449003292626526672393))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218), 1.0 - (590702.0*(1133.7860000000000582076609134674*RA + 56566.893000000000029103830456734*RB + RA*RB + 63813383.673449003292626526672393))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                     (590702.0*(566.8930000000000291038304567337*RA + 56566.893000000000029103830456734*RB + RA*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                            -(590702.0*(1133.7860000000000582076609134674*RA + 566.8930000000000291038304567337*RB + RA*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                              -(590702.0*(566.8930000000000291038304567337*RA - 56000.0*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)},
            {                           -(1133.7860000000000582076609134674*(56000.0*RB - 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),      -(1133.7860000000000582076609134674*(351351.0*RA + 16707088414.443000008595845429227))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                   (1133.7860000000000582076609134674*(566.8930000000000291038304567337*RA + 56566.893000000000029103830456734*RB + RA*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218), 1.0 - (1133.7860000000000582076609134674*(351917.89300000000002910383045673*RA + 56566.893000000000029103830456734*RB + RA*RB + 16707088414.443000008595845429227))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218), (1133.7860000000000582076609134674*(566.8930000000000291038304567337*RA + 566.8930000000000291038304567337*RB + RA*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                             -(1133.7860000000000582076609134674*(351351.0*RA + 56000.0*RB + 16539656000.0))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)},
            {                          -(112000.0*(295917.89300000000002910383045673*RB + 334864828.88600001719169085845351))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),       -(112000.0*(566.8930000000000291038304567337*RA - 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                           -(112000.0*(1133.7860000000000582076609134674*RA + 566.8930000000000291038304567337*RB + RA*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                  (112000.0*(566.8930000000000291038304567337*RA + 566.8930000000000291038304567337*RB + RA*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                  1.0 - (112000.0*(1133.7860000000000582076609134674*RA + 296484.78600000000005820766091347*RB + RA*RB + 334864828.88600001719169085845351))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                -(112000.0*(566.8930000000000291038304567337*RA + 295917.89300000000002910383045673*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)},
            { -(1133.7860000000000582076609134674*(351917.89300000000002910383045673*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218), -(1133.7860000000000582076609134674*(351917.89300000000002910383045673*RA + 16539656000.0))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                    -(1133.7860000000000582076609134674*(566.8930000000000291038304567337*RA - 56000.0*RB))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),                                                                                     -(1133.7860000000000582076609134674*(351351.0*RA + 56000.0*RB + 16539656000.0))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218),       -(1133.7860000000000582076609134674*(566.8930000000000291038304567337*RA + 295917.89300000000002910383045673*RB + 167432414.44300000859584542922676))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218), 1.0 - (1133.7860000000000582076609134674*(351917.89300000000002910383045673*RA + 351917.89300000000002910383045673*RB + 16707088414.443000008595845429227))/(398678212.55944902048431738512591*RA + 16770901798.116449011888471955899*RB + 351917.89300000000002910383045673*RA*RB + 18847346681336.836571480537279218)}
        };


        for ( unsigned int ii = 0; ii < 6; ++ii ) {
            for ( unsigned int jj = 0; jj < 6; ++jj ) {
                rootMats->Smat.at(ii, jj) = arr[ii][jj];
            }

        }

        return 0;
    }

    //----------------------------------------------------------------------
    void setInputValue( double signalIn ) {
        Vres->Vs = signalIn;
    }

    //----------------------------------------------------------------------
    double getOutputValue( ) {
        return Res1m->upPort->getPortVoltage( ) + Res3m->upPort->getPortVoltage( ) + S3->upPort->getPortVoltage( );
    }

    //----------------------------------------------------------------------
    const char* getTreeIdentifier( ) {
        return treeName.c_str( );

    }

    //----------------------------------------------------------------------
    void setParam( size_t paramID, double paramValue ) {
        if( paramID == 0 ) {
            Res2->R = paramValue * R2;               // bass
            adaptTree();
            params[0].value = paramValue;
        }
        if( paramID == 1 ) {
            Res3p->R = (1-paramValue) * R3;            // mid
            Res3m->R =   paramValue * R3;
            adaptTree();
            params[1].value = paramValue;
        }
        if( paramID == 2 ) {
            Res1m->R =  (double) (paramValue * R1);     // treble
            Res1p->R =  (double) (R1 - Res1m->R);
            adaptTree();
            params[2].value = paramValue;
        }
    }

};