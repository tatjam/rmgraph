/*Copyright 2014 Francisco Alvaro

This file is part of SESHAT.

  SESHAT is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SESHAT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SESHAT.  If not, see <http://www.gnu.org/licenses/>.


This file is a modification of the RNNLIB original software covered by
the following copyright and permission notice:

*/
/*Copyright 2009,2010 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_BiasLayer_h
#define _INCLUDED_BiasLayer_h

#include <vector>
#include "Layer.hpp"

struct BiasLayer: public Layer {
  // data
  View<real_t> acts;
  View<real_t> errors;

  // functions
  BiasLayer(WeightContainer *wc, DataExportHandler *deh) :
    Layer("bias", 0, 0, 1, wc, deh), acts(this->outputActivations[0]),
      errors(this->outputErrors[0]) {
    acts.front() = 1;
  }

  ~BiasLayer() {}

  const View<real_t> out_acts(const vector<int>& coords) {
    return acts;
  }

  const View<real_t> out_errs(const vector<int>& coords) {
    return errors;
  }
};

#endif
