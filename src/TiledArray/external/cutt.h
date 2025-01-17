/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2018  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Chong Peng
 *  Department of Chemistry, Virginia Tech
 *  Aug 15, 2018
 *
 */

#ifndef TILEDARRAY_EXTERNAL_CUTT_H__INCLUDED
#define TILEDARRAY_EXTERNAL_CUTT_H__INCLUDED

#include <TiledArray/config.h>


#ifdef TILEDARRAY_HAS_CUDA

#include <vector>
#include <algorithm>

#include <cutt.h>

#include <TiledArray/range.h>
#include <TiledArray/permutation.h>

namespace TiledArray {

/**
 * convert the extent of a Tensor from RowMajor to ColMajor
 *
 * @param extent  the extent of a RowMajor Tensor
 */
inline void extent_to_col_major(std::vector<int> &extent) {
  std::reverse(extent.begin(), extent.end());
}


/**
 * convert the permutation representation of a Tensor from RowMajor to ColMajor
 * @param perm  the permutation of a RowMajor Tensor
 */
inline void permutation_to_col_major(std::vector<int> &perm) {
  int size = perm.size();

  std::vector<int> col_major_perm(size, 0);

  for (int input_index = 0; input_index < size; input_index++) {
    int output_index = perm[input_index];

    // change input and output index to col major
    int input_index_col_major = size - input_index - 1;
    int output_index_col_major = size - output_index - 1;

    col_major_perm[input_index_col_major] = output_index_col_major;
  }

  perm.swap(col_major_perm);
}

/**
 * @param inData  pointer to data in input Tensor, must be accessible on GPU
 * @param outData pointer to data in output Tensor, must be accessible on GPU
 * @param range the Range of input Tensor inData
 * @param perm  the permutation object
 * @param stream  the CUDA stream this permutation will be submitted to
 */
template <typename T>
void cutt_permute(T* inData, T* outData, const TiledArray::Range& range,
                  const TiledArray::Permutation& perm, cudaStream_t stream) {

  auto extent = range.extent();
  std::vector<int> extent_int(extent.begin(), extent.end());

  // cuTT uses FROM notation
  auto perm_inv = perm.inv();
  std::vector<int> perm_int(perm_inv.begin(), perm_inv.end());

  // cuTT uses ColMajor
  TiledArray::extent_to_col_major(extent_int);
  TiledArray::permutation_to_col_major(perm_int);

  cuttResult_t status;

  cuttHandle plan;
  status = cuttPlan(&plan, range.rank(), extent_int.data(), perm_int.data(),
                    sizeof(T), stream);

  TA_ASSERT(status == CUTT_SUCCESS);

  status = cuttExecute(plan, inData, outData);

  TA_ASSERT(status == CUTT_SUCCESS);

  status = cuttDestroy(plan);

  TA_ASSERT(status == CUTT_SUCCESS);

}

} // namespace TiledArray

#endif //  TILEDARRAY_HAS_CUDA

#endif //TILEDARRAY_EXTERNAL_CUTT_H__INCLUDED
