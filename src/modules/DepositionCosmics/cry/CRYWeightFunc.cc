/*

Copyright (c) 2007-2012, The Regents of the University of California.
Produced at the Lawrence Livermore National Laboratory
UCRL-CODE-227323.
All rights reserved.

For details, see http://nuclear.llnl.gov/simulations
Please also read this http://nuclear.llnl.gov/simulations/additional_bsd.html

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions of source code must retain the above copyright
notice, this list of conditions and the disclaimer below.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the disclaimer (as noted below) in
the documentation and/or other materials provided with the
distribution.

3. Neither the name of the UC/LLNL nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OF
THE UNIVERSITY OF CALIFORNIA, THE U.S. DEPARTMENT OF ENERGY OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "core/utils/log.h"

#include <assert.h>
#include <iostream>
#include "CRYBinning.h"
#include "CRYWeightFunc.h"

CRYWeightFunc::CRYWeightFunc(CRYBinning* bins, std::vector<double> weights) {

    _bins = bins;
    _weights = new std::vector<double>(weights);

    // consistency checks

    if(_bins == nullptr) {
        LOG(ERROR) << "CRY::CRYWeightFunc: bins = NULL passed into CRYWeightFunc. Stopping\n";
        assert(0);
    }
    if(_bins->bins()->size() != 1 + _weights->size()) {
        LOG(ERROR) << "CRY::CRYWeightFunc: Inconsistent set of weights and binnings. Stopping\n";
        assert(0);
    }
}

double CRYWeightFunc::weight(double value) {
    auto b = static_cast<size_t>(_bins->bin(value));
    return (*_weights)[b];
}

double CRYWeightFunc::weightBin(unsigned int b) { return (*_weights)[b]; }
