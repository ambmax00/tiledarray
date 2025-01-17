/*
 * This file is a part of TiledArray.
 * Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TILEDARRAY_TEST_ARRAY_FIXTURE_H__INCLUDED
#define TILEDARRAY_TEST_ARRAY_FIXTURE_H__INCLUDED

#include "tiledarray.h"
#include "unit_test_config.h"
#include "range_fixture.h"

#include "../src/TiledArray/dist_array.h"

struct ArrayFixture : public TiledRangeFixture {
  typedef TArrayI ArrayN;
  typedef TSpArrayI SpArrayN;
  typedef ArrayN::index index;
  typedef ArrayN::size_type size_type;
  typedef ArrayN::value_type tile_type;

  ArrayFixture();

  ~ArrayFixture();


  TiledArray::Tensor<float> shape_tensor;
  TiledArray::World& world;
  ArrayN a;
  SpArrayN b;
}; // struct ArrayFixture


#endif // TILEDARRAY_TEST_ARRAY_FIXTURE_H__INCLUDED
